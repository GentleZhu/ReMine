from flask import Flask, request, render_template, jsonify, Response,json
import requests
from stanza.nlp.corenlp import CoreNLPClient
#from corenlp_pywrap import pywrap
import subprocess
import sys,os
from subprocess import Popen, PIPE
import os.path
from gevent.wsgi import WSGIServer

from flask_cors import CORS, cross_origin
import StringIO
import libtmux
import json
from src_py.remine_online import Solver

app = Flask(__name__)
cors = CORS(app)
app.config['CORS_HEADERS'] = 'Content-Type'





@app.route('/')
@cross_origin(origin='*')
def render():
    return render_template('example.html')

#todo generate an api to set model.


#pass information to c++ web
@app.route('/remine', methods =['POST'])
@cross_origin(origin='*')
def senddata():
    #get input from front end
    data = request.data
    #print(data)
    json_data = json.loads(data)
    #print(json_data)
    raw = json_data["text"]
    #print(raw)

    # dep_text = '0_4_det\n1_4_compound\n2_4_compound\n3_7_nsubj\n4_6_case\n5_4_nmod: without\n6_0_root\n7_19_mark\n8_10_case\n9_19_nmod: since\n10_19_punct\n11_14_advmod\n12_12_mwe\n13_16_nummod\n14_16_amod\n15_19_nsubjpass\n16_19_aux\n17_19_auxpass\n18_7_ccomp\n19_19_cc\n20_7_ccomp\n21_24_case\n22_24_compound\n23_19_nmod: at\n24_27_case\n25_27_det\n26_24_nmod: in\n27_29_case\n28_27_nmod: of\n29_29_punct\n30_29_appos\n31_7_punct\n0_3_det\n1_3_amod\n2_5_nsubj\n3_5_aux\n4_11_ccomp\n5_8_nsubj\n6_8_aux\n7_5_ccomp\n8_8_advmod\n9_11_punct\n10_0_root\n11_14_compound\n12_14_compound\n13_11_nsubj\n14_14_punct\n15_14_appos\n16_18_case\n17_16_nmod: of\n18_11_punct\n'
    # token_text = '18 2632 421 1310 1895 376 427 2 1524 1219 17 147 156 19160 24653 438 216 10 4870 42 10418 28 153974 1271 26 18 468 4 24820 17 56999 60\n18 465 438 554 1018 14 10 1448 473 17 427 696 8884 1033 17 880 4 5137 60'
    # pos_text = 'DT\nNN\nNN\nNNS\nIN\nNNS\nVBD\nIN\nIN\nNNP\n,\nJJR\nIN\nCD\nJJ\nNNS\nVBD\nVBN\nVBN\nCC\nVBN\nIN\nNNP\nNNP\nIN\nDT\nNN\nIN\nNNP\n,\nNNP\n.\nDT\nJJ\nNNS\nMD\nVB\nWP\nVBZ\nVBG\nRB\n,\nVBD\nNNP\nNNP\nNNP\n,\nNNP\nIN\nNNP\n.'
    # ems_text = '0_4 5_6 9_10 14_16 22_24 25_29 30_31\n0_3 5_8 11_14 15_18'
    # dep_text = '4_det 4_compound 4_compound 7_nsubj 6_case 4_nmod:without 0_root 19_mark 10_case 19_nmod:since 19_punct 14_advmod 12_mwe 16_nummod 16_amod 19_nsubjpass 19_aux 19_auxpass 7_ccomp 19_cc 7_ccomp 24_case 24_compound 19_nmod:at 27_case 27_det 24_nmod:in 29_case 27_nmod:of 29_punct 29_appos 7_punct\n3_det 3_amod 5_nsubj 5_aux 11_ccomp 8_nsubj 8_aux 5_ccomp 8_advmod 11_punct 0_root 14_compound 14_compound 11_nsubj 14_punct 14_appos 18_case 16_nmod:of 11_punct'
    # token_text = 'the aid group doctor without border say that since Saturday , more than 275 wounded people have be admit and treat at Donka Hospital in the capital of Guinea , Conakry .\nthe american people can see what be happen here , say Senator Ben Nelson , Democrat of Nebraska .'
    # pos_text = 'DT NN NN NNS IN NNS VBD IN IN NNP , JJR IN CD JJ NNS VBD VBN VBN CC VBN IN NNP NNP IN DT NN IN NNP , NNP .\nDT JJ NNS MD VB WP VBZ VBG RB , VBD NNP NNP NNP , NNP IN NNP .'

    #dep_text =''
    #token_text = ''
    #pos_text = ''
    #ems_text = ''
    dep_text = StringIO.StringIO()
    token_text = StringIO.StringIO()
    pos_text = StringIO.StringIO()
    annotated = NLP_client.annotate(raw)

    for sentence in annotated.sentences:
        edges = sentence.depparse().to_json()
        dep_list = [''] * (len(edges)+1)
        for edge in edges:
            if edge['dep'] == "root":
                dep_list[edge['dependent']] = "0_root"
            else:
                dep_list[edge['dependent']] = "{}_{}".format(edge['governer'], edge['dep'])
        dep_text.write(' '.join(dep_list[1:]) + '\n')
        token_len = len(sentence)
        cout = 0
        for token in sentence:
            #print('parse',token.depparse())
            #print('pos',token.pos)
            #dep_text = dep_text + '\n' + token.dep
            # token_text = token_text + '\n' + token.lemma
            # pos_text = pos_text + '\n' + token.pos
            # dep_text = dep_text + '\n' + token.depparse
            #print(token.lemma)
            #print(token.pos)
            if cout == token_len -1 :
                token_text.write(token.lemma)
                pos_text.write(token.pos)
            else:
                token_text.write(token.lemma + ' ')
                pos_text.write(token.pos + ' ')
            cout += 1

        token_text.write('\n')
        pos_text.write('\n')


    #remine-ie.sh
    dep_text = dep_text.getvalue().rstrip()
    token_text = token_text.getvalue().rstrip()
    pos_text = pos_text.getvalue().rstrip()
    #print(dep_text)
    #print(token_text)
    #print(pos_text)
    answer = Solver()
    answer.load()
    answer.tokenized_test(token_text, pos_text, dep_text)


    print(answer.fdoc)
    print(answer.fpos)
    print(answer.fdep)
    # print(answer.fems)
    response = requests.get('http://dmserv4.cs.illinois.edu:10086/pass_result', json ={"pos": answer.fpos, "tokens": answer.fdoc, "dep": answer.fdep, "ent": answer.fems, "mode": 0})
    remine_segmentation = response.text
    #print("remine_segement",remine_segmentation)
    remine_seg_out = answer.mapBackv2(remine_segmentation)
    check = answer.extract_transformat(remine_seg_out, token_text, pos_text)
    assert check == 1
    #print('ems:\n', answer.fems)
    print(answer.fdoc)
    #print(answer.fpos)
    #print(answer.fdep)
    response = requests.get('http://dmserv4.cs.illinois.edu:10086/pass_result', json ={"pos": answer.fpos, "tokens": answer.fdoc, "dep": answer.fdep, "ent": answer.fems, "mode": 1})
    remine_segmentation = response.text

    #result = answer.translate(remine_segmentation)

    #print(result)
    #print(remine_segmentation)
    #with open("result.txt","w") as f:
        #f.write(result)

    return jsonify({'tuple': remine_segmentation})



if __name__=='__main__':
    #app.run(debug = True, host = '0.0.0.0',port=1111)
    # app.run(debug = True, host = 'localhost', port=5000)

    #create the tmux server to preload the model
    #TODO preload different models with different windows, ready for any model
    #server = libtmux.Server()
    #session = server.find_where({"session_name": "preload"})
    #window = session.new_window(attach=False, window_name="remine")
    #pane = window.split_window(attach=False)
    #pane.send_keys('cd /shared/data/remine/CS512_Website/Remine', enter = True)
    #pane.send_keys('./bin/remine --model pre_train/segmentation.model --mode 1', enter=True)
    #global NLP_client
    NLP_client = CoreNLPClient(server='http://dmserv4.cs.illinois.edu:9003',default_annotators=['depparse', 'lemma', 'pos'])
   # NLP_clinet = pywrap.CoreNLP(url='http://localhost:9002', annotator_list=["lemma","ner","depparse","pos"])

    http_server = WSGIServer(('0.0.0.0', 1111), app)

    http_server.serve_forever()
