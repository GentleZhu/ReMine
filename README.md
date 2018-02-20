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
**[background phrase]**
`entity phrase`
_relation phrase_

**[Gov. Tim Pawlenty of Minnesota]** _order_ **[the state health department]** **[this month]** **[to monitor]** `day-to-day operation` _at_ `the Minneapolis Veterans Home` _after_ `state inspector` _find_ _that_ **[three man]** _have die_ there _in_ **[the previous month]** because of neglect or `medical error`
**[the aid group doctor]** without border _say that since_ `Saturday`, more than 275 **[wounded people]** _have_ _be_admit_ _and_ _treat at_ `Donka Hospital` in the capital of `Guinea`, **[Conakry]**. 
```
$ bash remine-ie.sh
```
