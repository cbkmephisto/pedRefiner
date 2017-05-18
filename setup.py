#!/usr/bin/env python3
# encoding: utf8

from distutils.core import setup

setup(
    name='pedRefiner',
    # this must be the same as the name above
    packages=['pedRefiner', 'test'],
    version='0.1.0',
    description='Trivial tool that takes a list of animal IDs, ' +
                'extracts a csv pedigree file for the given IDs and all ' +
                'their ancestors\' IDs, builds a new pedigree with them ' +
                'sorted, and stores it into a new file.',
    author='Hailin Su',
    author_email='cbkmephisto@gmail.com',
    data_files=[('test_data', ['test/list', 'test/long_ped.csv', 'test/out.long_ped.csv', 'test/test.sh'])],
    # use the URL to the github repo
    url='https://github.com/cbkmephisto/pedRefiner.py',
    # I'll explain this in a second
    download_url='https://github.com/cbkmephisto/' +
                 'pedRefiner.py/archive/0.1.zip',
    scripts=['pedRefiner/pedRefiner.py'],
    # arbitrary keywords
    keywords=['pedigree', 'refine', 'extract', 'ancestors'],
    classifiers=[
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Development Status :: 4 - Beta",
        "Environment :: Console",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)",
        "Operating System :: OS Independent",
        "Topic :: Scientific/Engineering :: Bio-Informatics"
    ],
    long_description="""
Trivial tool that takes a list of animal IDs, extracts a pedigree file for the given IDs and all their ancestors' IDs, builds a new pedigree file with them sorted, and dumps the output to a new file.

Suggested usage of PedRefiner: the pipeline() method.

:param  1 lst_fn:              file containing animal list to be grepped from the pedigree, one line each
:param  2 ped_fn:              input pedigree file, 3 col
:param  3 opt_fn:              output pedigree file, 3 col
:param  4 gen_max:     (0)     maximum recursive generation to grep for EVERYONE in lst_fn
:param  5 missing_in:  ('.')   missing value for input file, default = '.'
:param  6 missing_out: ('.')   missing value for output file, default = '.'
:param  7 sep_in:      (',')   separator for input file, default = ',' i.e. csv
:param  8 sep_out:     (',')   separator for output file, default = ',', i.e. csv
:param  9 xref_fn:     (None)  cross-reference file to modify output pedigree
:param 10 flag_r:      (False) bool, output descendant IDs if True

Example

- Python
    from pedRefiner import PedRefiner
    pr = PedRefiner()
    # pr.help()
    pr.pipeline('animal_list', 'ped.input.csv', 'ped.output.csv', gen_max=3)

- Bash
    #             1       2             3               4gen_max  5missin  6missout  7sepin 8sepout 9    10
    pedRefiner.py ANM_LST PED_INPUT.csv PED_OUTPUT.csv [3         0        0         ,      ,       xref True]


[Description]

- extracts the pedigree of all the ancestors of the animals in the anmList file;
- if '-r' option is given, prints out all the descendants' IDs from the anmList file instead of print out the refined pedigree;
- xref the pedigree info if specified a whitespace-delimited 3-col xref file with 1st col as command:
       # line starting with sharp will be ignored
       # cmd = A  : [A]ll ID1 (col1, 2 and 3) will be changed to ID2
       A ID1 ID2

       # cmd = S  : All ID1 in the [S]ire (2nd) column will be changed to ID2
       S ID1 ID2

       # cmd = D  : All ID1 in the [D]am  (3rd) column will be changed to ID2
       D ID1 ID2
- force accepting the latest version if multiple entries for the same animal were detected;


[Updates]

- version 2017-02-23 prevent unexpected order for complex pedigree while specifying rec_gen_max
- version 2017-02-22
  - modified to be a non-recursive version to prevent segmentation fault for a very very very long pedigree
  - only maintaining python version
- version 2016-03-31 added -r option: will print out all the descendants' IDs from anmList, recursively to the end
- version 2015-07-13 added output: pedOut.ID2Gender according to the ped, and 'M' for unknown
- version 2015-07-02 added xref to correct animal IDs: replace 1st col with 2nd col
- version 2015-06-29 prevent segmentation fault 11 due to pedigree loop
- version 2015-06-12 added updating pre-loaded empty entries
- version 2015-04-15 added a 3rd parameter to determine the recursive generations
- version 2014-11-10 modified check() so that all the errors would be checked in one run added sort()
- version 2014-04-15 initial version"""
)
