# ReMine: Integrating Local and Global Cohesiveness for Open Information Extraction
Source code and data for under review paper "Integrating Local and Global Cohesiveness for Open Information Extraction"

## Dependencies

We run all experiments on Ubuntu 16.04.

* python 3.5
* Python library dependencies
* [eigen 3.2.5](http://bitbucket.org/eigen/eigen/get/3.2.5.tar.bz2) (already included).
### Build
```
$ bash compile.sh
```

## Test with pre-trained model
```
$ bash remine-ie.sh
```
The result files can be found at results_remine/remine_results.txt

## Re-train our model on NYT and twitter corpus(under polishing)
### Phrase Extraction Module
```
$ bash phrase_extraction.sh
```
### Example Segmented Corpus
(background_phrase)
[entity_phrase]
\<relation_phrase\>

(Gov. Tim Pawlenty of Minnesota) \<ordered\> (the state health department) (this month) (to monitor) [day-to-day operation] \<at\> the [Minneapolis Veterans Home] \<after\> [state inspector] \<found\> \<that\> (three man) \<had died\> there \<in\> (the previous month) (because of) [neglect] \<or\> [medical error]
  
(The aid group Doctor) (Without Border) \<said that since\> [Saturday], more than 275 (wounded people) \<had\> \<been admitted\> \<and\> \<treated\> \<at\> [Donka Hospital] (in the capital of) [Guinea], (Conakry).

### Integrated Optimizer
```
$ bash train.sh
```
