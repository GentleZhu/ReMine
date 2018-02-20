# ReMine: Integrating Local and Global Cohesiveness for Open Information Extraction
Source code and data for under review paper "Integrating Local and Global Cohesiveness for Open Information Extraction"

## Dependencies

We run all experiments on Ubuntu 16.04.

* python 3.5
* Python library dependencies
* [eigen 3.2.5](http://bitbucket.org/eigen/eigen/get/3.2.5.tar.bz2) (already included).

## Re-train our model on NYT and twitter corpus
### Phrase Extraction Module
```
$ bash phrase_extraction.sh
```
### Example Segmented Corpus
**background_phrase**
`entity_phrase`
_relation_phrase_

**Gov._Tim_Pawlenty_of_Minnesota** _order_ **the_state_health_department** **this_month** **to_monitor** `day-to-day_operation` _at_ the `Minneapolis_Veterans_Home` _after_ `state_inspector` _find_ _that_ **three_man** _have_die_ there _in_ **the_previous_month** because of neglect or `medical error`

**the_aid_group_doctor** without border _say_that_since_ `Saturday`, more than 275 **wounded_people** _have_ _be_admit_ _and_ _treat at_ `Donka_Hospital` in the capital of `Guinea`, **Conakry**. 
```
$ bash remine-ie.sh
```
