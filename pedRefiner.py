#!/usr/bin/env python3

# -*- coding: utf-8 -*-
import os
import sys


def errprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class PedMap:
    def __init__(self, file_name, sep=",", flag_r=False):
        self.isValid = False

        # 20150702 mapXref
        self.mapXrefA = {}
        self.mapXrefS = {}
        self.mapXrefD = {}
        self.mapID2Gender = {}
        # 20150331 -r need mapID2Offspring
        self.mapID2SetOffspring = {}
        self.ped_map = {}
        self.load(file_name, sep, flag_r)

    def load_xref_map(self, file_name):
        """
        :param: file_name
        :return: bool, True if success
        """
        ret = False
        if os.path.exists(file_name):
            ret = True
            with open(file_name, 'r') as fp:
                for line in fp:
                    vec_tmp = line.split()
                    if len(vec_tmp[0]) == 0 or vec_tmp[0][0] == "#":  # ignore empty or comment lines
                        continue
                    if len(vec_tmp) != 3:
                        errprint("PedMap::load_xref_map() ERROR - xref file ", file_name,
                                 " is not a 3-col-through file.")
                        errprint("PedMap::load_xref_map() ERROR - line content [", line, "] contains ", len(vec_tmp),
                                 " fields.")
                        ret = False
                        break

                    if vec_tmp[0] == "A":
                        self.mapXrefA[vec_tmp[1]] = vec_tmp[2]
                    elif vec_tmp[0] == "S":
                        self.mapXrefS[vec_tmp[1]] = vec_tmp[2]
                    elif vec_tmp[0] == "D":
                        self.mapXrefD[vec_tmp[1]] = vec_tmp[2]
                    else:
                        errprint('PedMap::loadXrefMap() WARNING - unknown command "', line, '", ignored')

        return ret

    def load(self, file_name, sep=",", flag_r=False):
        if os.path.exists(file_name):
            with open(file_name, 'r') as fp:
                for line in fp:
                    self.isValid = self.load_line(line, sep, flag_r)
                    if not self.isValid:
                        break
        return self.isValid

    def load_line(self, line, sep=",", flag_r=False):
        ret = True
        vec_tmp = line.strip().split(sep)
        if len(vec_tmp) != 3:
            errprint(" * Error reading ped file: not a 3-col csv file, line starting with ", vec_tmp[0])
            errprint("   number of columns read: ", len(vec_tmp))
            ret = False
        else:
            for i in range(3):
                if len(vec_tmp[i]) == 0 or vec_tmp[i] == '0' or vec_tmp[i] == '.'\
                        or int(vec_tmp[i]) < 0:  # TODO: missing value
                    vec_tmp[i] = '.'
                # 20150702 mapXref
                if vec_tmp[i] in self.mapXrefA:   # vec_tmp[i] is in the xrefA map
                    vec_tmp[i] = self.mapXrefA[vec_tmp[i]]
            if vec_tmp[1] in self.mapXrefS:       # vec_tmp[1] is in the xrefS map
                vec_tmp[1] = self.mapXrefS[vec_tmp[1]]
            if vec_tmp[2] in self.mapXrefD:       # vec_tmp[1] is in the xrefS map
                vec_tmp[2] = self.mapXrefD[vec_tmp[2]]

            if flag_r and vec_tmp[0] != '.':     # only do this if -r is specified
                for i in range(1, 3):
                    if vec_tmp[i] == '.':
                        continue
                    if vec_tmp[i] not in self.mapID2SetOffspring:     # vec_tmp[1] is NOT in the mapID2Offspring map
                        self.mapID2SetOffspring[vec_tmp[i]] = set()   # initial the result set for it
                    self.mapID2SetOffspring[vec_tmp[i]].add(vec_tmp[0])  # put vec_tmp[0] as a offspring of vec_tmp[1]

        if vec_tmp[0] != '.' and (vec_tmp[0] == vec_tmp[1] or vec_tmp[0] == vec_tmp[2]):
            errprint(" #I ID ", vec_tmp[0], " is same for its parent(s), replaced with None.")
            if vec_tmp[0] == vec_tmp[1]:
                vec_tmp[1] = '.'
            if vec_tmp[0] == vec_tmp[2]:
                vec_tmp[2] = '.'
            self.ped_map[vec_tmp[0]] = (vec_tmp[1], vec_tmp[2])

        if vec_tmp[0] not in self.ped_map:            # new entry
            self.ped_map[vec_tmp[0]] = (vec_tmp[1], vec_tmp[2])
        # duplicated but same record, just return: no-missing stored first
        # 20150729: partly canceled. overriding using new entry
        elif self.ped_map[vec_tmp[0]][0] == vec_tmp[1] and self.ped_map[vec_tmp[0]][1] == vec_tmp[2]:  # same
            errprint(" #D duplicated (but same) entry for ", vec_tmp[0])

        # 20150612: override empty entry
        # 20150729: canceled. overriding using new entry
        else:
            errprint(" #D duplicated (yet quite different) entry for ", vec_tmp[0], ", force using version2")
            errprint("     version1: ", self.get_record(vec_tmp[0]))
            self.ped_map[vec_tmp[0]] = (vec_tmp[1], vec_tmp[2])
            errprint("     version2: ", self.get_record(vec_tmp[0]))
        return ret

    def check(self):
        ret = True
        if not self.isValid:
            errprint(" * Errors occurred while reading pedigree.")
            ret = False

        map_s2i = {}
        map_d2i = {}   # map Sire/Dam 2 Integer(count)
        # filling the sets
        errprint("   - filling maps[ID -> parents]")
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
        errprint("   - filling sire set and dam set")
        # inserting intersection of Sire and Dam
        # print out the intersection and its count to support what gender it was
        intersection = [x for x in set(map_s2i.keys()).intersection(set(map_d2i.keys())) if x != '.']
        if len(intersection):
            errprint(" *B Error: IDs appeared in both sire and dam columns. Use 'xref.CorrectB'",
                     "as a xref file for the next run.")
            errprint("   - creating 'xref.CorrectB'")
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

                    s_op = "{} {} 0\n".format(s_op, idx)
                    fp.write(s_op)
        return ret

    def get_record(self, idx):
        return "{0},{1[0]},{1[1]}".format(idx, self.ped_map[idx])

    def get_offspring_set(self, idx):
        ret = set()
        if idx in self.mapID2SetOffspring:
            ret = self.mapID2SetOffspring[idx]
        return ret
