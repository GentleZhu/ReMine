## Integrating Local Context and Global Cohesiveness for Open Information Extraction <br />

<br />

#### **Updates**
**2018.8** ReMine supports our KDD showcase demo [AutoNet](http://35.166.108.88:8000/)!

**2018.7** ReMine migrates from Stanford NLP to Spacy regarding deployment.

**2018.3** ReMine 0.1 release! 


<br />

#### **Task Overview**

Extracting entities and their relations from text is an important task for understanding massive text corpora. Open information extraction (IE) systems mine relation tuples (i.e., entity arguments and a predicate string to describe their relation) from sentences. These relation tuples are not confined to a predefined schema for the relations of interests. However, current Open IE systems focus on modeling *local* context information in a sentence to extract relation tuples, while ignoring the fact that *global* statistics in a large corpus can be *collectively* leveraged to identify high-quality sentence-level extractions.

<br />

#### **Our Method**

To address the above issues, we propose a novel Open IE system, called **ReMine**,  which integrates local context signals and global structural signals in a unified, distant-supervision framework. Leveraging facts from external knowledge bases as supervision, the new system can be applied to many different domains to facilitate sentence-level tuple extractions using corpus-level statistics.

1. We integrate. <br />
2. Instead 

![alt text](framework_new.pdf)
<br />

#### **Framework**

1. Generate text features for each relation mention or QA entity
mention pair, and construct a heterogeneous graph using four
kinds of objects in combined corpus, namely relation mentions
from RE corpus, entity mention pairs from QA corpus, target relation
types and text features to encode aforementioned signals
in a unified form. (**Heterogeneous Network Construction**)
<br />
2. Jointly embed relation mentions, QA pairs, text features, and
type labels into two low-dimensional spaces connected by shared
features, where close objects tend to share the same types or
questions. (**Joint RE & QA Embedding**)
<br />
3. Estimate type labels **r** for each test relation mention **z** from
learned embeddings, by searching the target type set **R**.  (**Relation Type Inference**) <br />

![alt text](framework.png)
<br />

#### **Performance**

We evaluated our model on two domains: NYT and Twitter, each extraction in the mixed extraction pool is annotated by two annotators independently.

The table includes performance comparison with several *Open IE* systems over NYT corpus (**sentence-level extraction**):

Methods|Precision@100 | Precision@200 | MAP | NDCG@100 | NDCG@200
-------|--------------|---------------|-----|----------|---------
ClausIE ([Luciano et al., 2013](http://resources.mpi-inf.mpg.de/d5/clausie/clausie-www13.pdf)) | 0.580 | 0.625 | 0.623 | 0.575 | 0.667
Stanford ([Angeli et al., 2015](https://nlp.stanford.edu/pubs/2015angeli-openie.pdf)) | 0.680 | 0.625 | 0.665 | 0.689 | 0.654
OLLIE ([Mausam et al., 2012](http://turing.cs.washington.edu/papers/emnlp12-mausam.pdf)) | 0.670 | 0.640 | 0.683 | 0.684 | 0.775
MinIE ([Gashteovski et al., 2017](http://www.aclweb.org/anthology/D15-1205)) | 0.680 | 0.645 | 0.687 | 0.724 | 0.723
ReMine | **0.780** | **0.720** | **0.760** | **0.787** | **0.791**

<br />

#### **Resources**

1. Codes and datasets have been uploaded to *[Github](https://github.com/GentleZhu/ReMine)*
2. *[Live demo](http://35.166.108.88:8000/)* on News and Bio corpus.
3. *[ReMine-server](https://github.com/GentleZhu/ReMine/tree/server-dev)* will start a local server, access ReMine by inputing https://localhost:port-number in browser.
3. *[ReMine-standalone](https://github.com/GentleZhu/ReMine/tree/kdd)* is the standalone command line version for testing and training on given corpus.


<br />
