from flask import Flask, request, render_template, jsonify, Response
import requests

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



# @app.route('/C')
# @cross_origin(origin='*')
# def runC():
#     #subprocess.call(['make', '-C', '../c++'])
#     process =Popen(['./../c++/q1'],stdout = PIPE,stderr = PIPE)
#     stdout,stderr = process.communicate()
#     return stdout

# @app.route('/remine', methods =['POST'])
# @cross_origin(origin='*')
# def runRemine():
#     #subprocess.call(['bash','remine-ie.sh'])
# stdout,stderr = process.communicate(input = b' {}\n{}\n{}\n'.formmat(input_path, pos_path, dep_path))
# subprocess.call(['./bin/remine',
#                  '--input_file', '{}'.format(input_path),
#                  '--pos_file', '{}'.format(pos_path),
#                  '--deps_file', '{}'.format(dep_path),
#                  '--model', '{}'.format(model_path),
#                  '--mode', '0'])


@app.route('/')
@cross_origin(origin='*')
def render():
    return render_template('example.html')

#todo generate an api to set model.


#pass information to c++ web
@app.route('/remine', methods =['POST'])
@cross_origin(origin='*')
def senddata():
    dep_text = '0_4_det\n1_4_compound\n2_4_compound\n3_7_nsubj\n4_6_case\n5_4_nmod: without\n6_0_root\n7_19_mark\n8_10_case\n9_19_nmod: since\n10_19_punct\n11_14_advmod\n12_12_mwe\n13_16_nummod\n14_16_amod\n15_19_nsubjpass\n16_19_aux\n17_19_auxpass\n18_7_ccomp\n19_19_cc\n20_7_ccomp\n21_24_case\n22_24_compound\n23_19_nmod: at\n24_27_case\n25_27_det\n26_24_nmod: in\n27_29_case\n28_27_nmod: of\n29_29_punct\n30_29_appos\n31_7_punct\n0_3_det\n1_3_amod\n2_5_nsubj\n3_5_aux\n4_11_ccomp\n5_8_nsubj\n6_8_aux\n7_5_ccomp\n8_8_advmod\n9_11_punct\n10_0_root\n11_14_compound\n12_14_compound\n13_11_nsubj\n14_14_punct\n15_14_appos\n16_18_case\n17_16_nmod: of\n18_11_punct\n'
    token_text = '18 2632 421 1310 1895 376 427 2 1524 1219 17 147 156 19160 24653 438 216 10 4870 42 10418 28 153974 1271 26 18 468 4 24820 17 56999 60\n18 465 438 554 1018 14 10 1448 473 17 427 696 8884 1033 17 880 4 5137 60'
    pos_text = 'DT\nJJ\nNNS\nMD\nVB\nWP\nVBZ\nVBG\nRB\n,\nVBD\nNNP\nNNP\nNNP\n,\nNNP\nIN\nNNP\n.\nCC\nNNP\nVBD\nPRP\nVBD\nVB\nJJ\nNN\nIN\nPRP$\nJJ\nNN\nTO\nNNP\nCC\nNNP\nIN\nNNP\nNNP\nIN\nNNP\n.'
    ems_text = '0_4 5_6 9_10 14_16 22_24 25_29 30_31\n0_3 5_8 11_14 15_18'
    response = requests.get('http://dmserv4.cs.illinois.edu:10086/pass_result', json ={"pos": pos_text, "tokens": token_text, "dep": dep_text, "ent": ems_text})
    #json_data = response.json()
    #print(json_data)
    print(response.text)
    return response.text



# @app.route('/remine', methods =['POST'])
# @cross_origin(origin='*')
# def runRemine():
#     default_input_model = 'pre_train/segmentation.model'
#     #process = Popen(stdin=PIPE, stdout=PIPE, stderr=PIPE)
#     # input = request.data
#     # text = input.get('text')
#     # print(input)
#     input_path = 'tmp_remine/tokenized_test.txt'
#     pos_path = 'tmp_remine/pos_tags_test.txt'
#     dep_path = 'tmp_remine/deps_test.txt'
#     ems_path = 'tmp_remine/remine_entity_position.txt'
#
#     token_text = '18 2632 421 1310 1895 376 427 2 1524 1219 17 147 156 19160 24653 438 216 10 4870 42 10418 28 153974 1271 26 18 468 4 24820 17 56999 60\t18 465 438 554 1018 14 10 1448 473 17 427 696 8884 1033 17 880 4 5137 60'
#     pos_text = 'DT JJ NNS MD VB WP VBZ VBG RB , VBD NNP NNP NNP , NNP IN NNP .\nCC NNP VBD PRP VBD VB JJ NN IN PRP$ JJ NN TO NNP CC NNP IN NNP NNP IN NNP .'
#     dep_text = '3_det 3_amod 5_nsubj 5_aux 11_ccomp 8_nsubj 8_aux 5_ccomp 8_advmod 11_punct 0_root 14_compound 14_compound 11_nsubj 14_punct 14_appos 18_case 16_nmod:of 11_punct\n3_cc 3_nsubj 0_root 6_nsubj 6_aux 3_ccomp 8_amod 6_dobj 12_case 12_nmod:poss 12_amod 6_nmod:in 14_case 12_nmod:to 14_cc 12_nmod:to 19_case 19_compound 12_nmod:in 21_case 12_nmod:in 3_punct'
#     ems_text = '0_3 5_8 11_14 15_18\n1_2 6_8 9_10 10_12 13_14 15_19 20_21'
#     total = token_text + '\t' + pos_text + '\t' + dep_text
#     command = '{} {} {} {}'.format(input_path, pos_path, dep_path,ems_path)
#     ret = []
#     pane.send_keys(command, enter =True)
#
#     output_path = 'remine_tokenized_segmented_sentences.txt'
#     while True:
#         if os.path.isfile('tmp_remine/finish.txt'):
#             try:
#                 with open('tmp_remine/{}'.format(output_path), 'r') as f:
#                     for line in f:
#                         ret.append(line)
#                 break
#             except IOError:
#                 break
#     #clear the output
#     os.remove('tmp_remine/finish.txt')
#     os.remove('tmp_remine/remine_tokenized_segmented_sentences.txt')
#
#     return jsonify({'tuple': ret})

if __name__=='__main__':
    #app.run(debug = True, host = '0.0.0.0',port=1111)
    # app.run(debug = True, host = 'localhost', port=5000)

    #create the tmux server to preload the model
    #TODO preload different models with different windows, ready for any model
    server = libtmux.Server()
    session = server.find_where({"session_name": "preload"})
    window = session.new_window(attach=False, window_name="remine")
    pane = window.split_window(attach=False)
    pane.send_keys('cd /shared/data/remine/CS512_Website/Remine', enter = True)
    pane.send_keys('./bin/remine --model pre_train/segmentation.model --mode 1', enter=True)

    http_server = WSGIServer(('0.0.0.0', 1111), app)

    http_server.serve_forever()

