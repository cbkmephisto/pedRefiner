/*
 * Copyright (c) 2016 Hailin Su, ISU NBCEC
 *
 * This file is part of pedRefiner.
 *
 * pedRefiner is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pedRefiner is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Lesser Public License
 * along with pedRefiner. If not, see <http://www.gnu.org/licenses/>.
 */

//
//  pedRefiner.h
//  libHailins
//
//  Created by Hailin SU on 4/15/14.
//

#ifndef __libHailins__pedRefiner__
#define __libHailins__pedRefiner__

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <set>
#include <cstdlib>
#include <vector>
#include <map>
#include "splitablestring.h"

namespace libcbk
{
    class Parents
    {
    public:
        Parents(){}
        Parents(const string s, const string d)
        : sire(s), dam(d) {}
        string sire;
        string dam;
    };

    class PedMap : public map<string, Parents>
    {
    public:
        unsigned i;
        bool isValid;
        // 20150702 mapXref
        map<string, string> mapXrefA;
        map<string, string> mapXrefS;
        map<string, string> mapXrefD;
        map<string, string> mapID2Gender;
        // 20150331 -r need mapID2Offspring
        map<string, set<string> > mapID2SetOffspring;

        PedMap()
        {
            isValid=true;
        }

        bool loadXrefMap(string fN)
        {
            bool ret=true;
            ifstream infs(fN.c_str());
            if(!infs)
                ret=false;
            else        // a file, read the xref content
            {
                vector<string> vtmp;
                splitablestring aLine;
                while(getline(infs, aLine))
                {
                    vtmp=aLine.split();
                    if(vtmp[0].length()==0 || vtmp[0]=="#") // ignore empty or comment lines
                        continue;
                    if(vtmp.size()!=3)
                    {
                        cerr << "PedMap::loadXrefMap() ERROR - xref file " << fN.c_str() << " is not a 3-col-through file." << endl;
                        cerr << "PedMap::loadXrefMap() ERROR - line content [" << aLine << "] contains " << vtmp.size() << " fields." << endl;
                        ret=false;
                        break;
                    }
                    if(vtmp[0]=="A")
                        mapXrefA[vtmp[1]]=vtmp[2];
                    else if(vtmp[0]=="S")
                        mapXrefS[vtmp[1]]=vtmp[2];
                    else if(vtmp[0]=="D")
                        mapXrefD[vtmp[1]]=vtmp[2];
                    else
                        cerr << "PedMap::loadXrefMap() WARNING - unknown command \"" << aLine << "\", ignored" << endl;
                }
                infs.close();
                vector<string>().swap(vtmp);
            }
            return ret;
        }

        void load(splitablestring &aLine, bool bR)
        {
            vtmp=aLine.split(',', 1);
            if(vtmp.size()!=3)
            {
                cerr << " * Error reading ped file: not a 3-col csv file, line starting with " << vtmp[0] << endl;
                cerr << "   number of columns read: " << vtmp.size() << endl;
                exit (0);
            }
            else
            {
                for(i=0; i<3; ++i)
                {
                    if(vtmp[i].length()<=1)
                        vtmp[i]="0";
                    // 20150702 mapXref
                    if(mapXrefA.find(vtmp[i])!=mapXrefA.end())    // vtmp[i] is in the xrefA map
                        vtmp[i]=mapXrefA[vtmp[i]];
                }
                if(mapXrefS.find(vtmp[1])!=mapXrefS.end())    // vtmp[1] is in the xrefS map
                    vtmp[1]=mapXrefS[vtmp[1]];
                if(mapXrefD.find(vtmp[2])!=mapXrefD.end())    // vtmp[1] is in the xrefS map
                    vtmp[2]=mapXrefD[vtmp[2]];

                if(bR && vtmp[0]!="0")  // only do this if -r is specified
                {
                    for(i=1; i<3; ++i)
                    {
                        if(vtmp[i]=="0")
                            continue;
                        if(mapID2SetOffspring.find(vtmp[i])==mapID2SetOffspring.end())      // vtmp[1] is NOT in the mapID2Offspring map
                            mapID2SetOffspring[vtmp[i]]=set<string>();                      // initial the result set for it
                        mapID2SetOffspring[vtmp[i]].insert(vtmp[0]);                        //    put vtmp[0] as a offspring of vtmp[1]
                    }
                }
            }
            if(vtmp[0]=="0")    // ignore empty entry
                return;
            else if( vtmp[0]==vtmp[1] || vtmp[0]==vtmp[2] )
            {
                cerr << " #I ID " << vtmp[0] << " is same for its parent(s), replaced with 0." << endl;
                if(vtmp[0]==vtmp[1])
                    vtmp[1]="0";
                if(vtmp[0]==vtmp[2])
                    vtmp[2]="0";
                (*this)[vtmp[0]]=Parents(vtmp[1], vtmp[2]);
            }

            if(this->find(vtmp[0])==this->end())            // new entry
                (*this)[vtmp[0]]=Parents(vtmp[1], vtmp[2]);
            else if( // duplicated but same record, just return: no-missing stored first
                    // 20150729: partly canceled. overriding using new entry
                    ((*this)[vtmp[0]].sire==vtmp[1] && (*this)[vtmp[0]].dam==vtmp[2]))                   // totally the same
//                    ||  ((*this)[vtmp[0]].sire==vtmp[1] && vtmp[2]=="0")                                    // same sire missing dam
//                    ||  ((*this)[vtmp[0]].dam ==vtmp[2] && vtmp[1]=="0")                                    // same dam  missing sire
//                    ||  (              vtmp[1]==vtmp[2] && vtmp[1]=="0"))                                   // new entry missing both
            {
                cerr << " #D duplicated (but same) entry for " << vtmp[0] << endl;
            }
            // 20150612: override empty entry
            // 20150729: canceled. overriding using new entry
//            else if( // duplicated but same record, update: missing stored first
//                    ((*this)[vtmp[0]].sire=="0" && (*this)[vtmp[0]].dam=="0" && (vtmp[1]!="0" || vtmp[2]!="0")) // both missing, but new record non-missing
//                    ||  ((*this)[vtmp[0]].sire==vtmp[1] && (*this)[vtmp[0]].dam=="0" && vtmp[2]!="0")       // same sire missing dam and new dam not missing
//                    ||  ((*this)[vtmp[0]].dam ==vtmp[2] && (*this)[vtmp[0]].sire=="0" && vtmp[1]!="0"))     // same dam  missing sire
//            {
//                cerr << " #D duplicated (but more  detailed) entry for " << vtmp[0] << ", UPDATED" << endl;
//                (*this)[vtmp[0]].sire=vtmp[1];
//                (*this)[vtmp[0]].dam =vtmp[2];
//            }
            else
            {
                cerr << " #D duplicated (yet quite diffrent) entry for " << vtmp[0] << ", force using version2" << endl;
                cerr << "     version1: " << getRecord(vtmp[0]) << endl;
                (*this)[vtmp[0]]=Parents(vtmp[1], vtmp[2]);
                cerr << "     version2: " << getRecord(vtmp[0]) << endl;
//                isValid=false;
            }
        }

        void check()
        {
            if(!isValid)
            {
                cerr << " * Errors occurred while reading pedigree." << endl;
            }
            map<string, unsigned> mapS2I, mapD2I;   // map Sire/Dam 2 Integer(count)
            set<string> setI;                       // used to find the intesection
            // filling the sets
            cerr << "   - filling maps[ID -> parents]" << endl;
            for(map<string, Parents>::iterator mit=this->begin(); mit!=this->end(); ++mit)
            {
                if(mapS2I.find(mit->second.sire)==mapS2I.end())  // new
                    mapS2I[mit->second.sire]=1;
                else
                    mapS2I[mit->second.sire]+=1;

                if(mapD2I.find(mit->second.dam)==mapD2I.end())  // new
                    mapD2I[mit->second.dam]=1;
                else
                    mapD2I[mit->second.dam]+=1;

//                setS.insert(mit->second.sire);
//                setD.insert(mit->second.dam);
            }
            // make sure the two sets excludes each other
            cerr << "   - filling sire set and dam set" << endl;
            map<string, unsigned>::iterator mit;
            // inserting intersection of Sire and Dam
            for(mit=mapS2I.begin(); mit!=mapS2I.end(); ++mit)
                if(mapD2I.find(mit->first)!=mapD2I.end()) // id from sire set appeared in dam set, quit
                    if(mit->first!="0")   // skip *sit==0: nonsense
                    {
                        cerr << " *B Error: ID " << mit->first << " appeared in both sire and dam columns." << endl;
                        setI.insert(mit->first);
                    }
            // don't need the opsite way: the intersection is what it is
//            for(sit=setD.begin(); sit!=setD.end(); ++sit)
//                if(setS.find(*sit)!=setS.end()) // id from dam set appeared in sire set, quit
//                {
//                    cerr << " *S Error: dam ID " << (*sit) << " appeared in sire column." << endl;
//                    isValid=false;
//                }

            // print out the intersection and its count to support what gender it was
            if(setI.size())
            {
                cerr << " *B Error: IDs appeared in both sire and dam columns. Use 'xref.CorrectB' as a xref file for the next run." << endl;
//                cerr << right << setfill(' ')
//                << setw(32) << "ID"
//                << setw(16) << "Count_Sire"
//                << setw(16) << "Count_Dam" << endl;
                cerr << "   - creating 'xref.CorrectB'" << endl;
                bool bOfs(true);
                ofstream ofs("xref.CorrectB");
                if(!ofs)
                    bOfs=false; // see if write to file or to screen

                string sID, sOP;
                unsigned cntS, cntD;
                for(set<string>::iterator sit=setI.begin(); sit!=setI.end(); ++sit)
                {
                    sID=*sit;
                    cntS=mapS2I[sID];
                    cntD=mapD2I[sID];
//                    cerr << right << setfill(' ')
//                    << setw(32) << sID
//                    << setw(16) << cntS
//                    << setw(16) << cntD << endl;
                    // suggestion for xref:
                    //  cntS==cntD: A -> 0
                    //  cntS>cntD:  D -> 0
                    //  cntS<cntD:  S -> 0
                    if(cntS==cntD)
                        sOP="A";
                    else if(cntS>cntD)
                        sOP="D";
                    else
                        sOP="S";
                    sOP.append(" ");
                    sOP.append(sID);
                    sOP.append(" 0");
                    if(!bOfs)
                        cerr << sOP << endl;
                    else
                        ofs << sOP << endl;
                }
                if(bOfs)
                    ofs.close();
                exit (-6);
            }
            // filling mapID2Gender
            for(mit=mapS2I.begin(); mit!=mapS2I.end(); ++mit)
                mapID2Gender[mit->first]="M";
            for(mit=mapD2I.begin(); mit!=mapD2I.end(); ++mit)
                mapID2Gender[mit->first]="F";
        }

        string getRecord(string id)
        {
            static string ret;
            ret=id+","+(*this)[id].sire+","+(*this)[id].dam;
            return ret;
        }

        bool getOffspringSet(string id, set<string> &rs)
        {
            bool ret(true);
            static set<string>::iterator sit;
            if(mapID2SetOffspring.find(id)==mapID2SetOffspring.end())   // no offspring in record
                ret=false;
            else
            {
                for(sit=mapID2SetOffspring[id].begin(); sit!=mapID2SetOffspring[id].end(); ++sit)
                    rs.insert(*sit);
            }
            return ret;
        }
    private:
        vector<string> vtmp;
    };
}
#endif /* defined(__libHailins__pedRefiner__) */
