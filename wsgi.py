from ReMine import app

if __name__ == "__main__":
    global coref
    coref = Coref()
    model1 = Model('tmp_remine/token_mapping.p')
    global NLP_client
    NLP_client = CoreNLPClient(server='http://dmserv4.cs.illinois.edu:9000',
                               default_annotators=['depparse', 'lemma', 'pos'])
    app.run()