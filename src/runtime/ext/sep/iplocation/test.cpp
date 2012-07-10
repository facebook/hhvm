/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include <string>
#include <iostream>
#include <map>
#include <utility>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>

using namespace std;

class IpLocKey {
public:
    long s;
    long e;
    IpLocKey(long s, long e) {
        this->s = s;
        this->e = e;
    }

    bool operator<(const IpLocKey& k) const {

        if(this->s >= k.s && this->e <= k.e) {
            return 0;
        }

        if(k.s  >= this->s  && k.e <= this->e ) {
            return 0;
        }

        int s_cmp = this->e < k.s;

        if(s_cmp == 0)
        {
            return this->s < k.s;
        }

        return s_cmp;
    }
};

int main()
{
    const char *ipdataPath = "/usr/local/src/dev/source/dev/hiphop-php/src/runtime/ext/sep/iplocation/ipdata.txt.gbk";
    std::map<IpLocKey,std::string> IpLocMap;
    char buffer[1024];
    fstream out;
    out.open(ipdataPath,ios::in);
    const char * split = ",";
    while(!out.eof())
    {
        out.getline(buffer,1024,'\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
        char * p;
        p = strtok (buffer,split);
        long start=0L;
        long end=0L;
        int index=0;

        std::string loc;
        const char * city;
        const char * province;
        const char * country;

        while(p!=NULL) {
            if(index == 0) {
                start=atol(p);
            } else if(index == 1) {
                end=atol(p);
            } else if(index == 5){
                province= p;
            } else if(index == 6){
                city= p;
            } else if(index == 7){
                country=p;
            }
            p = strtok(NULL,split);
            index++;
        }
        loc.append(province);
        loc.append("|");
        loc.append(city);
        loc.append("|");
        loc.append(country);
        IpLocKey k(start, end);
        IpLocMap[k] = loc;
        //cout<<buffer<<endl;
    }
    out.close();


    IpLocKey p4 (3395420160L, 3395420160L);
    std::map <IpLocKey, std::string>::iterator IpLocIter = IpLocMap.find(p4);
    printf("%s", IpLocIter->second.c_str());
//cin.get() 是用来读取回车键的,如果没这一行，输出的结果一闪就消失了
   //IpLocKey p1 (45L, 100L);
   //IpLocKey p2 (10L, 30L);
   //IpLocKey p3 (30L, 45L);
   //IpLocKey p4 (10L, 10L);

   //std::map<IpLocKey,std::string> mapa;
   //std::map <IpLocKey, std::string>::iterator m1_Iter;
   //mapa[p1] = "Manzana";
   //mapa[p2] = "Arandano";
   //mapa[p3] = "tttt";
   //printf("%d\n",p4<p1);
   //printf("%d\n",p1<p4);

    //  p3 < p2;
    //
//printf(mapa.find(p4)->second.c_str());
    //for ( m1_Iter = mapa.begin( ); m1_Iter != mapa.end( ); m1_Iter++ ) {
    //      printf("mapa[%ld,%ld] --> %s\n", m1_Iter->first.s,m1_Iter->first.e,m1_Iter->second.c_str());
    //  }

    //printf("mapa[%ld,%ld] --> %s\n", p1.s,p1.e,mapa.begin()->second.c_str());
    //printf("mapa[%ld,%ld] --> %s\n", p2.s,p2.e,(mapa.next())->second.c_str());
    return 0;
}

