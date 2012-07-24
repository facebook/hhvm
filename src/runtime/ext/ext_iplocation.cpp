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

#include "ext_iplocation.h"
#include <map>
#include <fstream>
#include <runtime/base/ini_setting.h>
#include <runtime/ext/ext_network.h>
#include <util/logger.h>
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

   class IpLocKey {
    public:
        int64 s;
        int64 e;
        IpLocKey(int64 s, int64 e) {
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

    class IpLocMap {
    public:
        static std::map<IpLocKey,String> lMap;

        static std::string ipDataPath;

        static String find(int64 ip) {
            //use find method to prevent write  4 thread safety
            //do not use [] 4 thread safety
            //TODO more safefy
            std::map<IpLocKey,String>::iterator IpLocIter = IpLocMap::lMap.find(IpLocKey (ip, ip));
            if(IpLocIter != IpLocMap::lMap.end()){
                return IpLocIter->second;
            }
            return null_string;
        }

        static std::map<IpLocKey,String> createLMap (){
            std::map<IpLocKey,String> ret;
            return ret;
        }

        static void init() {
            IniSetting::Bind("location.data_file_path", "/home/admin/opt/ipdata.txt.gbk", ini_on_update_string, &ipDataPath);
            Logger::Info("load ip data " + ipDataPath);
            char buffer[1024];
            std::fstream out;
            out.open(ipDataPath.c_str(),std::ios::in);
            if(!out.is_open()) {
                raise_warning("error to load ip data" + ipDataPath);
                return;
            }
            const char * split = ",";
            while(!out.eof())
            {
                out.getline(buffer,1024,'\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
                char * p;
                p = strtok (buffer,split);
                int64 start=0L;
                int64 end=0L;
                int index=0;

                String loc;
                String  city;
                String  province;
                String  country;

                while(p!=NULL) {
                    if(index == 0) {
                        start=atol(p);
                    } else if(index == 1) {
                        end=atol(p);
                    } else if(index == 5){
                        province = String(p);
                    } else if(index == 6){
                        city=  String(p);
                    } else if(index == 7){
                        country= String(p);
                    }
                    p = strtok(NULL,split);
                    index++;
                }
                loc += province;
                loc += " ";
                loc += city;
                loc += " ";
                loc += country;
                IpLocKey k(start, end);
                lMap[k] = loc;
                //cout<<buffer<<endl;
            }
            out.close();
        }
    };

    std::map<IpLocKey,String> IpLocMap::lMap;
    std::string IpLocMap::ipDataPath;
    ///////////////////////////////////////////////////////////////////////////////
    String f_ip_get_location(CStrRef ip) {
        Variant ipint = f_ip2long(ip);
        if(same(ipint, false)) {
            raise_warning(ip + " is not a validate ip");
            return String("");
        }
        return IpLocMap::find(ipint.toInt64());
    }
    static class iplocationExtension : public Extension {
    public:
        iplocationExtension() : Extension("iplocation") {}
        virtual void moduleInit() {
            IpLocMap::init();
        }
    } s_iplocation_extension;
    ///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
}
