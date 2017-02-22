# pedRefiner [![Build Status](https://travis-ci.org/cbkmephisto/pedRefiner.svg?branch=recursive_version)](https://travis-ci.org/cbkmephisto/pedRefiner)
Trivial tool that takes a list of animal IDs, extracts a csv pedigree file for the given IDs and all their ancestors' IDs, builds a new pedigree file with them sorted, and prints it on screen (stdout). Can be redirected into a new csv file.

```
pedRefiner          by hailins@iastate.edu

[Usage]

pedRefiner anmList full_ped_csv_file [ -r | num_recursive_gen | xref-filename] > pedOut.csv 2>errLog

[Description]
 - extracts the pedigree of all the ancestors of the animals in the anmList file;
 - if '-r' option is given, prints out all the descendants' IDs from the anmList file instead of print out the refined pedigree;
 - xref the pedigree info if specified a whitespace-delimited 3-col xref file with 1st col as command:
       #          : line starting with sharp will be ignored
       A ID1 ID2  : all ID1 (col1, 2 and 3) will be changed to ID2
       S ID1 ID2  : all ID1 in the sire (2nd) column will be changed to ID2
       D ID1 ID2  : all ID1 in the dam  (3rd) column will be changed to ID2
 - force accepting the latest version if multiple entries for the same animal were detected;
 - messages print out on screen (stderr, 2> to redirect)
 - results  print out on screen (stdout, 1> to redirect)
            ancestors ordered prior than offsprings, delimetered by comma ','

[Updates]
 - version 2016-03-31 added -r option: will print out all the descendants' IDs from anmList, recursively to the end
 - version 2015-07-13 added output: pedOut.ID2Gender according to the ped, and 'M' for unknown
 - version 2015-07-02 added xref to correct animal IDs: replace 1st col with 2nd col
 - version 2015-06-29 prevent segmentation fault 11 due to pedigree loop
 - version 2015-06-12 added updating pre-loaded empty entries
 - version 2015-04-15 added a 3rd parameter to determine the recursive generations
 - version 2014-11-10 modified check() so that all the errors would be checked in one run
                      added sort()
 - version 2014-04-15 initial version
```
