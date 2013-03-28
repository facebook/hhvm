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
namespace HPHP {

/*
bool HPHP::f_checkdate(int, int, int)
_ZN4HPHP11f_checkdateEiii

(return value) => rax
month => rdi
day => rsi
year => rdx
*/

bool fh_checkdate(int month, int day, int year) asm("_ZN4HPHP11f_checkdateEiii");

/*
HPHP::Object HPHP::f_date_add(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP10f_date_addERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_add(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP10f_date_addERKNS_6ObjectES2_");

/*
HPHP::Object HPHP::f_date_create_from_format(HPHP::String const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP25f_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE

(return value) => rax
_rv => rdi
format => rsi
time => rdx
timezone => rcx
*/

Value* fh_date_create_from_format(Value* _rv, Value* format, Value* time, Value* timezone) asm("_ZN4HPHP25f_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE");

/*
HPHP::Object HPHP::f_date_create(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP13f_date_createERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
time => rsi
timezone => rdx
*/

Value* fh_date_create(Value* _rv, Value* time, Value* timezone) asm("_ZN4HPHP13f_date_createERKNS_6StringERKNS_6ObjectE");

/*
void HPHP::f_date_date_set(HPHP::Object const&, int, int, int)
_ZN4HPHP15f_date_date_setERKNS_6ObjectEiii

object => rdi
year => rsi
month => rdx
day => rcx
*/

void fh_date_date_set(Value* object, int year, int month, int day) asm("_ZN4HPHP15f_date_date_setERKNS_6ObjectEiii");

/*
HPHP::String HPHP::f_date_default_timezone_get()
_ZN4HPHP27f_date_default_timezone_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_default_timezone_get(Value* _rv) asm("_ZN4HPHP27f_date_default_timezone_getEv");

/*
bool HPHP::f_date_default_timezone_set(HPHP::String const&)
_ZN4HPHP27f_date_default_timezone_setERKNS_6StringE

(return value) => rax
name => rdi
*/

bool fh_date_default_timezone_set(Value* name) asm("_ZN4HPHP27f_date_default_timezone_setERKNS_6StringE");

/*
HPHP::Object HPHP::f_date_diff(HPHP::Object const&, HPHP::Object const&, bool)
_ZN4HPHP11f_date_diffERKNS_6ObjectES2_b

(return value) => rax
_rv => rdi
datetime => rsi
datetime2 => rdx
absolute => rcx
*/

Value* fh_date_diff(Value* _rv, Value* datetime, Value* datetime2, bool absolute) asm("_ZN4HPHP11f_date_diffERKNS_6ObjectES2_b");

/*
HPHP::String HPHP::f_date_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_date_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
object => rsi
format => rdx
*/

Value* fh_date_format(Value* _rv, Value* object, Value* format) asm("_ZN4HPHP13f_date_formatERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Array HPHP::f_date_get_last_errors()
_ZN4HPHP22f_date_get_last_errorsEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_get_last_errors(Value* _rv) asm("_ZN4HPHP22f_date_get_last_errorsEv");

/*
HPHP::Object HPHP::f_date_interval_create_from_date_string(HPHP::String const&)
_ZN4HPHP39f_date_interval_create_from_date_stringERKNS_6StringE

(return value) => rax
_rv => rdi
time => rsi
*/

Value* fh_date_interval_create_from_date_string(Value* _rv, Value* time) asm("_ZN4HPHP39f_date_interval_create_from_date_stringERKNS_6StringE");

/*
HPHP::String HPHP::f_date_interval_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP22f_date_interval_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
interval => rsi
format_spec => rdx
*/

Value* fh_date_interval_format(Value* _rv, Value* interval, Value* format_spec) asm("_ZN4HPHP22f_date_interval_formatERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_date_isodate_set(HPHP::Object const&, int, int, int)
_ZN4HPHP18f_date_isodate_setERKNS_6ObjectEiii

object => rdi
year => rsi
week => rdx
day => rcx
*/

void fh_date_isodate_set(Value* object, int year, int week, int day) asm("_ZN4HPHP18f_date_isodate_setERKNS_6ObjectEiii");

/*
void HPHP::f_date_modify(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_date_modifyERKNS_6ObjectERKNS_6StringE

object => rdi
modify => rsi
*/

void fh_date_modify(Value* object, Value* modify) asm("_ZN4HPHP13f_date_modifyERKNS_6ObjectERKNS_6StringE");

/*
long HPHP::f_date_offset_get(HPHP::Object const&)
_ZN4HPHP17f_date_offset_getERKNS_6ObjectE

(return value) => rax
object => rdi
*/

long fh_date_offset_get(Value* object) asm("_ZN4HPHP17f_date_offset_getERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_date_parse(HPHP::String const&)
_ZN4HPHP12f_date_parseERKNS_6StringE

(return value) => rax
_rv => rdi
date => rsi
*/

TypedValue* fh_date_parse(TypedValue* _rv, Value* date) asm("_ZN4HPHP12f_date_parseERKNS_6StringE");

/*
HPHP::Object HPHP::f_date_sub(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP10f_date_subERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_sub(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP10f_date_subERKNS_6ObjectES2_");

/*
HPHP::Array HPHP::f_date_sun_info(long, double, double)
_ZN4HPHP15f_date_sun_infoEldd

(return value) => rax
_rv => rdi
ts => rsi
latitude => xmm0
longitude => xmm1
*/

Value* fh_date_sun_info(Value* _rv, long ts, double latitude, double longitude) asm("_ZN4HPHP15f_date_sun_infoEldd");

/*
HPHP::Variant HPHP::f_date_sunrise(long, int, double, double, double, double)
_ZN4HPHP14f_date_sunriseElidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunrise(TypedValue* _rv, long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP14f_date_sunriseElidddd");

/*
HPHP::Variant HPHP::f_date_sunset(long, int, double, double, double, double)
_ZN4HPHP13f_date_sunsetElidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunset(TypedValue* _rv, long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP13f_date_sunsetElidddd");

/*
void HPHP::f_date_time_set(HPHP::Object const&, int, int, int)
_ZN4HPHP15f_date_time_setERKNS_6ObjectEiii

object => rdi
hour => rsi
minute => rdx
second => rcx
*/

void fh_date_time_set(Value* object, int hour, int minute, int second) asm("_ZN4HPHP15f_date_time_setERKNS_6ObjectEiii");

/*
long HPHP::f_date_timestamp_get(HPHP::Object const&)
_ZN4HPHP20f_date_timestamp_getERKNS_6ObjectE

(return value) => rax
datetime => rdi
*/

long fh_date_timestamp_get(Value* datetime) asm("_ZN4HPHP20f_date_timestamp_getERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_date_timestamp_set(HPHP::Object const&, long)
_ZN4HPHP20f_date_timestamp_setERKNS_6ObjectEl

(return value) => rax
_rv => rdi
datetime => rsi
timestamp => rdx
*/

Value* fh_date_timestamp_set(Value* _rv, Value* datetime, long timestamp) asm("_ZN4HPHP20f_date_timestamp_setERKNS_6ObjectEl");

/*
HPHP::Variant HPHP::f_date_timezone_get(HPHP::Object const&)
_ZN4HPHP19f_date_timezone_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

TypedValue* fh_date_timezone_get(TypedValue* _rv, Value* object) asm("_ZN4HPHP19f_date_timezone_getERKNS_6ObjectE");

/*
void HPHP::f_date_timezone_set(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP19f_date_timezone_setERKNS_6ObjectES2_

object => rdi
timezone => rsi
*/

void fh_date_timezone_set(Value* object, Value* timezone) asm("_ZN4HPHP19f_date_timezone_setERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_date(HPHP::String const&, long)
_ZN4HPHP6f_dateERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_date(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP6f_dateERKNS_6StringEl");

/*
HPHP::Array HPHP::f_getdate(long)
_ZN4HPHP9f_getdateEl

(return value) => rax
_rv => rdi
timestamp => rsi
*/

Value* fh_getdate(Value* _rv, long timestamp) asm("_ZN4HPHP9f_getdateEl");

/*
HPHP::Variant HPHP::f_gettimeofday(bool)
_ZN4HPHP14f_gettimeofdayEb

(return value) => rax
_rv => rdi
return_float => rsi
*/

TypedValue* fh_gettimeofday(TypedValue* _rv, bool return_float) asm("_ZN4HPHP14f_gettimeofdayEb");

/*
HPHP::Variant HPHP::f_gmdate(HPHP::String const&, long)
_ZN4HPHP8f_gmdateERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_gmdate(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP8f_gmdateERKNS_6StringEl");

/*
HPHP::Variant HPHP::f_gmmktime(int, int, int, int, int, int)
_ZN4HPHP10f_gmmktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_gmmktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP10f_gmmktimeEiiiiii");

/*
HPHP::String HPHP::f_gmstrftime(HPHP::String const&, long)
_ZN4HPHP12f_gmstrftimeERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

Value* fh_gmstrftime(Value* _rv, Value* format, long timestamp) asm("_ZN4HPHP12f_gmstrftimeERKNS_6StringEl");

/*
HPHP::Variant HPHP::f_idate(HPHP::String const&, long)
_ZN4HPHP7f_idateERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_idate(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP7f_idateERKNS_6StringEl");

/*
HPHP::Array HPHP::f_localtime(long, bool)
_ZN4HPHP11f_localtimeElb

(return value) => rax
_rv => rdi
timestamp => rsi
is_associative => rdx
*/

Value* fh_localtime(Value* _rv, long timestamp, bool is_associative) asm("_ZN4HPHP11f_localtimeElb");

/*
HPHP::Variant HPHP::f_microtime(bool)
_ZN4HPHP11f_microtimeEb

(return value) => rax
_rv => rdi
get_as_float => rsi
*/

TypedValue* fh_microtime(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP11f_microtimeEb");

/*
HPHP::Variant HPHP::f_mktime(int, int, int, int, int, int)
_ZN4HPHP8f_mktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_mktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP8f_mktimeEiiiiii");

/*
HPHP::Variant HPHP::f_strftime(HPHP::String const&, long)
_ZN4HPHP10f_strftimeERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_strftime(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP10f_strftimeERKNS_6StringEl");

/*
HPHP::Variant HPHP::f_strptime(HPHP::String const&, HPHP::String const&)
_ZN4HPHP10f_strptimeERKNS_6StringES2_

(return value) => rax
_rv => rdi
date => rsi
format => rdx
*/

TypedValue* fh_strptime(TypedValue* _rv, Value* date, Value* format) asm("_ZN4HPHP10f_strptimeERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_strtotime(HPHP::String const&, long)
_ZN4HPHP11f_strtotimeERKNS_6StringEl

(return value) => rax
_rv => rdi
input => rsi
timestamp => rdx
*/

TypedValue* fh_strtotime(TypedValue* _rv, Value* input, long timestamp) asm("_ZN4HPHP11f_strtotimeERKNS_6StringEl");

/*
long HPHP::f_time()
_ZN4HPHP6f_timeEv

(return value) => rax
*/

long fh_time() asm("_ZN4HPHP6f_timeEv");

/*
HPHP::Array HPHP::f_timezone_abbreviations_list()
_ZN4HPHP29f_timezone_abbreviations_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_abbreviations_list(Value* _rv) asm("_ZN4HPHP29f_timezone_abbreviations_listEv");

/*
HPHP::Array HPHP::f_timezone_identifiers_list()
_ZN4HPHP27f_timezone_identifiers_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_identifiers_list(Value* _rv) asm("_ZN4HPHP27f_timezone_identifiers_listEv");

/*
HPHP::Array HPHP::f_timezone_location_get(HPHP::Object const&)
_ZN4HPHP23f_timezone_location_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_location_get(Value* _rv, Value* timezone) asm("_ZN4HPHP23f_timezone_location_getERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_timezone_name_from_abbr(HPHP::String const&, int, bool)
_ZN4HPHP25f_timezone_name_from_abbrERKNS_6StringEib

(return value) => rax
_rv => rdi
abbr => rsi
gmtoffset => rdx
isdst => rcx
*/

TypedValue* fh_timezone_name_from_abbr(TypedValue* _rv, Value* abbr, int gmtoffset, bool isdst) asm("_ZN4HPHP25f_timezone_name_from_abbrERKNS_6StringEib");

/*
HPHP::String HPHP::f_timezone_name_get(HPHP::Object const&)
_ZN4HPHP19f_timezone_name_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_name_get(Value* _rv, Value* object) asm("_ZN4HPHP19f_timezone_name_getERKNS_6ObjectE");

/*
long HPHP::f_timezone_offset_get(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_timezone_offset_getERKNS_6ObjectES2_

(return value) => rax
object => rdi
dt => rsi
*/

long fh_timezone_offset_get(Value* object, Value* dt) asm("_ZN4HPHP21f_timezone_offset_getERKNS_6ObjectES2_");

/*
HPHP::Object HPHP::f_timezone_open(HPHP::String const&)
_ZN4HPHP15f_timezone_openERKNS_6StringE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_open(Value* _rv, Value* timezone) asm("_ZN4HPHP15f_timezone_openERKNS_6StringE");

/*
HPHP::Array HPHP::f_timezone_transitions_get(HPHP::Object const&)
_ZN4HPHP26f_timezone_transitions_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_transitions_get(Value* _rv, Value* object) asm("_ZN4HPHP26f_timezone_transitions_getERKNS_6ObjectE");

/*
HPHP::String HPHP::f_timezone_version_get()
_ZN4HPHP22f_timezone_version_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_version_get(Value* _rv) asm("_ZN4HPHP22f_timezone_version_getEv");


} // !HPHP

