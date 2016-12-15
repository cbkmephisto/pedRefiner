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
//  pedRefiner.cpp
//  libHailins
//
//  Created by Hailin SU on 4/15/14.
//

#include "pedRefiner.h"

using namespace std;
using namespace libcbk;

unsigned recGen(0), recGenMax(0);

// 20150629: prevent loop
string stem;
set<string> stemSet;
bool stemFault(false);

void usage(char* argv0)
{
    cout << endl;
    cout << "pedRefiner          by hailins@iastate.edu" << endl << endl;
    cout << "[Usage]" << endl << endl
    << argv0 << " anmList full_ped_csv_file [ -r | num_recursive_gen | xref-filename] > pedOut.csv 2>errLog" << endl << endl;
    cout << "[Description]" << endl
    << " - extracts the pedigree of all the ancestors of the animals in the anmList file;" << endl
    << " - if '-r' option is given, prints out all the descendants' IDs from the anmList file instead of print out the refined pedigree;" << endl
    << " - xref the pedigree info if specified a whitespace-delimited 3-col xref file with 1st col as command:" << endl
    << "       #          : line starting with sharp will be ignored " << endl
    << "       A ID1 ID2  : all ID1 (col1, 2 and 3) will be changed to ID2" << endl
    << "       S ID1 ID2  : all ID1 in the sire (2nd) column will be changed to ID2" << endl
    << "       D ID1 ID2  : all ID1 in the dam  (3rd) column will be changed to ID2" << endl
    << " - force accepting the latest version if multiple entries for the same animal were detected;" << endl
    << " - messages print out on screen (stderr, 2> to redirect)" << endl
    << " - results  print out on screen (stdout, 1> to redirect)" << endl
    << "            ancestors ordered prior than offsprings, delimetered by comma ',' " << endl << endl;
    cout << "[Updates]" << endl
    << " - version 2016-03-31 added -r option: will print out all the descendants' IDs from anmList, recursively to the end" << endl //
    << " - version 2015-07-13 added output: pedOut.ID2Gender according to the ped, and 'M' for unknown" << endl //
    << " - version 2015-07-02 added xref to correct animal IDs: replace 1st col with 2nd col" << endl //
    << " - version 2015-06-29 prevent segmentation fault 11 due to pedigree loop" << endl //
    << " - version 2015-06-12 added updating pre-loaded empty entries" << endl //
    << " - version 2015-04-15 added a 3rd parameter to determine the recursive generations" << endl //
    << " - version 2014-11-10 modified check() so that all the errors would be checked in one run" << endl
    << "                      added sort()" << endl
    << " - version 2014-04-15 initial version" << endl //
    << endl;
}

void reCout(const string &anm, PedMap &pm, set<string> &resultSet)
{
    if(resultSet.find(anm)!=resultSet.end())    // already in
        return;
    if(recGenMax>0 && ++recGen>recGenMax)
    {
        pm[anm]=Parents("0", "0");
        --recGen;
        return;
    }

    resultSet.insert(anm);
    if(pm.find(anm)!=pm.end())  // found
    {
        if(pm[anm].sire!="0")
            reCout(pm[anm].sire, pm, resultSet);
        if(pm[anm].dam!="0")
            reCout(pm[anm].dam, pm, resultSet);
        // 20150629: prevent loop
        if(pm[anm].dam==stem || pm[anm].sire==stem)
            stemFault=true;
    }
    else                        // not found
        pm[anm]=Parents("0", "0");
    --recGen;
}

void reSort(const string &anm, PedMap &pm, vector<string> &vecSortedID, set<string> &setSortedID)
{
    stemSet.insert(anm);
    if(setSortedID.find(anm)==setSortedID.end())    // if not in the sorted list
    {
        if(pm[anm].sire!="0" && !stemFault)                       // make sure parents get in the vec first
        {
            if(stemSet.find(pm[anm].sire)!=stemSet.end())   // loop found
            {
                cerr << endl << "         loop detected: [s] " << pm[anm].sire << endl;
                stemFault=true;
                vecSortedID.push_back(anm);
                setSortedID.insert(anm);
                return;
            }
            reSort(pm[anm].sire, pm, vecSortedID, setSortedID);
        }
        if(pm[anm].dam!="0" && !stemFault)
        {
            if(stemSet.find(pm[anm].dam)!=stemSet.end())
            {
                cerr << endl << "         loop detected: [d] " << pm[anm].dam << endl;
                stemFault=true;
                vecSortedID.push_back(anm);
                setSortedID.insert(anm);
                return;
            }
            reSort(pm[anm].dam, pm, vecSortedID, setSortedID);
        }
        vecSortedID.push_back(anm);
        setSortedID.insert(anm);
    }
    stemSet.erase(anm);
}

void reFillOffspring(const string &anm, PedMap &pm, set<string> &rs)
{
    set<string> setTmp;
    if(rs.find(anm)!=rs.end())  // if anm already in resultset, it means all its descendants are in the resultset
        return;

    if(recGenMax>0 && ++recGen>recGenMax)
    {
        --recGen;
        return;
    }
    if(pm.getOffspringSet(anm, setTmp))
    {
        for(set<string>::iterator sit=setTmp.begin(); sit!=setTmp.end(); ++sit)
        {
            reFillOffspring(*sit, pm, rs);
            rs.insert(*sit);
        }
        set<string>().swap(setTmp);
    }
    --recGen;
    rs.insert(anm);
}

int main (const int argc, char** argv)
{
    if((argc==5 && string(argv[3])!="-r") || (argc!=3 && argc!=4 && argc!=5))
    {
        cout << argc << " parameters read.";
        usage(argv[0]);
        exit (-1);
    }
    string lstFile(argv[1]), tgtFile(argv[2]);

    // read the list
    ifstream infs;
    infs.open(lstFile.c_str());
    if(!infs)
    {
        cerr << " * AnimalID list file " << lstFile.c_str() << " could not be open to read." << endl;
        exit (-2);
    }

    vector<string> vecAnmLst;
    string stmp;
    cerr << " - Loading AnimalID list" << endl;
    while(infs >> stmp)
        vecAnmLst.push_back(stmp);

    infs.close();

    stringstream sst;
    splitablestring aLine;
    PedMap pm;

    // 20160331: bR
    bool bR(false);
    if(argc==5)
    {
        bR=true;
        sst << argv[4];
        sst >> recGenMax;
        sst.clear();
    }
    else if(argc==4)
    {
        infs.open(argv[3]);
        if(!infs)   // not a file, treat it as "-r" or recursive generations parameter
        {
            if(string(argv[3])=="-r")
                bR=true;
            else
            {
                sst << argv[3];
                sst >> recGenMax;
                sst.clear();
            }
        }
        else        // a file, read the xref content
        {
            infs.close();
            cerr << " - loading xref" << endl;
            if(!pm.loadXrefMap(argv[3]))
            {
                cerr << "Xref not loaded. Aborting..." << endl;
                infs.close();
                exit(-3);
            }
        }
    }

    // pipe the target
    infs.open(tgtFile.c_str());
    if(!infs)
    {
        cerr << " * Target file " << tgtFile.c_str() << " could not be open to read." << endl;
        exit (-3);
    }

    cerr << " - Loading whole pedigree" << endl;
    while(getline(infs, aLine))
        pm.load(aLine, bR);
    infs.close();

    cerr << " - Checking pedigree for errors" << endl;
    pm.check();

    cerr << " - Filling output result set" << endl;
    set<string> resultSet;

    if(!bR) // 20160331: if it is a regular pedRefiner job
    {
        for(unsigned i=0; i<vecAnmLst.size(); ++i)
        {
            if(i%500==0)
                cerr << "\r          \r" << 100*i/vecAnmLst.size() << "%";
            // 20150629: prevent ped loop
            stem=vecAnmLst[i];
            recGen=1;
            reCout(stem, pm, resultSet);   // recursion
            if(stemFault)
                cerr << "\r              \r *L Pedigree loop detected while filling result set, stem = " << stem << endl;
            stemFault=false;
        }
        cerr << "\r              \r";

        cerr << " - Sorting" << endl;   // 2014-11-10
        vector<string>  vecSortedID;
        set<string>     setSortedID;
        unsigned ls(0);
        bool stemFaultStop(false);
        for(set<string>::iterator sit=resultSet.begin(); sit!=resultSet.end(); ++sit)
        {
            if(ls++ % 500 == 0)
                cerr << "\r       \r" << 100*ls/resultSet.size() << "%";
            stem=(*sit);
            reSort(stem, pm, vecSortedID, setSortedID);   // recursion
            if(stemFault)
            {
                cerr << " * Pedigree loop detected while sorting, stem = " << stem << endl;
                stemFaultStop=true;
            }
            stemFault=false;
            set<string>().swap(stemSet);
        }
        if(stemFaultStop)
        {
            cerr << endl
            << "####################################################################" << endl
            << "[1] print the output pedigree (with loop) to stderr" << endl
            << "[2] write the output pedigree (with loop) to file 'pedOut.wLoop.csv'" << endl
            << "[X] or other input: quit" << endl
            << "### Your choice (default=X): " << flush;
            string choice;
            cin >> choice;
            if(choice=="1")
            {
                for(ls=0; ls<vecSortedID.size(); ++ls)
                    cerr << "      " << pm.getRecord(vecSortedID[ls]) << endl;
            }
            else if(choice=="2")
            {
                ofstream ofs ("pedOut.wLoop.csv");
                if(ofs)
                {
                    for(ls=0; ls<vecSortedID.size(); ++ls)
                        ofs << "      " << pm.getRecord(vecSortedID[ls]) << endl;
                    ofs.close();
                }
            }
            exit (-3);
        }
        cerr << "\r       \r";
        cerr << " - Outputing" << endl;

        ofstream ofsID2Gender("pedOut.ID2Gender");
        for(ls=0; ls<vecSortedID.size(); ++ls)
        {
            if(ls % 500 == 0)
                cerr << "\r       \r" << 100*ls/vecSortedID.size() << "%";
            cout << pm.getRecord(vecSortedID[ls]) << endl;
            if(ofsID2Gender)
            {
                if(pm.mapID2Gender.find(vecSortedID[ls])!=pm.mapID2Gender.end())    // in list
                    ofsID2Gender << vecSortedID[ls] << " " << pm.mapID2Gender[vecSortedID[ls]] << endl;
                else                // use U as unknown
                    ofsID2Gender << vecSortedID[ls] << " U" << endl;
            }
        }
        cerr << "\r       \r";
        if(ofsID2Gender)
            ofsID2Gender.close();
    }
    else    // 20160331: if it is a rev pedRefiner job -r
    {
        // put lst in first
        const unsigned long sz(vecAnmLst.size());
        for(unsigned i=0; i<sz; ++i)
        {
            if(i%200==0)
                cerr << "\r                       \r  [ " << i+1 << " / " << sz << " ]";
            recGen=1;
            reFillOffspring(vecAnmLst[i], pm, resultSet);
        }
        cerr << "\r                       \r";

        cerr << " - Outputing" << endl;
        for(set<string>::iterator sit = resultSet.begin(); sit!=resultSet.end(); ++sit)
            cout << *sit << endl;
    }
}
