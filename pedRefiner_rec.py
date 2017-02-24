#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#  Copyright (c) 2017 Hailin Su, ISU NBCEC
#
#  This file is part of pedRefiner.
#
#  pedRefiner is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  pedRefiner is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU General Lesser Public License
#  along with pedRefiner. If not, see <http://www.gnu.org/licenses/>.
#
#
#  pedRefiner.cpp
#      Created by Hailin SU on 4/15/14.
#
#  pedRefiner.py
#      Created by Hailin SU on 1/23/17.

import os
import sys
import logging
from collections import OrderedDict


# RECURSIVE VERSION
class PedRefiner:
    def __init__(self):
        self.isValid = False
        self.rec_gen_max = 0
        self.rec_gen = 0
        self.missing_in = '.'      # I code it as '.' inside
        self.missing_out = '.'
        self.stem = ""
        self.stem_fault = False
        fmt = "%(name)s %(asctime)s : %(levelname)-7s | %(message)s"
        date_fmt = "%H:%M:%S"
        logging.basicConfig(format=fmt, datefmt=date_fmt, level=logging.DEBUG)
        self.l = logging.getLogger('PedMap')
        # 20150702 mapXref
        self.mapXrefA = {}
        self.mapXrefS = {}
        self.mapXrefD = {}
        self.mapID2Gender = {}
        # 20150331 -r need mapID2Offspring
        self.mapID2SetOffspring = {}
        self.ped_map = {}     # holding original pedigree info
        self.opt_map = {}     # holding output pedigree set
#        self.opt_vec = []
#        self.opt_set = set()
        self.opt_set = OrderedDict()
        # self.l.info("PedRefiner().pipeline\n{}".format(self.pipeline.__doc__))

    def load_xref_map(self, file_name):
        """
        :param: file_name
        :return: bool, True if success
        """
        ret = False
        self.mapXrefA = {}
        self.mapXrefS = {}
        self.mapXrefD = {}
        if os.path.exists(file_name):
            ret = True
            with open(file_name, 'r') as fp:
                for line in fp:
                    vec_tmp = line.split()
                    if len(vec_tmp[0]) == 0 or vec_tmp[0][0] == "#":  # ignore empty or comment lines
                        continue
                    if len(vec_tmp) != 3:
                        self.l.error("PedMap::load_xref_map() ERROR - xref file {} is not a 3-col-through file."
                                     .format(file_name))
                        self.l.error("PedMap::load_xref_map() ERROR - line content [{}] contains {} fields."
                                     .format(line, len(vec_tmp)))
                        ret = False
                        break

                    if vec_tmp[0] == "A":
                        self.mapXrefA[vec_tmp[1]] = vec_tmp[2]
                    elif vec_tmp[0] == "S":
                        self.mapXrefS[vec_tmp[1]] = vec_tmp[2]
                    elif vec_tmp[0] == "D":
                        self.mapXrefD[vec_tmp[1]] = vec_tmp[2]
                    else:
                        self.l.warning('PedMap::loadXrefMap() WARNING - unknown command "{}", ignored'
                                       .format(line))

        return ret

    def load_ped(self, file_name, sep=",", missing_in='.', missing_out='.'):
        self.missing_in = missing_in
        self.missing_out = missing_out
        self.ped_map = {}
        if os.path.exists(file_name):
            with open(file_name, 'r') as fp:
                line_vec = fp.read().splitlines()

            self.isValid = True
            for line in line_vec:
                if not self.__load_line(line, sep):    # get rid of \n
                    self.isValid = False
                    break
        return self.isValid

    def __load_line(self, line, sep=","):
        ret = True
        len_x = 1 + line.count(sep)
        if len_x == 3:
            vec_tmp = line.split(sep)
            for i in range(3):
                idx = vec_tmp[i]
                if idx == self.missing_in:
                    idx = self.missing_out
                # 20150702 mapXref
                if idx in self.mapXrefA:   # vec_tmp[i] is in the xrefA map
                    idx = self.mapXrefA[idx]
                vec_tmp[i] = idx
            idx, sire, dam = vec_tmp
            if sire in self.mapXrefS:       # vec_tmp[1] is in the xrefS map
                sire = self.mapXrefS[sire]
            if dam in self.mapXrefD:       # vec_tmp[2] is in the xrefD map
                dam = self.mapXrefD[dam]

            if idx != self.missing_out:
                p = (sire, dam)
                if idx in p:
                    self.l.warning("  # I ID {} is same for its parent(s), replaced with missing.".format(idx))
                    if idx == sire:
                        sire = self.missing_out
                    if idx == dam:
                        dam = self.missing_out
                    self.ped_map[idx] = (sire, dam)

                if idx not in self.ped_map:            # new entry
                    self.ped_map[idx] = p
                # duplicated but same record, just return: no-missing stored first
                # 20150729: partly canceled. overriding using new entry
                elif self.ped_map[idx][0] == sire and self.ped_map[idx][1] == dam:  # same
                    self.l.warning("  # D duplicated (but same) entry for {}".format(idx))

                # 20150612: override empty entry
                # 20150729: canceled. overriding using new entry
                else:
                    self.l.warning("  # D duplicated (yet quite different) entry for {}, force using version2"
                                   .format(idx))
                    self.l.warning("       version1: {0}{2}{1[0]}{2}{1[1]}".format(idx, self.ped_map[idx], sep))
                    self.ped_map[idx] = p
                    self.l.warning("       version2: {0}{2}{1[0]}{2}{1[1]}".format(idx, self.ped_map[idx], sep))
        else:
            self.l.error("  * Error reading ped file: not a 3-col csv file, line {}".format(line))
            self.l.error("    number of columns read: {}".format(len_x))
            ret = False

        return ret

    def check(self):
        ret = True
        if not self.isValid:
            self.l.warning("  * Errors occurred while reading pedigree.")
            ret = False

        map_s2i = {}
        map_d2i = {}   # map Sire/Dam 2 Integer(count)
        self.mapID2Gender = {}

        # filling the sets
        # self.l.debug("filling maps[ID -> parents]")
        for k in self.ped_map:
            s, d = self.ped_map[k]
            if s not in map_s2i:  # new
                map_s2i[s] = 1
                self.mapID2Gender[s] = "M"
            else:
                map_s2i[s] += 1

            if d not in map_d2i:  # new
                map_d2i[d] = 1
                self.mapID2Gender[d] = "F"
            else:
                map_d2i[d] += 1

        # make sure the two sets excludes each other
        # inserting intersection of Sire and Dam
        # print out the intersection and its count to support what gender it was
        intersection = set(map_s2i.keys()).intersection(map_d2i.keys())
        if len(intersection) > 1:  # 1 is always self.missing_out
            self.l.warning("  * B Error: IDs appeared in both sire and dam columns. Use 'xref.CorrectB'" +
                           " as a xref file for the next run.")
            self.l.warning("    - creating 'xref.CorrectB'")
            ret = False
            with open('xref.CorrectB', 'w') as fp:
                for idx in intersection:
                    cnt_s = map_s2i[idx]
                    cnt_d = map_d2i[idx]
                    if cnt_s == cnt_d:
                        s_op = "A"
                    elif cnt_s > cnt_d:
                        s_op = "D"
                    else:
                        s_op = "S"

                    s_op = "{} {} {}\n".format(s_op, idx, self.missing_out)
                    fp.write(s_op)
        return ret

    def get_offspring_set(self, idx):
        ret = set()
        if idx in self.mapID2SetOffspring:
            ret = self.mapID2SetOffspring[idx]
        return ret

    # 20160331: a regular pedRefiner job
    def refine(self, list_file_name, opt_file_name, rec_gen_max=0, sep_out=",", flag_r=False):
        if os.path.exists(list_file_name):
            with open(list_file_name, 'r') as fp:
                anm_list = fp.read().splitlines()

            self.l.debug("{} individuals marked for populating result ped".format(len(anm_list)))
            self.__populate_opt_map(anm_list, rec_gen_max)  # filling result set: opt_map

            if self.stem_fault:
                self.l.error("  * L Pedigree loop detected while filling result set, stem = {}".format(self.stem))
            else:
                self.isValid = True
                self.l.debug("writing output")
                with open(opt_file_name, 'w') as fp:
                    for idx in self.opt_set:    # self.opt_vec:
                        fp.write("{0}{2}{1[0]}{2}{1[1]}\n".format(idx, self.opt_map[idx], sep_out))
        else:
            self.l.error(" * AnimalID list file {} could not be open to read.".format(list_file_name))
            self.isValid = False

        self.l.debug("refine() is returning [{}]".format(self.isValid))
        return self.isValid

    def __single_populate_opt_map(self, idx):
        # asserted idx!=self.missing_out
        if idx in self.opt_map and self.rec_gen_max == 0:  # already in output ped, and no recGen restriction
            return

        self.rec_gen += 1
        if self.rec_gen > self.rec_gen_max > 0:
            if idx not in self.opt_map:
                self.opt_map[idx] = (self.missing_out, self.missing_out)
            self.rec_gen -= 1
            return

        if idx in self.ped_map:     # in input ped
            sire, dam = self.ped_map[idx]
            if sire != self.missing_out:
                self.__single_populate_opt_map(sire)
            if dam != self.missing_out:
                self.__single_populate_opt_map(dam)

            # 20150629: prevent loop
            if sire == self.stem or dam == self.stem:
                self.stem_fault = True
        else:  # not found
            sire = dam = self.missing_out

        self.opt_map[idx] = (sire, dam)
        self.rec_gen -= 1

    def __populate_opt_map(self, anm_list, rec_gen_max=0):
        self.opt_map = {}
        self.opt_vec = []
        self.rec_gen_max = rec_gen_max
        for idx in anm_list:
            if idx != self.missing_out:
                self.stem = idx
                self.rec_gen = 1
                self.__single_populate_opt_map(idx)
        
        # make a sorted list in self.opt_vec
        self.l.debug("{} individuals in the result ped. sorting...".format(len(self.opt_map)))
        for idx in self.opt_map:
            self.__single_populate_opt_vec(idx)
            
    def __single_populate_opt_vec(self, idx):
        if idx in self.opt_set:
            return

        sire, dam = self.opt_map[idx]
        if sire != self.missing_out:
            self.__single_populate_opt_vec(sire)
        if dam != self.missing_out:
            self.__single_populate_opt_vec(dam)

        self.opt_set[idx] = None
        # if idx not in self.opt_set:
        #    self.opt_vec.append(idx)
        #    self.opt_set.add(idx)

    def pipeline(self, lst_fn, ped_fn, opt_fn, gen_max=0, missing_in='.', missing_out='.',
                 sep_in=',', sep_out=',', xref_fn=None, flag_r=False):
        """
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
        """
        if xref_fn is not None:
            self.l.debug('loading xref')
            self.load_xref_map(xref_fn)
        self.l.debug('loading ped and then check')
        ret_load = self.load_ped(ped_fn, sep_in, missing_in, missing_out)
        ret_check = self.check()
        if ret_load:
            if ret_check:
                self.refine(lst_fn, opt_fn, int(gen_max), sep_out, bool(flag_r))
            else:
                self.l.error('error checking pedigree')
        else:
            self.l.error('error loading input pedigree')

    def help(self):
        print(self.pipeline.__doc__)

if __name__ == "__main__":
    pr = PedRefiner()

    if len(sys.argv) == 1:
        pr.help()
    else:
        pr.pipeline(*sys.argv[1:])
    """
    # test on bovine
    pr.pipeline('/Volumes/data/epds/ASA_20150714_Carcass_analysis_w_BWCG/20161218-newRefine/lstID',  # lst_fn
                '/Volumes/data/epds/ASA_20150714_Carcass_analysis_w_BWCG/20161218-newRefine/pedAll.csv',
                '/Volumes/data/epds/ASA_20150714_Carcass_analysis_w_BWCG/20161218-newRefine/opt.pedpy.csv',
                3)
    """