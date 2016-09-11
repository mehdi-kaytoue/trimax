TriMax
======

TriMax algorithm for computing maximal biclusters of similar values

Requirements
----------
* Boost C++ library (tested on 1.54.0)

Compiling and executing
---------

* compile with 

> g++ -O3 -I /usr/local/boost_1_54_0/ *.cpp -o trimax

* Exec with 

> ./trimax datafile theta min_objects min_attributes max_objects max_attributes

> e.g. ./trimax sample.txt 2 0 0 4 5

Author
----------
Mehdi Kaytoue - LIRIS / INSA de Lyon


Reference 
----------
**Biclustering meets triadic concept analysis.**
M. Kaytoue, S.O. Kuznetsov, J. Macko, A. Napoli. 
Annals of Mathematics and Artificial Intelligence ():1-25, 
Springer Netherlands, ISSN 1012-2443.   2013.
DOI : 10.1007/s10472-013-9379-1
http://liris.cnrs.fr/publis/?id=6292
