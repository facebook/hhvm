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
#include <util/logger.h>
#include <runtime/base/zend/zend_url.h>

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


const int64 k_FILTER_SANITIZE_ALL               = 0x0200;
const int64 k_FILTER_CALLBACK                   = 0x0400;

const int64 k_INPUT_POST                        = 0;
const int64 k_INPUT_GET                         = 1;
const int64 k_INPUT_COOKIE                      = 2;
const int64 k_INPUT_ENV                         = 4;
const int64 k_INPUT_SERVER                      = 5; // not IMPLEMENT
const int64 k_INPUT_SESSION                     = 6; // not IMPLEMENT

#define MAX_LENGTH_OF_LONG 20
#define URL_ALLOW_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$-_.+!*'(),{}|\\^~[]`<>#%\";/?:@&="

Variant filterGetStorage (int64 type);
Variant php_filter_call(CVarRef filtered, int64 filter, CVarRef filter_args, const int copy, int64  filter_flags);
Variant php_zval_filter_recursive(CVarRef value, int64 filter, int64 flags, CVarRef options, CStrRef charset, int copy);
Variant php_zval_filter(CVarRef value, int64 filter, int64 flags, CVarRef options, CStrRef charset, int copy);

Variant php_filter_int(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_boolean(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_float(CVarRef value, int64 flags, CVarRef options, CStrRef charset);

Variant php_filter_validate_regexp(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_validate_url(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_validate_email(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_validate_ip(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_string(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_encoded(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_special_chars(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_full_special_chars(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_unsafe_raw(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_email(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_url(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_number_int(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_number_float(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_magic_quotes(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
Variant php_filter_callback(CVarRef value, int64 flags, CVarRef options, CStrRef charset);


typedef struct filter_list_entry {
	const char *name;
	int    id;
	Variant (*function)(CVarRef value, int64 flags, CVarRef options, CStrRef charset);
} filter_list_entry;

static const filter_list_entry filter_list[] = {
	{ "int",             k_FILTER_VALIDATE_INT,           php_filter_int             },
	{ "boolean",         k_FILTER_VALIDATE_BOOLEAN,       php_filter_boolean         },
	{ "float",           k_FILTER_VALIDATE_FLOAT,         php_filter_float           },

	{ "validate_regexp", k_FILTER_VALIDATE_REGEXP,        php_filter_validate_regexp },
	{ "validate_url",    k_FILTER_VALIDATE_URL,           php_filter_validate_url    },
	{ "validate_email",  k_FILTER_VALIDATE_EMAIL,         php_filter_validate_email  },
	{ "validate_ip",     k_FILTER_VALIDATE_IP,            php_filter_validate_ip     },

	{ "string",          k_FILTER_SANITIZE_STRING,        php_filter_string          },
	{ "stripped",        k_FILTER_SANITIZE_STRING,        php_filter_string          },
	{ "encoded",         k_FILTER_SANITIZE_ENCODED,       php_filter_encoded         },
	{ "special_chars",   k_FILTER_SANITIZE_SPECIAL_CHARS, php_filter_special_chars   },
	{ "full_special_chars",   k_FILTER_SANITIZE_FULL_SPECIAL_CHARS, php_filter_full_special_chars   },
	{ "unsafe_raw",      k_FILTER_UNSAFE_RAW,             php_filter_unsafe_raw      },
	{ "email",           k_FILTER_SANITIZE_EMAIL,         php_filter_email           },
	{ "url",             k_FILTER_SANITIZE_URL,           php_filter_url             },
	{ "number_int",      k_FILTER_SANITIZE_NUMBER_INT,    php_filter_number_int      },
	{ "number_float",    k_FILTER_SANITIZE_NUMBER_FLOAT,  php_filter_number_float    },
	{ "magic_quotes",    k_FILTER_SANITIZE_MAGIC_QUOTES,  php_filter_magic_quotes    },
	{ "callback",        k_FILTER_CALLBACK,               php_filter_callback        },
};


static filter_list_entry php_find_filter(int64 id)
{
	int i, size = sizeof(filter_list) / sizeof(filter_list_entry);

	for (i = 0; i < size; ++i) {
		if (filter_list[i].id == id) {
			return filter_list[i];
		}
	}
	/* Fallback to "string" filter */
	for (i = 0; i < size; ++i) {
		if (filter_list[i].id == k_FILTER_DEFAULT) {
			return filter_list[i];
		}
	}
	/* To shut up GCC */
	return filter_list[0];
}

Variant f_filter_input(int64 type, CStrRef variable_name, int64 filter /* = 0 */, CVarRef options /* = 0 */) {
 // throw NotImplementedException(__func__);
    Variant filteredVar = filterGetStorage(type);
    int64 filter_got = k_FILTER_DEFAULT;

    if(!filter == 0L ) {
        filter_got = filter;
    }

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
    return php_filter_call(filteredVar[variable_name], filter_got, options, 1, k_FILTER_REQUIRE_SCALAR);
    //return filteredVar[variable_name].toString();
}

//get the global variable
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

Variant php_filter_call(CVarRef filtered, int64 filter, CVarRef filter_args, const int copy, int64 filter_flags )
{
	Variant options = null;

	CStrRef charset = null;

	if (!filter_args.is(KindOfNull) && !filter_args.is(KindOfArray) ) {
		long lval;
                lval = filter_args.toInt64();

		if (filter != -1) {
                    /* handler for array apply */
			/* filter_args is the filter_flags */
			filter_flags = lval;

			if (!(filter_flags & k_FILTER_REQUIRE_ARRAY ||  filter_flags & k_FILTER_FORCE_ARRAY)) {
				filter_flags |= k_FILTER_REQUIRE_SCALAR;
			}
		} else {
			filter = lval;
		}

	} else if (!filter_args.is(KindOfNull)) {


                if (filter_args.is(KindOfArray) && filter_args.toArray().exists(String("filter"))) {
                        filter = filter_args.toArray()[String("filter")].toInt64();
		}


                if (filter_args.is(KindOfArray) && filter_args.toArray().exists(String("flags"))) {
                        filter_flags = filter_args.toArray()[String("flags")].toInt64();

			if (!(filter_flags & k_FILTER_REQUIRE_ARRAY ||  filter_flags & k_FILTER_FORCE_ARRAY)) {
				filter_flags |= k_FILTER_REQUIRE_SCALAR;
			}
		}

                if (filter_args.is(KindOfArray) && filter_args.toArray().exists(String("options"))) {
			if (filter != k_FILTER_CALLBACK) {
                            if(filter_args.toArray()[String("options")].is(KindOfArray)) {
                                options = filter_args.toArray()[String("options")].toArray();
                            }
			} else {
                                //TODO
				options = filter_args.toArray()[String("options")];
				filter_flags = 0;
			}
		}
	}

	if (filtered.is(KindOfArray)) {
		if (filter_flags & k_FILTER_REQUIRE_SCALAR) {
			if (filter_flags & k_FILTER_NULL_ON_FAILURE) {
                            return null_variant;
			} else {
                            return false;
			}
		}
                //TODO
		return php_zval_filter_recursive(filtered, filter, filter_flags, options, charset, copy);
	}

	if (filter_flags & k_FILTER_REQUIRE_ARRAY) {
            if (filter_flags & k_FILTER_NULL_ON_FAILURE) {
                return null_variant;
            } else {
                return false;
            }
            return null_variant;
	}

	Variant filtered_ret = php_zval_filter(filtered, filter, filter_flags, options, charset, copy);

	if (filter_flags & k_FILTER_FORCE_ARRAY) {
            if(!filtered_ret.is(KindOfArray)) {
                Array tmp = Array::Create();
                tmp.append(filtered_ret);
                return tmp;
            }
	}

        return filtered_ret;
}

Variant php_zval_filter_recursive(CVarRef value, int64 filter, int64 flags, CVarRef options, CStrRef charset, int copy)
{
    if(value.is(KindOfArray)) {
        Variant value_ret = value;
        for (ArrayIter iter(value); iter; ++iter) {
            Variant key = iter.first();
            Variant val = iter.second();
            Variant res;
            if(val.is(KindOfArray)) {
		res = php_zval_filter_recursive(iter.second(), filter, flags, options, charset, copy);
            } else {
                res = php_zval_filter(iter.second(), filter, flags, options, charset, copy);
            }
            value_ret.set(key, res);
        }
        return value_ret;
    } else {
        return php_zval_filter(value, filter, flags, options, charset, copy);
    }
}


Variant php_zval_filter(CVarRef value, int64 filter, int64 flags, CVarRef options, CStrRef charset, int copy)
{
    filter_list_entry  filter_func;

    filter_func = php_find_filter(filter);

    if (!filter_func.id) {
        /* Find default filter */
        filter_func = php_find_filter(k_FILTER_DEFAULT);
    }
    //if (copy) {
    //	SEPARATE_ZVAL(value);
    //}
    /* #49274, fatal error with object without a toString method
       Fails nicely instead of getting a recovarable fatal error. */
    if (value.is(KindOfObject)) {
        if(!f_method_exists(value, "__toString")){
            return false;
        }
    }

    /* Here be strings */
    //convert_to_string(*value);
    Variant value_ret = filter_func.function(value, flags, options, charset );
    if (options &&
                (options.is(KindOfArray) || options.is(KindOfObject) )
                && (flags & k_FILTER_NULL_ON_FAILURE && (value.is(KindOfNull)) ||
                    (!(flags & k_FILTER_NULL_ON_FAILURE) && value.is(KindOfBoolean) && value.toBoolean() == false)
                    )
                && options.toArray().exists(String("options"))
                && options[String("options")].toArray().exists(String("default"))) {
                return  options[String("options")]["default"];
            }
    return value_ret;
}


static int php_filter_parse_octal(CStrRef str , uint64 str_start, long *ret ) {

	unsigned long ctx_value = 0;
	//const char *end = str + str_len;
	uint64 length = str.size();

        for(uint64 i = str_start;i< length;i++ ) {
		if (str[i] >= '0' && str[i] <= '7') {
			unsigned long n = (str[i] - '0');

			if ((ctx_value > ((unsigned long)(~(long)0)) / 8) ||
				((ctx_value = ctx_value * 8) > ((unsigned long)(~(long)0)) - n)) {
				return -1;
			}
			ctx_value += n;
		} else {
			return -1;
		}
	}

	*ret = (long)ctx_value;
	return 1;
}

static int php_filter_parse_hex(CStrRef str, uint64 str_start, long *ret ) {

	unsigned long ctx_value = 0;
	uint64 length = str.size();
	unsigned long n;

        for(uint64 i = str_start;i< length;i++ ) {
            if (str[i] >= '0' && str[i] <= '9') {
                n = ((str[i]) - '0');
            } else if (str[i] >= 'a' && str[i] <= 'f') {
                n = (str[i]) - ('a' - 10);
            } else if (str[i] >= 'A' && str[i] <= 'F') {
                n = (str[i]) - ('A' - 10);
            } else {
                return -1;
            }

            if ((ctx_value > ((unsigned long)(~(long)0)) / 16) ||
                ((ctx_value = ctx_value * 16) > ((unsigned long)(~(long)0)) - n)) {
                return -1;
            }
            ctx_value += n;
        }

	*ret = (long)ctx_value;
	return 1;
}


static int php_filter_parse_int(CStrRef str, uint64 str_start , long *ret ) {
	long ctx_value;
	int sign = 0, digit = 0;
	//const char *end = str + str_len;
	uint64 length = str.size();

	switch (str[str_start]) {
		case '-':
			sign = 1;
		case '+':
                        str_start++;
		default:
			break;
	}

	/* must start with 1..9*/
	if (str_start  < length && str[str_start] >= '1' && str[str_start] <= '9') {
		ctx_value = ((sign)?-1:1) * ((str[str_start]) - '0');
                str_start++;
	} else {
		return -1;
	}


	if ((length - str_start  > MAX_LENGTH_OF_LONG - 1) /* number too long */
	 || (SIZEOF_LONG == 4 && (length - str_start == MAX_LENGTH_OF_LONG - 1) && str[str_start] > '2')) {
		/* overflow */
		return -1;
	}


        for(uint64 i = str_start;i< length;i++ ) {
		if (str[i] >= '0' && str[i] <= '9') {
			digit = (str[i] - '0');

			if ( (!sign) && ctx_value <= (LONG_MAX-digit)/10 ) {
				ctx_value = (ctx_value * 10) + digit;
			} else if ( sign && ctx_value >= (LONG_MIN+digit)/10) {
				ctx_value = (ctx_value * 10) - digit;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
	}

	*ret = ctx_value;
	return 1;
}

Variant php_filter_int(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
	//zval **option_val;
	int64  min_range=0, max_range=0, option_flags;
	int    min_range_set=0, max_range_set=0;
	int    allow_octal = 0, allow_hex = 0;
	int    len, error = 0;
	long   ctx_value;

        CStrRef str_value = value.toString();
	String p;

	/* Parse options */
        if(options) {
            if( options.toArray().exists(String("min_range"))
               && options[String("min_range")]) {
                min_range_set = 1;
                min_range = options[String("min_range")].toInt64();
            }
            if( options.toArray().exists(String("max_range"))
               && options[String("max_range")]) {
                max_range_set = 1;
                max_range = options[String("max_range")].toInt64();
            }
        }

	option_flags = flags;
        len  = str_value.size();
	if (len == 0) {
            if (flags & k_FILTER_NULL_ON_FAILURE) {
                return null_variant;
            } else {
                return false;
            }
	}

	if (option_flags & k_FILTER_FLAG_ALLOW_OCTAL) {
		allow_octal = 1;
	}

	if (option_flags & k_FILTER_FLAG_ALLOW_HEX) {
		allow_hex = 1;
	}

	/* Start the validating loop */
        p = f_trim(value);
	ctx_value = 0;
        if (p.length() == 0) {
            if (flags & k_FILTER_NULL_ON_FAILURE) {
                return null_variant;
            } else {
                return false;
            }
        }

        uint64 parse_start = 0;

	if (p[0] == '0') {
                parse_start++;
		if (allow_hex && (p[1] == 'x' || p[1] == 'X')) {
                        parse_start++;
			if (php_filter_parse_hex(p,parse_start, &ctx_value) < 0) {
				error = 1;
			}
		} else if (allow_octal) {
			if (php_filter_parse_octal(p, parse_start, &ctx_value) < 0) {
                                //Logger::Info("filterd=%d" , php_filter_parse_hex(p,parse_start, &ctx_value));
				error = 1;
			}
		} else if (len-parse_start != 0) {
			error = 1;
		}
	} else {
		if (php_filter_parse_int(p, parse_start, &ctx_value) < 0) {
			error = 1;
		}
	}

	if (error > 0 || (min_range_set && (ctx_value < min_range)) || (max_range_set && (ctx_value > max_range))) {
            if (flags & k_FILTER_NULL_ON_FAILURE) {
                return null_variant;
            } else {
                return false;
            }
	} else {
            return Variant(ctx_value);
        }
}

Variant php_filter_boolean(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {

    String str = f_trim(value.toString());
    int len = str.size();
    int ret;
    /* returns true for "1", "true", "on" and "yes"
     * returns false for "0", "false", "off", "no", and ""
     * null otherwise. */
    switch (len) {
    case 1:
        if (str[0] == '1') {
            ret = 1;
        } else if (str[0] == '0') {
            ret = 0;
        } else {
            ret = -1;
        }
        break;
    case 2:
        if (strncasecmp(str, "on", 2) == 0) {
            ret = 1;
        } else if (strncasecmp(str, "no", 2) == 0) {
            ret = 0;
        } else {
            ret = -1;
        }
        break;
    case 3:
        if (strncasecmp(str, "yes", 3) == 0) {
            ret = 1;
        } else if (strncasecmp(str, "off", 3) == 0) {
            ret = 0;
        } else {
            ret = -1;
        }
        break;
    case 4:
        if (strncasecmp(str, "true", 4) == 0) {
            ret = 1;
        } else {
            ret = -1;
        }
        break;
    case 5:
        if (strncasecmp(str, "false", 5) == 0) {
            ret = 0;
        } else {
            ret = -1;
        }
        break;
    default:
        ret = -1;
    }

    if (ret == -1) {
        if (flags & k_FILTER_NULL_ON_FAILURE) {
            return null_variant;
        } else {
            return false;
        }
    } else {
        if(ret == 0){
            return false;
        } else {
            return true;
        }
    }
}

Variant php_filter_float(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
        String str = f_trim(value.toString());
        uint64 len = str.size();
	char *num, *p;

	String decimal;
	int decimal_set=0;
	char dec_sep = '.';
	const char * tsd_sep = "',.";

	int64 lval = 0L;
	double dval = 0;

	int first, n;


        uint64 str_start = 0;

        if(options && options.toArray().exists(String("decimal"))
               && options[String("decimal")]) {
                decimal_set = 1;
                decimal = options[String("decimal")].toString();
        }

	if (decimal_set) {
		if (decimal.size() != 1) {
                        Logger::Warning("decimal separator must be one char");
                        if (flags & k_FILTER_NULL_ON_FAILURE) {
                            return null_variant;
                        } else {
                            return false;
                        }
		} else {
			dec_sep = decimal[0];
		}
	}

	num = p = (char *)malloc(len+1);
        memset(p,0, len+1);
	if (len > 0  && (str[str_start] == '+' || str[str_start] == '-')) {
		*p++ = str[str_start];
                str_start++;
	}

	first = 1;

	while (1) {
		n = 0;
                for(uint64 i=str_start;i<len;i++) {
                    if(str[i] > '0' && str[i] <='9') {
                        ++n;
                        *p++ = str[i];
                        str_start++;
                    } else {
                        break;
                    }
                }


                if (str_start == (len - 1) || str[str_start] == dec_sep || str[str_start] == 'e' || str[str_start] == 'E') {
			if (!first && n != 3) {
				goto error;
			}
			if (str[str_start] == dec_sep) {
				*p++ = '.';
				str_start++;

                                for(uint64 i=str_start;i<len;i++) {
                                    if(str[i] < '0' && str[i] > '9') {
                                        break;
                                    }
                                    *p++ = str[i];
                                    str_start++;
                                }
			}

			if (str[str_start] == 'e' || str[str_start] == 'E') {
				*p++ = str[str_start];
                                str_start++;

				if (str_start < (len - 1 ) && (str[str_start] == '+' || str[str_start] == '-')) {
                                        *p++ = str[str_start];
                                        str_start++;
				}
                                for(uint64 i=str_start;i<len;i++) {
                                    if(str[i] < '0' && str[i] > '9') {
                                        break;
                                    }
                                    *p++ = str[i];
                                    str_start++;
				}
			}
			break;
		}
		if ((flags & k_FILTER_FLAG_ALLOW_THOUSAND) && (str[str_start] == tsd_sep[0] || str[str_start] == tsd_sep[1] || str[str_start] == tsd_sep[2])) {
			if (first?(n < 1 || n > 3):(n != 3)) {
				goto error;
			}
			first = 0;
			str_start++;
		} else {
			goto error;
		}
	}

     	if (str_start != len) {
		goto error;
	}


	*p = 0;
	switch (is_numeric_string(num, p - num, &lval, &dval, 0)) {
		case KindOfInt64:
                        free(num);
                        return lval;
			break;
		case KindOfDouble:
			if ((!dval && p - num > 1 && strpbrk(num, "123456789")) || !finite(dval)) {
				goto error;
			}
                        free(num);
                        return dval;
			break;
		default:
                    error:
			free(num);
                        if (flags & k_FILTER_NULL_ON_FAILURE) {
                            return null_variant;
                        } else {
                            return false;
                        }
        }

    free(num);
    return value;
}

Variant php_filter_validate_regexp(CVarRef value, int64 flags, CVarRef options, CStrRef charset){
        String str = f_trim(value.toString());
	String regexp;
	long   option_flags;
	int    regexp_set=0, option_flags_set=0;
	pcre       *re = NULL;
	pcre_extra *pcre_extra = NULL;
	int preg_options = 0;

	int         ovector[3];
	int         matches;

	/* Parse options */
        if(options && options.toArray().exists(String("regexp"))
               && options[String("regexp")]) {
                regexp_set = 1;
                regexp = options[String("regexp")].toString();
        }

        if(options && options.toArray().exists(String("flags"))
           && options[String("flags")]) {
            option_flags_set = 1;
            option_flags = options[String("flags")].toInt64();
        }

	if (!regexp_set) {
                Logger::Warning("'regexp' option missing");
                if (flags & k_FILTER_NULL_ON_FAILURE) {
                    return null_variant;
                } else {
                    return false;
                }
	}
        //TODO
        //re = pcre_get_compiled_regex(regexp, &pcre_extra, &preg_options );

	if (!re) {
            if (flags & k_FILTER_NULL_ON_FAILURE) {
                return null_variant;
            } else {
                return false;
            }
	}
        //TODO
	//matches = pcre_exec(re, NULL, str, str.size() , 0, 0, ovector, 3);

	/* 0 means that the vector is too small to hold all the captured substring offsets */
	if (matches < 0) {
            if (flags & k_FILTER_NULL_ON_FAILURE) {
                return null_variant;
            } else {
                return false;
            }
	}
    return str;
}

Variant php_filter_validate_url(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
	//php_url *url;
        /**
	int old_len = value.toString().size();

	php_filter_url(value, flags, option_array, charset TSRMLS_CC);

	if (Z_TYPE_P(value) != IS_STRING || old_len != Z_STRLEN_P(value)) {
		RETURN_VALIDATION_FAILED
	}
	// Use parse_url - if it returns false, we return NULL
	url = php_url_parse_ex(Z_STRVAL_P(value), Z_STRLEN_P(value));

	if (url == NULL) {
		RETURN_VALIDATION_FAILED
	}

	if (url->scheme != NULL && (!strcasecmp(url->scheme, "http") || !strcasecmp(url->scheme, "https"))) {
		char *e, *s;

		if (url->host == NULL) {
			goto bad_url;
		}

		e = url->host + strlen(url->host);
		s = url->host;

		// First char of hostname must be alphanumeric
		if(!isalnum((int)*(unsigned char *)s)) {
			goto bad_url;
		}

		while (s < e) {
			if (!isalnum((int)*(unsigned char *)s) && *s != '-' && *s != '.') {
				goto bad_url;
			}
			s++;
		}

		if (*(e - 1) == '.') {
			goto bad_url;
		}
	}

	if (
		url->scheme == NULL ||
	// some schemas allow the host to be empty
		(url->host == NULL && (strcmp(url->scheme, "mailto") && strcmp(url->scheme, "news") && strcmp(url->scheme, "file"))) ||
		((flags & FILTER_FLAG_PATH_REQUIRED) && url->path == NULL) || ((flags & FILTER_FLAG_QUERY_REQUIRED) && url->query == NULL)
	) {
bad_url:
		php_url_free(url);
		RETURN_VALIDATION_FAILED
	}
	php_url_free(url);
        */
    return value;
}
Variant php_filter_validate_email(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}

Variant php_filter_validate_ip(CVarRef value, int64 flags, CVarRef options, CStrRef charset){
    return value;
}
Variant php_filter_string(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_encoded(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_special_chars(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_full_special_chars(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_unsafe_raw(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_email(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_url(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}

Variant php_filter_number_int(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_number_float(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_magic_quotes(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}
Variant php_filter_callback(CVarRef value, int64 flags, CVarRef options, CStrRef charset) {
    return value;
}

///////////////////////////////////////////////////////////////////////////////
}
