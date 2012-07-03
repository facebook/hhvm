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

#include <runtime/ext/ext_filter.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DEFAULT_EXTENSION(filter);

const int64 k_FILTER_REQUIRE_ARRAY               =  0x1000000;
const int64 k_FILTER_REQUIRE_SCALAR              =  0x2000000;

const int64 k_FILTER_FORCE_ARRAY                 =  0x4000000;
const int64 k_FILTER_NULL_ON_FAILURE             =  0x8000000;

const int64 k_FILTER_FLAG_ALLOW_OCTAL            =  0x0001;
const int64 k_FILTER_FLAG_ALLOW_HEX              =  0x0002;

const int64 k_FILTER_FLAG_STRIP_LOW              =  0x0004;
const int64 k_FILTER_FLAG_STRIP_HIGH             =  0x0008;
const int64 k_FILTER_FLAG_ENCODE_LOW             =  0x0010;
const int64 k_FILTER_FLAG_ENCODE_HIGH            =  0x0020;
const int64 k_FILTER_FLAG_ENCODE_AMP             =  0x0040;
const int64 k_FILTER_FLAG_NO_ENCODE_QUOTES       =  0x0080;
const int64 k_FILTER_FLAG_EMPTY_STRING_NULL      =  0x0100;
const int64 k_FILTER_FLAG_STRIP_BACKTICK         =  0x0200;

const int64 k_FILTER_FLAG_ALLOW_FRACTION         =  0x1000;
const int64 k_FILTER_FLAG_ALLOW_THOUSAND         =  0x2000;
const int64 k_FILTER_FLAG_ALLOW_SCIENTIFIC       =  0x4000;

const int64 k_FILTER_FLAG_SCHEME_REQUIRED        =  0x010000;
const int64 k_FILTER_FLAG_HOST_REQUIRED          =  0x020000;
const int64 k_FILTER_FLAG_PATH_REQUIRED          =  0x040000;
const int64 k_FILTER_FLAG_QUERY_REQUIRED         =  0x080000;

const int64 k_FILTER_FLAG_IPV4                   =  0x100000;
const int64 k_FILTER_FLAG_IPV6                   =  0x200000;
const int64 k_FILTER_FLAG_NO_RES_RANGE           =  0x400000;
const int64 k_FILTER_FLAG_NO_PRIV_RANGE          =  0x800000;

const int64 k_FILTER_VALIDATE_INT                =  0x0101;
const int64 k_FILTER_VALIDATE_BOOLEAN            =  0x0102;
const int64 k_FILTER_VALIDATE_FLOAT              =  0x0103;

const int64 k_FILTER_VALIDATE_REGEXP             =  0x0110;
const int64 k_FILTER_VALIDATE_URL                =  0x0111;
const int64 k_FILTER_VALIDATE_EMAIL              =  0x0112;
const int64 k_FILTER_VALIDATE_IP                 =  0x0113;
const int64 k_FILTER_VALIDATE_LAST               =  0x0113;

const int64 k_FILTER_VALIDATE_ALL                =  0x0100;

const int64 k_FILTER_DEFAULT                     =  0x0204;
const int64 k_FILTER_UNSAFE_RAW                  =  0x0204;

const int64 k_FILTER_SANITIZE_STRING             =  0x0201;
const int64 k_FILTER_SANITIZE_ENCODED            =  0x0202;
const int64 k_FILTER_SANITIZE_SPECIAL_CHARS      =  0x0203;
const int64 k_FILTER_SANITIZE_EMAIL              =  0x0205;
const int64 k_FILTER_SANITIZE_URL                =  0x0206;
const int64 k_FILTER_SANITIZE_NUMBER_INT         =  0x0207;
const int64 k_FILTER_SANITIZE_NUMBER_FLOAT       =  0x0208;
const int64 k_FILTER_SANITIZE_MAGIC_QUOTES       =  0x0209;
const int64 k_FILTER_SANITIZE_FULL_SPECIAL_CHARS =  0x020a;
const int64 k_FILTER_SANITIZE_LAST               =  0x020a;
const int64 k_FILTER_SANITIZE_STRIPPED           =  0x020b;

const int64 k_INPUT_POST                        = 0;
const int64 k_INPUT_GET                         = 1;
const int64 k_INPUT_COOKIE                      = 2;
const int64 k_INPUT_ENV                         = 4;
const int64 k_INPUT_SERVER                      = 5; // not IMPLEMENT
const int64 k_INPUT_SESSION                     = 6; // not IMPLEMENT


Variant filterGetStorage (int64 type);
Variant f_filter_input(int64 type, CStrRef variable_name, int64 filter /* = 0 */, CVarRef options /* = 0 */) {
 // throw NotImplementedException(__func__);
    Variant filteredVar = filterGetStorage(type);

    if (filteredVar.is(KindOfNull) || !filteredVar.toArray().exists(String(variable_name))) {
        int64 filter_flags = 0L;

        if (options) {
            if (options.is(KindOfInt64)) {
                filter_flags = options.toInt64();
            } else if (options.is(KindOfArray) && options.toArray().exists(String("flags"))) {
                filter_flags = options["flags"].toInt64();
                //PHP_FILTER_GET_LONG_OPT(option, filter_flags);
            }

            if (options.is(KindOfArray)
                && options.toArray().exists(String("options"))
                && options[String("options")].toArray().exists(String("default"))) {
                return  options[String("options")]["default"];
            }
        }

        /* The FILTER_NULL_ON_FAILURE flag inverts the usual return values of
         * the function: normally when validation fails false is returned, and
         * when the input value doesn't exist NULL is returned. With the flag
         * set, NULL and false should be returned, respectively. Ergo, although
         * the code below looks incorrect, it's actually right. */
        if (filter_flags & k_FILTER_NULL_ON_FAILURE) {
            return false;
        } else {
            return null_variant;
        }
    }
    return filteredVar[variable_name].toString();
}

Variant filterGetStorage (int64 type) {
    SystemGlobals *g = (SystemGlobals*)get_global_variables();
    Variant ret = null_variant;
    switch(type){
    case k_INPUT_POST:
        ret = g->GV(_POST);
        break;
    case k_INPUT_GET:
        ret = g->GV(_GET);
        break;
    case k_INPUT_COOKIE:
        ret = g->GV(_COOKIE);
        break;
    case k_INPUT_ENV:
        ret = g->GV(_ENV);
        break;
    case k_INPUT_SERVER:
        ret = g->GV(_SERVER);
        break;
    case k_INPUT_SESSION:
        ret = g->GV(_SESSION);
        break;
    }
    return ret;
}
///////////////////////////////////////////////////////////////////////////////
}
