/*
 * Part of this file adopted codes from matvec, a free software published under
 *     the terms of the GNU Library General Public License version 2.
 *
 * Contributors of these codes (sorted in alphabetic order):
 *     David Habier
 *     Rohan L. Fernando
 *     Tianlin Wang
 */
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
//  splitablestring.h
//  libHailins
//
//  Created by Hailin SU on 10/2/13.

#ifndef libHailins_splitablestring_h
#define libHailins_splitablestring_h

#include<vector>
using namespace std;

class splitablestring : public string
{
private:
    vector<string> flds;
public:
    splitablestring(char *s)    : string(s)  { }
    splitablestring(string s)   : string(s)  { }
    splitablestring()           : string("") { }
    // split: receives a char delimiter; returns a vector of strings
    // By default ignores repeated delimiters, unless argument rep == 1.
    vector<string>& split(char delim, int rep=0)
    {
        // Algorithmed by http://www.cplusplus.com/articles/1UqpX9L8/
        if (flds.size())
        {
            vector<string>().swap(flds);  // empty vector if necessary
            flds.clear();
        }
        static string work;
        work = data();
        static string buf;
        buf = "";
        static unsigned i;
        i = 0;
        while (i < work.length())
        {
            if (work[i] != delim)   // if work[i] is still string content
                buf += work[i];
            else if (rep == 1)      // if work[i] is delim, and rep==1
            {
                flds.push_back(buf);
                buf = "";
            }
            else if (buf.length() > 0)  // if work[i] is delim, and rep==0 and buf not empty
            {
                flds.push_back(buf);
                buf = "";
            }
            i++;
        }
        if (buf.length() || (rep == 1 && work[i-1]==delim) )
            flds.push_back(buf);
        return flds;
    }

    vector<string>& split(string delims=" \t", bool rep=false)
    {
        /* Algorithmed from matvec
         *
         * Authors: Rohan L. Fernando
         *   this is essentially the split funciton from matvec
         *   I guess split was written by Tianlin Wang
         *   (2005)
         * Contributors:
         *   (2014) modified and added here by Hailin Su, Feb 11 10:13 AM
         */
        string::size_type begidx, endidx(0), nextidx;
        if (flds.size())
        {
            vector<string>().swap(flds);
            flds.clear();
//            vecDeb.swap(flds);  // empty vector if necessary
//            cout << "size" << vecDeb.size() << endl;
        }
        static string work;
        work = data();

        while ( (begidx=work.find_first_not_of(delims, endidx)) != string::npos)
        {
            if( (endidx=work.find_first_of(delims, begidx)) == string::npos)
                endidx = work.length();
            flds.push_back(work.substr(begidx, endidx-begidx));

            /* Algorithmed from matvec
             *
             * Authors: David Habier
             *   (2008)
             * Contributors:
             *   (2014) modified and added here by Hailin Su, Feb 11 10:13 AM
             */
            if(rep)
                do
                {
                    nextidx=work.find_first_of(delims, endidx+1);
                    if(nextidx==endidx+1)
                    {
                        flds.push_back(".");
                        ++endidx;
                    }
                } while(nextidx==endidx);
        }
        return flds;
    }

    /*
    splitablestring operator = (const char* s)
    {
        return splitablestring(s);
    }
    splitablestring operator = (const string& s)
    {
        return splitablestring(s);
    }*/
 };

#endif
