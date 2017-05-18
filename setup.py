#!/usr/bin/env python3
# encoding: utf8

from distutils.core import setup

setup(
    name='pedRefiner',
    # this must be the same as the name above
    packages=['pedRefiner'],
    version='0.1',
    description='Trivial tool that takes a list of animal IDs, ' +
                'extracts a csv pedigree file for the given IDs and all ' +
                'their ancestors\' IDs, builds a new pedigree with them ' +
                'sorted, and stores it into a new file.',
    author='Hailin Su',
    author_email='cbkmephisto@gmail.com',
    # use the URL to the github repo
    url='https://github.com/cbkmephisto/pedRefiner.py',
    # I'll explain this in a second
    download_url='https://github.com/cbkmephisto/' +
                 'pedRefiner.py/archive/0.1.zip',
    # arbitrary keywords
    keywords=['pedigree', 'refine', 'extract', 'ancestors'],
    classifiers=[],
)
