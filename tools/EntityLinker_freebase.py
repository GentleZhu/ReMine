import urllib2
import urllib
import re
from lxml import html
import json
import threading
import sys, traceback
import operator
from collections import defaultdict
import os
import time

"""Entity Linker for a set of documents"""
#the set where 
typeDict = {}
docList = {}
indexMap = {}
dbIdToFbId = defaultdict(str)
fbIdToFbType = defaultdict(set)
threadNum = 20

def entityMapper(mapper):
    f = open(mapper)
    for line in f:
        segments = line.strip().split(' ')
        dbID = segments[0][29:-1]
        fbID = segments[2][28:-1]
        dbIdToFbId[dbID] = fbID
    print 'load dbID to fbID map done.'
    print len(dbIdToFbId), 'dbIDs'

def typeMapper(mapper):
    f = open(mapper)
    start = time.time()
    count = 0
    for line in f:
        try:
            sub, obj = line.strip().split('\t')
            # print sub, obj
            fbID = sub[28:-1]
            type = obj[28:-1]
            fbIdToFbType[fbID].add(type)
        except:
            count += 0
    print 'load fbID to fbTypes map done.'
    print count, 'lines failed'
    print 'used', time.time() - start


class myLinker (threading.Thread):
    docList = {}
    offset = 0
    confidence = 0
    def __init__(self, docList, offset, outDir, confidence):
        threading.Thread.__init__(self)
        self.docList = docList
        self.offset = offset
        self.confidence = confidence
        self.outDir = outDir
    def run(self):
        print "Start DBpediaSpotlight"
        g = open(self.outDir + 'temp' + str(self.offset) + '.txt', 'w')
        index = 0
        while 1:
            did = str(index + self.offset)
            if did in self.docList:
                try:
                    doc = self.docList[did]
                    # print doc
                    url = "http://spotlight.sztaki.hu:2222/rest/annotate"
                    #url = "http://localhost:2222/rest/annotate"
                    data = {"confidence":self.confidence}
                    data["support"] = "20"
                    data["text"] = doc;
                    data = urllib.urlencode(data)
                    req = urllib2.Request(url)
                    req.add_header('Accept', 'application/json') #text/xml')
                    # print did
                    page = html.fromstring(urllib2.urlopen(req, data, timeout=100).read())
                    docJson = html.tostring(page)[3:-4]
                    # print docJson
                    validEntities = extractAnnotations(docJson)
                    for entity in validEntities:
                        if (entity['@URI'] != None):
                            g.write(str(index + self.offset) + '\t' + entity['@offset'] + '\t' + entity['@surfaceForm'] + '\t' + entity['@URI'] 
                                + '\t' + entity['@similarityScore'] + '\t' + entity['@percentageOfSecondRank']+ '\n')
                    index += threadNum
                except:
                    index += threadNum
                    print 'noresult'
                if int(did) % 10 == 0 and int(did) != 0:
                    sys.stdout.write("Progress: %d docs  \r" % (int(did)) )
                    sys.stdout.flush()
            else:
                break
        g.close()


def link(outDir, outFile, confidence):
    # typeDict['m.01c5'] = None
    # read documents
    index = 0
    f = open(outDir + outFile + '_index.txt')
    for doc in f:
        tab = doc.find('\t')
        did = doc[:tab]
        text = doc[tab+1:]
        docList[str(index)] = text
        indexMap[str(index)] = did
        index += 1
    f.close()

    print 'Total docs to link:', len(docList)
    
    #for i in range(0, threadNum):
    #   print 'starrrrr'
    #   thread.start_new_thread( eachLinker, (i,) )
    threadLock = threading.Lock()
    threads = []

    for i in range(0, threadNum):
        mythread = myLinker(docList, i, outDir, confidence)
        mythread.start()
        threads.append(mythread)

    # Wait for all threads to complete
    for t in threads:
        t.join()

    print "Start joining the files"

    allLines = {}

    for i in range(0, threadNum):
        f = open(outDir + 'temp' + str(i) + '.txt')
        for line in f:
            tab = line.find('\t')
            did = indexMap[line[:tab]]
            allLines[line] = did

    sorted_x = sorted(allLines.items(), key=operator.itemgetter(1))
    g = open(outDir + outFile + '_DBlinked.txt', 'w')
    for tup in sorted_x:
        g.write(tup[0])
    g.close()
    os.system("rm -rf results/temp*")

# type file
def type(typeFile):
    # read target types
    f = open(typeFile)
    for line in f:
        myType = line.split('\t')[0]
        if myType != 'NIL':
            typeDict[myType] = set()
            typeDict[myType].add(myType)
    print typeDict
    f.close()
    return 

# Extract dbpedia annotations
def extractAnnotations(docJson):
    validEntities = []
    decoded = json.loads(docJson)
    for entity in decoded['Resources']:
        # isTarget = False
        # types = entity['@types'].split(',')
        # for eType in types:
        #   if eType in typeSet:
        #       isTarget = True
        #       entity['@types'] = eType
        #       break
        isTarget = True
        if isTarget:
            validEntities.append(entity)

    return validEntities

# map dbpedia to freebase
def linkToFreebase(entity):
    dbID = entity['@URI'][28:]
    if dbID in dbpediaToFreebase:
        entity['@URI'] = dbpediaToFreebase[dbID]
    else:
        entity['@URI'] = None


# read the freebase entity and extract the notable type
def findFBTypes(outDir, outFile):
    print 'start mapping to freebase IDs and types.'
    f = open(outDir + outFile + '_DBlinked.txt')
    g = open(outDir + outFile + '_FBtyped.txt', 'w')
    err_count = 0
    # Get freebase mid and types
    for line in f:
        try:
            did, surfaceForm, URI, similarityScore, rank = line.strip().split('\t')
            dbID = URI[28:]
            sim_score = float(similarityScore)
            rank_score = float(rank)
            if dbID in dbIdToFbId and dbIdToFbId[dbID] in fbIdToFbType:
                fbID = dbIdToFbId[dbID]
                types = ', '.join(fbIdToFbType[fbID])
                g.write(did + '\t' + surfaceForm + '\t' + fbID + '\t' + types
         + '\t' + similarityScore + '\t' + rank + '\n')
        except:
            err_count += 1

    print err_count, 'lines failed.'
    g.close()
    f.close()

def addDocIndex(inFileName, outDir, outFile):
    count = 0
    with open(inFileName) as f, open(outDir + outFile + '_index.txt', 'w') as f1:
        for line in f:
            if line.strip():
                f1.write(str(count) + '\t' + line.strip() + '\n')
                count += 1
        

if __name__ == "__main__":

    if len(sys.argv) != 4:
        print "Usage: python EntityLinker_freebase.py /shared/data/xren7/multi-parsing/data-to-type/DBLP_five_domains_abstracts.txt results/ DBLP"

    inFileName=sys.argv[1] # filepath to RawText (one doc per line)
    outDir=sys.argv[2] # output directory
    outFile=sys.argv[3] # filename, e.g., DBLP

    ### add doc index (if not index is associated with each doc)
    addDocIndex(inFileName, outDir, outFile)

    ### DBpedia SpotLight
    link(outDir, outFile, 0.2) 

    ### Map to freebase type
    # entityMapper('/shared/data/xren7/EntityLinking/resource/freebase_links.nt') # freebase_links.nt
    # typeMapper('/shared/data/xren7/EntityLinking/resource/freebase-mid-type.map') # freebase-mid-type.map, 30G memory requirement
    # findFBTypes(outDir, outFile) # find notable type for each entity

