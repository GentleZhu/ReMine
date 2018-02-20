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
<span style="color:#0000FF">Gov. Tim Pawlenty of Minnesota</span> <span style="color:#000080">order</span> the state health department :BP this month :BP to monitor :BP day-to-day operation :EP at :RP the Minneapolis Veterans Home :EP after :RP state inspector :EP find :RP that :RP three man :BP have die :RP there in :RP the previous month :BP because of :BP neglect :RP or :RP medical error :EP 
`` the aid group doctor :BP without border :BP say that since :RP Saturday :EP , more than 275 wounded people :BP have :RP be admit :RP and :RP treat :RP at :RP Donka Hospital :EP in the capital of :BP Guinea :EP , Conakry :BP . 
`` the american people :BP can see :RP what be happen :BP here :RP , '' say :RP Senator Ben Nelson :EP , Democrat of Nebraska :BP . ''
## Run Open-IE demo
```
$ bash remine-ie.sh
```
