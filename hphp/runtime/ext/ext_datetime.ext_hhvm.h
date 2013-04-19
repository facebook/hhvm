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

TypedValue* fh_gettimeofday(TypedValue* _rv, bool return_float) asm("_ZN4HPHP14f_gettimeofdayEb");

TypedValue* fh_microtime(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP11f_microtimeEb");

long fh_time() asm("_ZN4HPHP6f_timeEv");

TypedValue* fh_mktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP8f_mktimeEiiiiii");

TypedValue* fh_gmmktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP10f_gmmktimeEiiiiii");

TypedValue* fh_idate(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP7f_idateERKNS_6StringEl");

TypedValue* fh_date(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP6f_dateERKNS_6StringEl");

TypedValue* fh_gmdate(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP8f_gmdateERKNS_6StringEl");

TypedValue* fh_strftime(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP10f_strftimeERKNS_6StringEl");

Value* fh_gmstrftime(Value* _rv, Value* format, long timestamp) asm("_ZN4HPHP12f_gmstrftimeERKNS_6StringEl");

Value* fh_getdate(Value* _rv, long timestamp) asm("_ZN4HPHP9f_getdateEl");

Value* fh_localtime(Value* _rv, long timestamp, bool is_associative) asm("_ZN4HPHP11f_localtimeElb");

TypedValue* fh_strptime(TypedValue* _rv, Value* date, Value* format) asm("_ZN4HPHP10f_strptimeERKNS_6StringES2_");

TypedValue* fh_strtotime(TypedValue* _rv, Value* input, long timestamp) asm("_ZN4HPHP11f_strtotimeERKNS_6StringEl");

Value* fh_date_default_timezone_get(Value* _rv) asm("_ZN4HPHP27f_date_default_timezone_getEv");

bool fh_date_default_timezone_set(Value* name) asm("_ZN4HPHP27f_date_default_timezone_setERKNS_6StringE");

Value* fh_timezone_identifiers_list(Value* _rv) asm("_ZN4HPHP27f_timezone_identifiers_listEv");

Value* fh_timezone_abbreviations_list(Value* _rv) asm("_ZN4HPHP29f_timezone_abbreviations_listEv");

TypedValue* fh_timezone_name_from_abbr(TypedValue* _rv, Value* abbr, int gmtoffset, bool isdst) asm("_ZN4HPHP25f_timezone_name_from_abbrERKNS_6StringEib");

Value* fh_timezone_open(Value* _rv, Value* timezone) asm("_ZN4HPHP15f_timezone_openERKNS_6StringE");

Value* fh_timezone_location_get(Value* _rv, Value* timezone) asm("_ZN4HPHP23f_timezone_location_getERKNS_6ObjectE");

Value* fh_timezone_name_get(Value* _rv, Value* object) asm("_ZN4HPHP19f_timezone_name_getERKNS_6ObjectE");

long fh_timezone_offset_get(Value* object, Value* dt) asm("_ZN4HPHP21f_timezone_offset_getERKNS_6ObjectES2_");

Value* fh_timezone_transitions_get(Value* _rv, Value* object) asm("_ZN4HPHP26f_timezone_transitions_getERKNS_6ObjectE");

Value* fh_timezone_version_get(Value* _rv) asm("_ZN4HPHP22f_timezone_version_getEv");

bool fh_checkdate(int month, int day, int year) asm("_ZN4HPHP11f_checkdateEiii");

Value* fh_date_add(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP10f_date_addERKNS_6ObjectES2_");

Value* fh_date_create_from_format(Value* _rv, Value* format, Value* time, Value* timezone) asm("_ZN4HPHP25f_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE");

Value* fh_date_create(Value* _rv, Value* time, Value* timezone) asm("_ZN4HPHP13f_date_createERKNS_6StringERKNS_6ObjectE");

void fh_date_date_set(Value* object, int year, int month, int day) asm("_ZN4HPHP15f_date_date_setERKNS_6ObjectEiii");

Value* fh_date_diff(Value* _rv, Value* datetime, Value* datetime2, bool absolute) asm("_ZN4HPHP11f_date_diffERKNS_6ObjectES2_b");

void fh_date_isodate_set(Value* object, int year, int week, int day) asm("_ZN4HPHP18f_date_isodate_setERKNS_6ObjectEiii");

Value* fh_date_format(Value* _rv, Value* object, Value* format) asm("_ZN4HPHP13f_date_formatERKNS_6ObjectERKNS_6StringE");

Value* fh_date_get_last_errors(Value* _rv) asm("_ZN4HPHP22f_date_get_last_errorsEv");

Value* fh_date_interval_create_from_date_string(Value* _rv, Value* time) asm("_ZN4HPHP39f_date_interval_create_from_date_stringERKNS_6StringE");

Value* fh_date_interval_format(Value* _rv, Value* interval, Value* format_spec) asm("_ZN4HPHP22f_date_interval_formatERKNS_6ObjectERKNS_6StringE");

void fh_date_modify(Value* object, Value* modify) asm("_ZN4HPHP13f_date_modifyERKNS_6ObjectERKNS_6StringE");

long fh_date_offset_get(Value* object) asm("_ZN4HPHP17f_date_offset_getERKNS_6ObjectE");

TypedValue* fh_date_parse(TypedValue* _rv, Value* date) asm("_ZN4HPHP12f_date_parseERKNS_6StringE");

void fh_date_time_set(Value* object, int hour, int minute, int second) asm("_ZN4HPHP15f_date_time_setERKNS_6ObjectEiii");

long fh_date_timestamp_get(Value* datetime) asm("_ZN4HPHP20f_date_timestamp_getERKNS_6ObjectE");

Value* fh_date_timestamp_set(Value* _rv, Value* datetime, long timestamp) asm("_ZN4HPHP20f_date_timestamp_setERKNS_6ObjectEl");

TypedValue* fh_date_timezone_get(TypedValue* _rv, Value* object) asm("_ZN4HPHP19f_date_timezone_getERKNS_6ObjectE");

void fh_date_timezone_set(Value* object, Value* timezone) asm("_ZN4HPHP19f_date_timezone_setERKNS_6ObjectES2_");

Value* fh_date_sub(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP10f_date_subERKNS_6ObjectES2_");

Value* fh_date_sun_info(Value* _rv, long ts, double latitude, double longitude) asm("_ZN4HPHP15f_date_sun_infoEldd");

TypedValue* fh_date_sunrise(TypedValue* _rv, long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP14f_date_sunriseElidddd");

TypedValue* fh_date_sunset(TypedValue* _rv, long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP13f_date_sunsetElidddd");

} // namespace HPHP
