from flask import Flask, request, render_template, jsonify, Response
import requests
from stanza.nlp.corenlp import CoreNLPClient
import subprocess
import sys
import os
from subprocess import Popen, PIPE
import os.path

from gevent.wsgi import WSGIServer

from flask_cors import CORS, cross_origin

import libtmux

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
    raw = request.data.text

    #dep_text = '0_4_det\n1_4_compound\n2_4_compound\n3_7_nsubj\n4_6_case\n5_4_nmod: without\n6_0_root\n7_19_mark\n8_10_case\n9_19_nmod: since\n10_19_punct\n11_14_advmod\n12_12_mwe\n13_16_nummod\n14_16_amod\n15_19_nsubjpass\n16_19_aux\n17_19_auxpass\n18_7_ccomp\n19_19_cc\n20_7_ccomp\n21_24_case\n22_24_compound\n23_19_nmod: at\n24_27_case\n25_27_det\n26_24_nmod: in\n27_29_case\n28_27_nmod: of\n29_29_punct\n30_29_appos\n31_7_punct\n0_3_det\n1_3_amod\n2_5_nsubj\n3_5_aux\n4_11_ccomp\n5_8_nsubj\n6_8_aux\n7_5_ccomp\n8_8_advmod\n9_11_punct\n10_0_root\n11_14_compound\n12_14_compound\n13_11_nsubj\n14_14_punct\n15_14_appos\n16_18_case\n17_16_nmod: of\n18_11_punct\n'
    #token_text = '18 2632 421 1310 1895 376 427 2 1524 1219 17 147 156 19160 24653 438 216 10 4870 42 10418 28 153974 1271 26 18 468 4 24820 17 56999 60\n18 465 438 554 1018 14 10 1448 473 17 427 696 8884 1033 17 880 4 5137 60'
    #pos_text = 'DT\nNN\nNN\nNNS\nIN\nNNS\nVBD\nIN\nIN\nNNP\n,\nJJR\nIN\nCD\nJJ\nNNS\nVBD\nVBN\nVBN\nCC\nVBN\nIN\nNNP\nNNP\nIN\nDT\nNN\nIN\nNNP\n,\nNNP\n.\nDT\nJJ\nNNS\nMD\nVB\nWP\nVBZ\nVBG\nRB\n,\nVBD\nNNP\nNNP\nNNP\n,\nNNP\nIN\nNNP\n.'
    #ems_text = '0_4 5_6 9_10 14_16 22_24 25_29 30_31\n0_3 5_8 11_14 15_18'
    dep_text =''
    token_text = ''
    pos_text = ''
    ems_text = ''
    annotated = NLP_client.annotate(raw)
    for sentence in annotated.sentences:
        print('sentence', sentence)
        for token in sentence:
            dep_text = dep_text + '\n' + token.dep
            token_text = token_text + '\n' + token.lemma
            pos_text = pos_text + '\n' + token.pos

    response = requests.get('http://dmserv4.cs.illinois.edu:10086/pass_result', json ={"pos": pos_text, "tokens": token_text, "dep": dep_text, "ent": ems_text})

    print(response.text)
    with open("result.txt","w") as f:
        f.write(response.text)

    return response.text



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
    global NLP_client
    print(1)
    NLP_client = CoreNLPClient(server='http://localhost:9999',default_annotators=['tokenize', 'lemma', 'pos', 'ner','dep'])


    http_server = WSGIServer(('0.0.0.0', 1111), app)

    http_server.serve_forever()



