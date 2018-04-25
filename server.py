from flask import Flask, request, render_template, jsonify, Response

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


@app.route('/remine', methods =['POST'])
@cross_origin(origin='*')
def runRemine():
    default_input_model = 'pre_train/segmentation.model'
    #process = Popen(stdin=PIPE, stdout=PIPE, stderr=PIPE)
    # input = request.data
    # text = input.get('text')
    # print(input)
    input_path = 'tmp_remine/tokenized_test.txt'
    pos_path = 'tmp_remine/pos_tags_test.txt'
    dep_path = 'tmp_remine/deps_test.txt'
    ems_path = 'tmp_remine/remine_entity_position.txt'

    command = '{} {} {} {}'.format(input_path, pos_path, dep_path,ems_path)
    ret = []
    pane.send_keys(command, enter =True)

    output_path = 'remine_tokenized_segmented_sentences.txt'
    while True:
        if os.path.isfile('tmp_remine/finish.txt'):
            try:
                with open('tmp_remine/{}'.format(output_path), 'r') as f:
                    for line in f:
                        ret.append(line)
                break
            except IOError:
                break
    #clear the output
    os.remove('tmp_remine/finish.txt')
    os.remove('tmp_remine/remine_tokenized_segmented_sentences.txt')
    #prepare for next remine ,load the model againff
    #pane.send_keys('./bin/remine --model pre_train/segmentation.model --mode 1', enter=True)
    return jsonify({'tuple': ret})

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

