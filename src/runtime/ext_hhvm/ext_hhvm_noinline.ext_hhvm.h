/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
bool HPHP::fni_apc_compile_file(HPHP::String const&, bool, long long)
_ZN4HPHP20fni_apc_compile_fileERKNS_6StringEbx

(return value) => rax
filename => rdi
atomic => rsi
cache_id => rdx
*/

bool fh_apc_compile_file(Value* filename, bool atomic, long long cache_id) asm("_ZN4HPHP20fni_apc_compile_fileERKNS_6StringEbx");

/*
bool HPHP::fni_apc_define_constants(HPHP::String const&, HPHP::String const&, bool, long long)
_ZN4HPHP24fni_apc_define_constantsERKNS_6StringES2_bx

(return value) => rax
key => rdi
constants => rsi
case_sensitive => rdx
cache_id => rcx
*/

bool fh_apc_define_constants(Value* key, Value* constants, bool case_sensitive, long long cache_id) asm("_ZN4HPHP24fni_apc_define_constantsERKNS_6StringES2_bx");

/*
bool HPHP::fni_apc_load_constants(HPHP::String const&, bool, long long)
_ZN4HPHP22fni_apc_load_constantsERKNS_6StringEbx

(return value) => rax
key => rdi
case_sensitive => rsi
cache_id => rdx
*/

bool fh_apc_load_constants(Value* key, bool case_sensitive, long long cache_id) asm("_ZN4HPHP22fni_apc_load_constantsERKNS_6StringEbx");

/*
HPHP::Array HPHP::fni_apc_sma_info(bool)
_ZN4HPHP16fni_apc_sma_infoEb

(return value) => rax
_rv => rdi
limited => rsi
*/

Value* fh_apc_sma_info(Value* _rv, bool limited) asm("_ZN4HPHP16fni_apc_sma_infoEb");

/*
HPHP::Array HPHP::fni_apc_filehits()
_ZN4HPHP16fni_apc_filehitsEv

(return value) => rax
_rv => rdi
*/

Value* fh_apc_filehits(Value* _rv) asm("_ZN4HPHP16fni_apc_filehitsEv");

/*
HPHP::Variant HPHP::fni_apc_delete_file(HPHP::Variant const&, long long)
_ZN4HPHP19fni_apc_delete_fileERKNS_7VariantEx

(return value) => rax
_rv => rdi
keys => rsi
cache_id => rdx
*/

TypedValue* fh_apc_delete_file(TypedValue* _rv, TypedValue* keys, long long cache_id) asm("_ZN4HPHP19fni_apc_delete_fileERKNS_7VariantEx");

/*
HPHP::Variant HPHP::fni_apc_bin_dump(long long, HPHP::Variant const&)
_ZN4HPHP16fni_apc_bin_dumpExRKNS_7VariantE

(return value) => rax
_rv => rdi
cache_id => rsi
filter => rdx
*/

TypedValue* fh_apc_bin_dump(TypedValue* _rv, long long cache_id, TypedValue* filter) asm("_ZN4HPHP16fni_apc_bin_dumpExRKNS_7VariantE");

/*
bool HPHP::fni_apc_bin_load(HPHP::String const&, long long, long long)
_ZN4HPHP16fni_apc_bin_loadERKNS_6StringExx

(return value) => rax
data => rdi
flags => rsi
cache_id => rdx
*/

bool fh_apc_bin_load(Value* data, long long flags, long long cache_id) asm("_ZN4HPHP16fni_apc_bin_loadERKNS_6StringExx");

/*
HPHP::Variant HPHP::fni_apc_bin_dumpfile(long long, HPHP::Variant const&, HPHP::String const&, long long, HPHP::Object const&)
_ZN4HPHP20fni_apc_bin_dumpfileExRKNS_7VariantERKNS_6StringExRKNS_6ObjectE

(return value) => rax
_rv => rdi
cache_id => rsi
filter => rdx
filename => rcx
flags => r8
context => r9
*/

TypedValue* fh_apc_bin_dumpfile(TypedValue* _rv, long long cache_id, TypedValue* filter, Value* filename, long long flags, Value* context) asm("_ZN4HPHP20fni_apc_bin_dumpfileExRKNS_7VariantERKNS_6StringExRKNS_6ObjectE");

/*
bool HPHP::fni_apc_bin_loadfile(HPHP::String const&, HPHP::Object const&, long long, long long)
_ZN4HPHP20fni_apc_bin_loadfileERKNS_6StringERKNS_6ObjectExx

(return value) => rax
filename => rdi
context => rsi
flags => rdx
cache_id => rcx
*/

bool fh_apc_bin_loadfile(Value* filename, Value* context, long long flags, long long cache_id) asm("_ZN4HPHP20fni_apc_bin_loadfileERKNS_6StringERKNS_6ObjectExx");

/*
HPHP::Variant HPHP::fni_array_fill(int, int, HPHP::Variant const&)
_ZN4HPHP14fni_array_fillEiiRKNS_7VariantE

(return value) => rax
_rv => rdi
start_index => rsi
num => rdx
value => rcx
*/

TypedValue* fh_array_fill(TypedValue* _rv, int start_index, int num, TypedValue* value) asm("_ZN4HPHP14fni_array_fillEiiRKNS_7VariantE");

/*
bool HPHP::fni_key_exists(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14fni_key_existsERKNS_7VariantES2_

(return value) => rax
key => rdi
search => rsi
*/

bool fh_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP14fni_key_existsERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::fni_array_pop(HPHP::VRefParamValue const&)
_ZN4HPHP13fni_array_popERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_pop(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13fni_array_popERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_array_shift(HPHP::VRefParamValue const&)
_ZN4HPHP15fni_array_shiftERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_shift(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP15fni_array_shiftERKNS_14VRefParamValueE");

/*
long long HPHP::fni_sizeof(HPHP::Variant const&, bool)
_ZN4HPHP10fni_sizeofERKNS_7VariantEb

(return value) => rax
var => rdi
recursive => rsi
*/

long long fh_sizeof(TypedValue* var, bool recursive) asm("_ZN4HPHP10fni_sizeofERKNS_7VariantEb");

/*
HPHP::Variant HPHP::fni_each(HPHP::VRefParamValue const&)
_ZN4HPHP8fni_eachERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_each(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP8fni_eachERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_current(HPHP::VRefParamValue const&)
_ZN4HPHP11fni_currentERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_current(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11fni_currentERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_hphp_current_ref(HPHP::VRefParamValue const&)
_ZN4HPHP20fni_hphp_current_refERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_hphp_current_ref(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP20fni_hphp_current_refERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_next(HPHP::VRefParamValue const&)
_ZN4HPHP8fni_nextERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_next(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP8fni_nextERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_pos(HPHP::VRefParamValue const&)
_ZN4HPHP7fni_posERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_pos(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7fni_posERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_prev(HPHP::VRefParamValue const&)
_ZN4HPHP8fni_prevERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_prev(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP8fni_prevERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_reset(HPHP::VRefParamValue const&)
_ZN4HPHP9fni_resetERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_reset(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9fni_resetERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_end(HPHP::VRefParamValue const&)
_ZN4HPHP7fni_endERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_end(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7fni_endERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_key(HPHP::VRefParamValue const&)
_ZN4HPHP7fni_keyERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_key(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7fni_keyERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_bzclose(HPHP::Object const&)
_ZN4HPHP11fni_bzcloseERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

TypedValue* fh_bzclose(TypedValue* _rv, Value* bz) asm("_ZN4HPHP11fni_bzcloseERKNS_6ObjectE");

/*
HPHP::Variant HPHP::fni_bzread(HPHP::Object const&, int)
_ZN4HPHP10fni_bzreadERKNS_6ObjectEi

(return value) => rax
_rv => rdi
bz => rsi
length => rdx
*/

TypedValue* fh_bzread(TypedValue* _rv, Value* bz, int length) asm("_ZN4HPHP10fni_bzreadERKNS_6ObjectEi");

/*
HPHP::Variant HPHP::fni_bzwrite(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP11fni_bzwriteERKNS_6ObjectERKNS_6StringEi

(return value) => rax
_rv => rdi
bz => rsi
data => rdx
length => rcx
*/

TypedValue* fh_bzwrite(TypedValue* _rv, Value* bz, Value* data, int length) asm("_ZN4HPHP11fni_bzwriteERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::fni_checkdate(int, int, int)
_ZN4HPHP13fni_checkdateEiii

(return value) => rax
month => rdi
day => rsi
year => rdx
*/

bool fh_checkdate(int month, int day, int year) asm("_ZN4HPHP13fni_checkdateEiii");

/*
HPHP::Object HPHP::fni_date_add(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP12fni_date_addERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_add(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP12fni_date_addERKNS_6ObjectES2_");

/*
HPHP::Object HPHP::fni_date_create_from_format(HPHP::String const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP27fni_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE

(return value) => rax
_rv => rdi
format => rsi
time => rdx
timezone => rcx
*/

Value* fh_date_create_from_format(Value* _rv, Value* format, Value* time, Value* timezone) asm("_ZN4HPHP27fni_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE");

/*
HPHP::Object HPHP::fni_date_create(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP15fni_date_createERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
time => rsi
timezone => rdx
*/

Value* fh_date_create(Value* _rv, Value* time, Value* timezone) asm("_ZN4HPHP15fni_date_createERKNS_6StringERKNS_6ObjectE");

/*
void HPHP::fni_date_date_set(HPHP::Object const&, int, int, int)
_ZN4HPHP17fni_date_date_setERKNS_6ObjectEiii

object => rdi
year => rsi
month => rdx
day => rcx
*/

void fh_date_date_set(Value* object, int year, int month, int day) asm("_ZN4HPHP17fni_date_date_setERKNS_6ObjectEiii");

/*
HPHP::String HPHP::fni_date_default_timezone_get()
_ZN4HPHP29fni_date_default_timezone_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_default_timezone_get(Value* _rv) asm("_ZN4HPHP29fni_date_default_timezone_getEv");

/*
bool HPHP::fni_date_default_timezone_set(HPHP::String const&)
_ZN4HPHP29fni_date_default_timezone_setERKNS_6StringE

(return value) => rax
name => rdi
*/

bool fh_date_default_timezone_set(Value* name) asm("_ZN4HPHP29fni_date_default_timezone_setERKNS_6StringE");

/*
HPHP::Object HPHP::fni_date_diff(HPHP::Object const&, HPHP::Object const&, bool)
_ZN4HPHP13fni_date_diffERKNS_6ObjectES2_b

(return value) => rax
_rv => rdi
datetime => rsi
datetime2 => rdx
absolute => rcx
*/

Value* fh_date_diff(Value* _rv, Value* datetime, Value* datetime2, bool absolute) asm("_ZN4HPHP13fni_date_diffERKNS_6ObjectES2_b");

/*
HPHP::String HPHP::fni_date_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP15fni_date_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
object => rsi
format => rdx
*/

Value* fh_date_format(Value* _rv, Value* object, Value* format) asm("_ZN4HPHP15fni_date_formatERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Array HPHP::fni_date_get_last_errors()
_ZN4HPHP24fni_date_get_last_errorsEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_get_last_errors(Value* _rv) asm("_ZN4HPHP24fni_date_get_last_errorsEv");

/*
HPHP::Object HPHP::fni_date_interval_create_from_date_string(HPHP::String const&)
_ZN4HPHP41fni_date_interval_create_from_date_stringERKNS_6StringE

(return value) => rax
_rv => rdi
time => rsi
*/

Value* fh_date_interval_create_from_date_string(Value* _rv, Value* time) asm("_ZN4HPHP41fni_date_interval_create_from_date_stringERKNS_6StringE");

/*
HPHP::String HPHP::fni_date_interval_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP24fni_date_interval_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
interval => rsi
format_spec => rdx
*/

Value* fh_date_interval_format(Value* _rv, Value* interval, Value* format_spec) asm("_ZN4HPHP24fni_date_interval_formatERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::fni_date_isodate_set(HPHP::Object const&, int, int, int)
_ZN4HPHP20fni_date_isodate_setERKNS_6ObjectEiii

object => rdi
year => rsi
week => rdx
day => rcx
*/

void fh_date_isodate_set(Value* object, int year, int week, int day) asm("_ZN4HPHP20fni_date_isodate_setERKNS_6ObjectEiii");

/*
void HPHP::fni_date_modify(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP15fni_date_modifyERKNS_6ObjectERKNS_6StringE

object => rdi
modify => rsi
*/

void fh_date_modify(Value* object, Value* modify) asm("_ZN4HPHP15fni_date_modifyERKNS_6ObjectERKNS_6StringE");

/*
long long HPHP::fni_date_offset_get(HPHP::Object const&)
_ZN4HPHP19fni_date_offset_getERKNS_6ObjectE

(return value) => rax
object => rdi
*/

long long fh_date_offset_get(Value* object) asm("_ZN4HPHP19fni_date_offset_getERKNS_6ObjectE");

/*
HPHP::Variant HPHP::fni_date_parse(HPHP::String const&)
_ZN4HPHP14fni_date_parseERKNS_6StringE

(return value) => rax
_rv => rdi
date => rsi
*/

TypedValue* fh_date_parse(TypedValue* _rv, Value* date) asm("_ZN4HPHP14fni_date_parseERKNS_6StringE");

/*
HPHP::Object HPHP::fni_date_sub(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP12fni_date_subERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_sub(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP12fni_date_subERKNS_6ObjectES2_");

/*
HPHP::Array HPHP::fni_date_sun_info(long long, double, double)
_ZN4HPHP17fni_date_sun_infoExdd

(return value) => rax
_rv => rdi
ts => rsi
latitude => xmm0
longitude => xmm1
*/

Value* fh_date_sun_info(Value* _rv, long long ts, double latitude, double longitude) asm("_ZN4HPHP17fni_date_sun_infoExdd");

/*
HPHP::Variant HPHP::fni_date_sunrise(long long, int, double, double, double, double)
_ZN4HPHP16fni_date_sunriseExidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunrise(TypedValue* _rv, long long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP16fni_date_sunriseExidddd");

/*
HPHP::Variant HPHP::fni_date_sunset(long long, int, double, double, double, double)
_ZN4HPHP15fni_date_sunsetExidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunset(TypedValue* _rv, long long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP15fni_date_sunsetExidddd");

/*
void HPHP::fni_date_time_set(HPHP::Object const&, int, int, int)
_ZN4HPHP17fni_date_time_setERKNS_6ObjectEiii

object => rdi
hour => rsi
minute => rdx
second => rcx
*/

void fh_date_time_set(Value* object, int hour, int minute, int second) asm("_ZN4HPHP17fni_date_time_setERKNS_6ObjectEiii");

/*
long long HPHP::fni_date_timestamp_get(HPHP::Object const&)
_ZN4HPHP22fni_date_timestamp_getERKNS_6ObjectE

(return value) => rax
datetime => rdi
*/

long long fh_date_timestamp_get(Value* datetime) asm("_ZN4HPHP22fni_date_timestamp_getERKNS_6ObjectE");

/*
HPHP::Object HPHP::fni_date_timestamp_set(HPHP::Object const&, long long)
_ZN4HPHP22fni_date_timestamp_setERKNS_6ObjectEx

(return value) => rax
_rv => rdi
datetime => rsi
timestamp => rdx
*/

Value* fh_date_timestamp_set(Value* _rv, Value* datetime, long long timestamp) asm("_ZN4HPHP22fni_date_timestamp_setERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::fni_date_timezone_get(HPHP::Object const&)
_ZN4HPHP21fni_date_timezone_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

TypedValue* fh_date_timezone_get(TypedValue* _rv, Value* object) asm("_ZN4HPHP21fni_date_timezone_getERKNS_6ObjectE");

/*
void HPHP::fni_date_timezone_set(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21fni_date_timezone_setERKNS_6ObjectES2_

object => rdi
timezone => rsi
*/

void fh_date_timezone_set(Value* object, Value* timezone) asm("_ZN4HPHP21fni_date_timezone_setERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::fni_date(HPHP::String const&, long long)
_ZN4HPHP8fni_dateERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_date(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP8fni_dateERKNS_6StringEx");

/*
HPHP::Array HPHP::fni_getdate(long long)
_ZN4HPHP11fni_getdateEx

(return value) => rax
_rv => rdi
timestamp => rsi
*/

Value* fh_getdate(Value* _rv, long long timestamp) asm("_ZN4HPHP11fni_getdateEx");

/*
HPHP::Variant HPHP::fni_gettimeofday(bool)
_ZN4HPHP16fni_gettimeofdayEb

(return value) => rax
_rv => rdi
return_float => rsi
*/

TypedValue* fh_gettimeofday(TypedValue* _rv, bool return_float) asm("_ZN4HPHP16fni_gettimeofdayEb");

/*
HPHP::Variant HPHP::fni_gmdate(HPHP::String const&, long long)
_ZN4HPHP10fni_gmdateERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_gmdate(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP10fni_gmdateERKNS_6StringEx");

/*
HPHP::Variant HPHP::fni_gmmktime(int, int, int, int, int, int)
_ZN4HPHP12fni_gmmktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_gmmktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP12fni_gmmktimeEiiiiii");

/*
HPHP::String HPHP::fni_gmstrftime(HPHP::String const&, long long)
_ZN4HPHP14fni_gmstrftimeERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

Value* fh_gmstrftime(Value* _rv, Value* format, long long timestamp) asm("_ZN4HPHP14fni_gmstrftimeERKNS_6StringEx");

/*
HPHP::Variant HPHP::fni_idate(HPHP::String const&, long long)
_ZN4HPHP9fni_idateERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_idate(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP9fni_idateERKNS_6StringEx");

/*
HPHP::Array HPHP::fni_localtime(long long, bool)
_ZN4HPHP13fni_localtimeExb

(return value) => rax
_rv => rdi
timestamp => rsi
is_associative => rdx
*/

Value* fh_localtime(Value* _rv, long long timestamp, bool is_associative) asm("_ZN4HPHP13fni_localtimeExb");

/*
HPHP::Variant HPHP::fni_microtime(bool)
_ZN4HPHP13fni_microtimeEb

(return value) => rax
_rv => rdi
get_as_float => rsi
*/

TypedValue* fh_microtime(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP13fni_microtimeEb");

/*
HPHP::Variant HPHP::fni_mktime(int, int, int, int, int, int)
_ZN4HPHP10fni_mktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_mktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP10fni_mktimeEiiiiii");

/*
HPHP::Variant HPHP::fni_strftime(HPHP::String const&, long long)
_ZN4HPHP12fni_strftimeERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_strftime(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP12fni_strftimeERKNS_6StringEx");

/*
HPHP::Variant HPHP::fni_strptime(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12fni_strptimeERKNS_6StringES2_

(return value) => rax
_rv => rdi
date => rsi
format => rdx
*/

TypedValue* fh_strptime(TypedValue* _rv, Value* date, Value* format) asm("_ZN4HPHP12fni_strptimeERKNS_6StringES2_");

/*
HPHP::Variant HPHP::fni_strtotime(HPHP::String const&, long long)
_ZN4HPHP13fni_strtotimeERKNS_6StringEx

(return value) => rax
_rv => rdi
input => rsi
timestamp => rdx
*/

TypedValue* fh_strtotime(TypedValue* _rv, Value* input, long long timestamp) asm("_ZN4HPHP13fni_strtotimeERKNS_6StringEx");

/*
long long HPHP::fni_time()
_ZN4HPHP8fni_timeEv

(return value) => rax
*/

long long fh_time() asm("_ZN4HPHP8fni_timeEv");

/*
HPHP::Array HPHP::fni_timezone_abbreviations_list()
_ZN4HPHP31fni_timezone_abbreviations_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_abbreviations_list(Value* _rv) asm("_ZN4HPHP31fni_timezone_abbreviations_listEv");

/*
HPHP::Array HPHP::fni_timezone_identifiers_list()
_ZN4HPHP29fni_timezone_identifiers_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_identifiers_list(Value* _rv) asm("_ZN4HPHP29fni_timezone_identifiers_listEv");

/*
HPHP::Array HPHP::fni_timezone_location_get(HPHP::Object const&)
_ZN4HPHP25fni_timezone_location_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_location_get(Value* _rv, Value* timezone) asm("_ZN4HPHP25fni_timezone_location_getERKNS_6ObjectE");

/*
HPHP::Variant HPHP::fni_timezone_name_from_abbr(HPHP::String const&, int, bool)
_ZN4HPHP27fni_timezone_name_from_abbrERKNS_6StringEib

(return value) => rax
_rv => rdi
abbr => rsi
gmtoffset => rdx
isdst => rcx
*/

TypedValue* fh_timezone_name_from_abbr(TypedValue* _rv, Value* abbr, int gmtoffset, bool isdst) asm("_ZN4HPHP27fni_timezone_name_from_abbrERKNS_6StringEib");

/*
HPHP::String HPHP::fni_timezone_name_get(HPHP::Object const&)
_ZN4HPHP21fni_timezone_name_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_name_get(Value* _rv, Value* object) asm("_ZN4HPHP21fni_timezone_name_getERKNS_6ObjectE");

/*
long long HPHP::fni_timezone_offset_get(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23fni_timezone_offset_getERKNS_6ObjectES2_

(return value) => rax
object => rdi
dt => rsi
*/

long long fh_timezone_offset_get(Value* object, Value* dt) asm("_ZN4HPHP23fni_timezone_offset_getERKNS_6ObjectES2_");

/*
HPHP::Object HPHP::fni_timezone_open(HPHP::String const&)
_ZN4HPHP17fni_timezone_openERKNS_6StringE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_open(Value* _rv, Value* timezone) asm("_ZN4HPHP17fni_timezone_openERKNS_6StringE");

/*
HPHP::Array HPHP::fni_timezone_transitions_get(HPHP::Object const&)
_ZN4HPHP28fni_timezone_transitions_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_transitions_get(Value* _rv, Value* object) asm("_ZN4HPHP28fni_timezone_transitions_getERKNS_6ObjectE");

/*
HPHP::String HPHP::fni_timezone_version_get()
_ZN4HPHP24fni_timezone_version_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_version_get(Value* _rv) asm("_ZN4HPHP24fni_timezone_version_getEv");

/*
HPHP::Variant HPHP::fni_call_user_func_array(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24fni_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
function => rsi
params => rdx
*/

TypedValue* fh_call_user_func_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP24fni_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE");

/*
double HPHP::fni_pi()
_ZN4HPHP6fni_piEv

(return value) => xmm0
*/

double fh_pi() asm("_ZN4HPHP6fni_piEv");

/*
bool HPHP::fni_is_finite(double)
_ZN4HPHP13fni_is_finiteEd

(return value) => rax
val => xmm0
*/

bool fh_is_finite(double val) asm("_ZN4HPHP13fni_is_finiteEd");

/*
bool HPHP::fni_is_infinite(double)
_ZN4HPHP15fni_is_infiniteEd

(return value) => rax
val => xmm0
*/

bool fh_is_infinite(double val) asm("_ZN4HPHP15fni_is_infiniteEd");

/*
bool HPHP::fni_is_nan(double)
_ZN4HPHP10fni_is_nanEd

(return value) => rax
val => xmm0
*/

bool fh_is_nan(double val) asm("_ZN4HPHP10fni_is_nanEd");

/*
double HPHP::fni_ceil(double)
_ZN4HPHP8fni_ceilEd

(return value) => xmm0
value => xmm0
*/

double fh_ceil(double value) asm("_ZN4HPHP8fni_ceilEd");

/*
double HPHP::fni_floor(double)
_ZN4HPHP9fni_floorEd

(return value) => xmm0
value => xmm0
*/

double fh_floor(double value) asm("_ZN4HPHP9fni_floorEd");

/*
double HPHP::fni_deg2rad(double)
_ZN4HPHP11fni_deg2radEd

(return value) => xmm0
number => xmm0
*/

double fh_deg2rad(double number) asm("_ZN4HPHP11fni_deg2radEd");

/*
double HPHP::fni_rad2deg(double)
_ZN4HPHP11fni_rad2degEd

(return value) => xmm0
number => xmm0
*/

double fh_rad2deg(double number) asm("_ZN4HPHP11fni_rad2degEd");

/*
HPHP::String HPHP::fni_decbin(long long)
_ZN4HPHP10fni_decbinEx

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decbin(Value* _rv, long long number) asm("_ZN4HPHP10fni_decbinEx");

/*
HPHP::String HPHP::fni_dechex(long long)
_ZN4HPHP10fni_dechexEx

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_dechex(Value* _rv, long long number) asm("_ZN4HPHP10fni_dechexEx");

/*
HPHP::String HPHP::fni_decoct(long long)
_ZN4HPHP10fni_decoctEx

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decoct(Value* _rv, long long number) asm("_ZN4HPHP10fni_decoctEx");

/*
HPHP::Variant HPHP::fni_bindec(HPHP::String const&)
_ZN4HPHP10fni_bindecERKNS_6StringE

(return value) => rax
_rv => rdi
binary_string => rsi
*/

TypedValue* fh_bindec(TypedValue* _rv, Value* binary_string) asm("_ZN4HPHP10fni_bindecERKNS_6StringE");

/*
HPHP::Variant HPHP::fni_hexdec(HPHP::String const&)
_ZN4HPHP10fni_hexdecERKNS_6StringE

(return value) => rax
_rv => rdi
hex_string => rsi
*/

TypedValue* fh_hexdec(TypedValue* _rv, Value* hex_string) asm("_ZN4HPHP10fni_hexdecERKNS_6StringE");

/*
HPHP::Variant HPHP::fni_octdec(HPHP::String const&)
_ZN4HPHP10fni_octdecERKNS_6StringE

(return value) => rax
_rv => rdi
octal_string => rsi
*/

TypedValue* fh_octdec(TypedValue* _rv, Value* octal_string) asm("_ZN4HPHP10fni_octdecERKNS_6StringE");

/*
double HPHP::fni_exp(double)
_ZN4HPHP7fni_expEd

(return value) => xmm0
arg => xmm0
*/

double fh_exp(double arg) asm("_ZN4HPHP7fni_expEd");

/*
double HPHP::fni_expm1(double)
_ZN4HPHP9fni_expm1Ed

(return value) => xmm0
arg => xmm0
*/

double fh_expm1(double arg) asm("_ZN4HPHP9fni_expm1Ed");

/*
double HPHP::fni_log10(double)
_ZN4HPHP9fni_log10Ed

(return value) => xmm0
arg => xmm0
*/

double fh_log10(double arg) asm("_ZN4HPHP9fni_log10Ed");

/*
double HPHP::fni_log1p(double)
_ZN4HPHP9fni_log1pEd

(return value) => xmm0
number => xmm0
*/

double fh_log1p(double number) asm("_ZN4HPHP9fni_log1pEd");

/*
double HPHP::fni_log(double, double)
_ZN4HPHP7fni_logEdd

(return value) => xmm0
arg => xmm0
base => xmm1
*/

double fh_log(double arg, double base) asm("_ZN4HPHP7fni_logEdd");

/*
double HPHP::fni_cos(double)
_ZN4HPHP7fni_cosEd

(return value) => xmm0
arg => xmm0
*/

double fh_cos(double arg) asm("_ZN4HPHP7fni_cosEd");

/*
double HPHP::fni_cosh(double)
_ZN4HPHP8fni_coshEd

(return value) => xmm0
arg => xmm0
*/

double fh_cosh(double arg) asm("_ZN4HPHP8fni_coshEd");

/*
double HPHP::fni_sin(double)
_ZN4HPHP7fni_sinEd

(return value) => xmm0
arg => xmm0
*/

double fh_sin(double arg) asm("_ZN4HPHP7fni_sinEd");

/*
double HPHP::fni_sinh(double)
_ZN4HPHP8fni_sinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_sinh(double arg) asm("_ZN4HPHP8fni_sinhEd");

/*
double HPHP::fni_tan(double)
_ZN4HPHP7fni_tanEd

(return value) => xmm0
arg => xmm0
*/

double fh_tan(double arg) asm("_ZN4HPHP7fni_tanEd");

/*
double HPHP::fni_tanh(double)
_ZN4HPHP8fni_tanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_tanh(double arg) asm("_ZN4HPHP8fni_tanhEd");

/*
double HPHP::fni_acos(double)
_ZN4HPHP8fni_acosEd

(return value) => xmm0
arg => xmm0
*/

double fh_acos(double arg) asm("_ZN4HPHP8fni_acosEd");

/*
double HPHP::fni_acosh(double)
_ZN4HPHP9fni_acoshEd

(return value) => xmm0
arg => xmm0
*/

double fh_acosh(double arg) asm("_ZN4HPHP9fni_acoshEd");

/*
double HPHP::fni_asin(double)
_ZN4HPHP8fni_asinEd

(return value) => xmm0
arg => xmm0
*/

double fh_asin(double arg) asm("_ZN4HPHP8fni_asinEd");

/*
double HPHP::fni_asinh(double)
_ZN4HPHP9fni_asinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_asinh(double arg) asm("_ZN4HPHP9fni_asinhEd");

/*
double HPHP::fni_atan(double)
_ZN4HPHP8fni_atanEd

(return value) => xmm0
arg => xmm0
*/

double fh_atan(double arg) asm("_ZN4HPHP8fni_atanEd");

/*
double HPHP::fni_atanh(double)
_ZN4HPHP9fni_atanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_atanh(double arg) asm("_ZN4HPHP9fni_atanhEd");

/*
double HPHP::fni_atan2(double, double)
_ZN4HPHP9fni_atan2Edd

(return value) => xmm0
y => xmm0
x => xmm1
*/

double fh_atan2(double y, double x) asm("_ZN4HPHP9fni_atan2Edd");

/*
double HPHP::fni_hypot(double, double)
_ZN4HPHP9fni_hypotEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_hypot(double x, double y) asm("_ZN4HPHP9fni_hypotEdd");

/*
double HPHP::fni_fmod(double, double)
_ZN4HPHP8fni_fmodEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_fmod(double x, double y) asm("_ZN4HPHP8fni_fmodEdd");

/*
double HPHP::fni_sqrt(double)
_ZN4HPHP8fni_sqrtEd

(return value) => xmm0
arg => xmm0
*/

double fh_sqrt(double arg) asm("_ZN4HPHP8fni_sqrtEd");

/*
long long HPHP::fni_getrandmax()
_ZN4HPHP14fni_getrandmaxEv

(return value) => rax
*/

long long fh_getrandmax() asm("_ZN4HPHP14fni_getrandmaxEv");

/*
long long HPHP::fni_mt_getrandmax()
_ZN4HPHP17fni_mt_getrandmaxEv

(return value) => rax
*/

long long fh_mt_getrandmax() asm("_ZN4HPHP17fni_mt_getrandmaxEv");

/*
long long HPHP::fni_mt_rand(long long, long long)
_ZN4HPHP11fni_mt_randExx

(return value) => rax
min => rdi
max => rsi
*/

long long fh_mt_rand(long long min, long long max) asm("_ZN4HPHP11fni_mt_randExx");

/*
double HPHP::fni_lcg_value()
_ZN4HPHP13fni_lcg_valueEv

(return value) => xmm0
*/

double fh_lcg_value() asm("_ZN4HPHP13fni_lcg_valueEv");

/*
HPHP::Variant HPHP::fni_mysql_set_charset(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP21fni_mysql_set_charsetERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
charset => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_set_charset(TypedValue* _rv, Value* charset, TypedValue* link_identifier) asm("_ZN4HPHP21fni_mysql_set_charsetERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_ping(HPHP::Variant const&)
_ZN4HPHP14fni_mysql_pingERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_ping(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP14fni_mysql_pingERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_client_encoding(HPHP::Variant const&)
_ZN4HPHP25fni_mysql_client_encodingERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_client_encoding(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP25fni_mysql_client_encodingERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_close(HPHP::Variant const&)
_ZN4HPHP15fni_mysql_closeERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_close(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP15fni_mysql_closeERKNS_7VariantE");

/*
HPHP::String HPHP::fni_mysql_get_client_info()
_ZN4HPHP25fni_mysql_get_client_infoEv

(return value) => rax
_rv => rdi
*/

Value* fh_mysql_get_client_info(Value* _rv) asm("_ZN4HPHP25fni_mysql_get_client_infoEv");

/*
HPHP::Variant HPHP::fni_mysql_get_host_info(HPHP::Variant const&)
_ZN4HPHP23fni_mysql_get_host_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_host_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23fni_mysql_get_host_infoERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_get_proto_info(HPHP::Variant const&)
_ZN4HPHP24fni_mysql_get_proto_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_proto_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP24fni_mysql_get_proto_infoERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_get_server_info(HPHP::Variant const&)
_ZN4HPHP25fni_mysql_get_server_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_server_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP25fni_mysql_get_server_infoERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_info(HPHP::Variant const&)
_ZN4HPHP14fni_mysql_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP14fni_mysql_infoERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_insert_id(HPHP::Variant const&)
_ZN4HPHP19fni_mysql_insert_idERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_insert_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_insert_idERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_stat(HPHP::Variant const&)
_ZN4HPHP14fni_mysql_statERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_stat(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP14fni_mysql_statERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_thread_id(HPHP::Variant const&)
_ZN4HPHP19fni_mysql_thread_idERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_thread_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_thread_idERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_create_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19fni_mysql_create_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_create_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_create_dbERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_select_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19fni_mysql_select_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_select_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_select_dbERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_drop_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP17fni_mysql_drop_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_drop_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP17fni_mysql_drop_dbERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_affected_rows(HPHP::Variant const&)
_ZN4HPHP23fni_mysql_affected_rowsERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_affected_rows(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23fni_mysql_affected_rowsERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_db_query(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP18fni_mysql_db_queryERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
database => rsi
query => rdx
link_identifier => rcx
*/

TypedValue* fh_mysql_db_query(TypedValue* _rv, Value* database, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP18fni_mysql_db_queryERKNS_6StringES2_RKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_list_fields(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP21fni_mysql_list_fieldsERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
database_name => rsi
table_name => rdx
link_identifier => rcx
*/

TypedValue* fh_mysql_list_fields(TypedValue* _rv, Value* database_name, Value* table_name, TypedValue* link_identifier) asm("_ZN4HPHP21fni_mysql_list_fieldsERKNS_6StringES2_RKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_mysql_db_name(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP17fni_mysql_db_nameERKNS_7VariantEiS2_

(return value) => rax
_rv => rdi
result => rsi
row => rdx
field => rcx
*/

TypedValue* fh_mysql_db_name(TypedValue* _rv, TypedValue* result, int row, TypedValue* field) asm("_ZN4HPHP17fni_mysql_db_nameERKNS_7VariantEiS2_");

/*
HPHP::Variant HPHP::fni_mysql_tablename(HPHP::Variant const&, int)
_ZN4HPHP19fni_mysql_tablenameERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
i => rdx
*/

TypedValue* fh_mysql_tablename(TypedValue* _rv, TypedValue* result, int i) asm("_ZN4HPHP19fni_mysql_tablenameERKNS_7VariantEi");

/*
bool HPHP::fni_checkdnsrr(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14fni_checkdnsrrERKNS_6StringES2_

(return value) => rax
host => rdi
type => rsi
*/

bool fh_checkdnsrr(Value* host, Value* type) asm("_ZN4HPHP14fni_checkdnsrrERKNS_6StringES2_");

/*
bool HPHP::fni_getmxrr(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP11fni_getmxrrERKNS_6StringERKNS_14VRefParamValueES5_

(return value) => rax
hostname => rdi
mxhosts => rsi
weight => rdx
*/

bool fh_getmxrr(Value* hostname, TypedValue* mxhosts, TypedValue* weight) asm("_ZN4HPHP11fni_getmxrrERKNS_6StringERKNS_14VRefParamValueES5_");

/*
HPHP::Variant HPHP::fni_socket_get_status(HPHP::Object const&)
_ZN4HPHP21fni_socket_get_statusERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream => rsi
*/

TypedValue* fh_socket_get_status(TypedValue* _rv, Value* stream) asm("_ZN4HPHP21fni_socket_get_statusERKNS_6ObjectE");

/*
bool HPHP::fni_socket_set_blocking(HPHP::Object const&, int)
_ZN4HPHP23fni_socket_set_blockingERKNS_6ObjectEi

(return value) => rax
stream => rdi
mode => rsi
*/

bool fh_socket_set_blocking(Value* stream, int mode) asm("_ZN4HPHP23fni_socket_set_blockingERKNS_6ObjectEi");

/*
bool HPHP::fni_socket_set_timeout(HPHP::Object const&, int, int)
_ZN4HPHP22fni_socket_set_timeoutERKNS_6ObjectEii

(return value) => rax
stream => rdi
seconds => rsi
microseconds => rdx
*/

bool fh_socket_set_timeout(Value* stream, int seconds, int microseconds) asm("_ZN4HPHP22fni_socket_set_timeoutERKNS_6ObjectEii");

/*
void HPHP::fni_define_syslog_variables()
_ZN4HPHP27fni_define_syslog_variablesEv

*/

void fh_define_syslog_variables() asm("_ZN4HPHP27fni_define_syslog_variablesEv");

/*
bool HPHP::fni_openlog(HPHP::String const&, int, int)
_ZN4HPHP11fni_openlogERKNS_6StringEii

(return value) => rax
ident => rdi
option => rsi
facility => rdx
*/

bool fh_openlog(Value* ident, int option, int facility) asm("_ZN4HPHP11fni_openlogERKNS_6StringEii");

/*
bool HPHP::fni_closelog()
_ZN4HPHP12fni_closelogEv

(return value) => rax
*/

bool fh_closelog() asm("_ZN4HPHP12fni_closelogEv");

/*
bool HPHP::fni_syslog(int, HPHP::String const&)
_ZN4HPHP10fni_syslogEiRKNS_6StringE

(return value) => rax
priority => rdi
message => rsi
*/

bool fh_syslog(int priority, Value* message) asm("_ZN4HPHP10fni_syslogEiRKNS_6StringE");

/*
long long HPHP::fni_cpu_get_count()
_ZN4HPHP17fni_cpu_get_countEv

(return value) => rax
*/

long long fh_cpu_get_count() asm("_ZN4HPHP17fni_cpu_get_countEv");

/*
HPHP::String HPHP::fni_cpu_get_model()
_ZN4HPHP17fni_cpu_get_modelEv

(return value) => rax
_rv => rdi
*/

Value* fh_cpu_get_model(Value* _rv) asm("_ZN4HPHP17fni_cpu_get_modelEv");

/*
bool HPHP::fni_ob_start(HPHP::Variant const&, int, bool)
_ZN4HPHP12fni_ob_startERKNS_7VariantEib

(return value) => rax
output_callback => rdi
chunk_size => rsi
erase => rdx
*/

bool fh_ob_start(TypedValue* output_callback, int chunk_size, bool erase) asm("_ZN4HPHP12fni_ob_startERKNS_7VariantEib");

/*
void HPHP::fni_ob_clean()
_ZN4HPHP12fni_ob_cleanEv

*/

void fh_ob_clean() asm("_ZN4HPHP12fni_ob_cleanEv");

/*
void HPHP::fni_ob_flush()
_ZN4HPHP12fni_ob_flushEv

*/

void fh_ob_flush() asm("_ZN4HPHP12fni_ob_flushEv");

/*
bool HPHP::fni_ob_end_clean()
_ZN4HPHP16fni_ob_end_cleanEv

(return value) => rax
*/

bool fh_ob_end_clean() asm("_ZN4HPHP16fni_ob_end_cleanEv");

/*
bool HPHP::fni_ob_end_flush()
_ZN4HPHP16fni_ob_end_flushEv

(return value) => rax
*/

bool fh_ob_end_flush() asm("_ZN4HPHP16fni_ob_end_flushEv");

/*
void HPHP::fni_flush()
_ZN4HPHP9fni_flushEv

*/

void fh_flush() asm("_ZN4HPHP9fni_flushEv");

/*
HPHP::String HPHP::fni_ob_get_clean()
_ZN4HPHP16fni_ob_get_cleanEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_clean(Value* _rv) asm("_ZN4HPHP16fni_ob_get_cleanEv");

/*
HPHP::String HPHP::fni_ob_get_contents()
_ZN4HPHP19fni_ob_get_contentsEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_contents(Value* _rv) asm("_ZN4HPHP19fni_ob_get_contentsEv");

/*
HPHP::String HPHP::fni_ob_get_flush()
_ZN4HPHP16fni_ob_get_flushEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_flush(Value* _rv) asm("_ZN4HPHP16fni_ob_get_flushEv");

/*
long long HPHP::fni_ob_get_length()
_ZN4HPHP17fni_ob_get_lengthEv

(return value) => rax
*/

long long fh_ob_get_length() asm("_ZN4HPHP17fni_ob_get_lengthEv");

/*
long long HPHP::fni_ob_get_level()
_ZN4HPHP16fni_ob_get_levelEv

(return value) => rax
*/

long long fh_ob_get_level() asm("_ZN4HPHP16fni_ob_get_levelEv");

/*
HPHP::Array HPHP::fni_ob_get_status(bool)
_ZN4HPHP17fni_ob_get_statusEb

(return value) => rax
_rv => rdi
full_status => rsi
*/

Value* fh_ob_get_status(Value* _rv, bool full_status) asm("_ZN4HPHP17fni_ob_get_statusEb");

/*
HPHP::String HPHP::fni_ob_gzhandler(HPHP::String const&, int)
_ZN4HPHP16fni_ob_gzhandlerERKNS_6StringEi

(return value) => rax
_rv => rdi
buffer => rsi
mode => rdx
*/

Value* fh_ob_gzhandler(Value* _rv, Value* buffer, int mode) asm("_ZN4HPHP16fni_ob_gzhandlerERKNS_6StringEi");

/*
void HPHP::fni_ob_implicit_flush(bool)
_ZN4HPHP21fni_ob_implicit_flushEb

flag => rdi
*/

void fh_ob_implicit_flush(bool flag) asm("_ZN4HPHP21fni_ob_implicit_flushEb");

/*
HPHP::Array HPHP::fni_ob_list_handlers()
_ZN4HPHP20fni_ob_list_handlersEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_list_handlers(Value* _rv) asm("_ZN4HPHP20fni_ob_list_handlersEv");

/*
bool HPHP::fni_output_add_rewrite_var(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26fni_output_add_rewrite_varERKNS_6StringES2_

(return value) => rax
name => rdi
value => rsi
*/

bool fh_output_add_rewrite_var(Value* name, Value* value) asm("_ZN4HPHP26fni_output_add_rewrite_varERKNS_6StringES2_");

/*
bool HPHP::fni_output_reset_rewrite_vars()
_ZN4HPHP29fni_output_reset_rewrite_varsEv

(return value) => rax
*/

bool fh_output_reset_rewrite_vars() asm("_ZN4HPHP29fni_output_reset_rewrite_varsEv");

/*
void HPHP::fni_hphp_stats(HPHP::String const&, long long)
_ZN4HPHP14fni_hphp_statsERKNS_6StringEx

name => rdi
value => rsi
*/

void fh_hphp_stats(Value* name, long long value) asm("_ZN4HPHP14fni_hphp_statsERKNS_6StringEx");

/*
long long HPHP::fni_hphp_get_stats(HPHP::String const&)
_ZN4HPHP18fni_hphp_get_statsERKNS_6StringE

(return value) => rax
name => rdi
*/

long long fh_hphp_get_stats(Value* name) asm("_ZN4HPHP18fni_hphp_get_statsERKNS_6StringE");

/*
HPHP::Array HPHP::fni_hphp_get_iostatus()
_ZN4HPHP21fni_hphp_get_iostatusEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_get_iostatus(Value* _rv) asm("_ZN4HPHP21fni_hphp_get_iostatusEv");

/*
void HPHP::fni_hphp_set_iostatus_address(HPHP::String const&)
_ZN4HPHP29fni_hphp_set_iostatus_addressERKNS_6StringE

name => rdi
*/

void fh_hphp_set_iostatus_address(Value* name) asm("_ZN4HPHP29fni_hphp_set_iostatus_addressERKNS_6StringE");

/*
HPHP::String HPHP::fni_posix_ctermid()
_ZN4HPHP17fni_posix_ctermidEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_ctermid(Value* _rv) asm("_ZN4HPHP17fni_posix_ctermidEv");

/*
long long HPHP::fni_posix_get_last_error()
_ZN4HPHP24fni_posix_get_last_errorEv

(return value) => rax
*/

long long fh_posix_get_last_error() asm("_ZN4HPHP24fni_posix_get_last_errorEv");

/*
HPHP::String HPHP::fni_posix_getcwd()
_ZN4HPHP16fni_posix_getcwdEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_getcwd(Value* _rv) asm("_ZN4HPHP16fni_posix_getcwdEv");

/*
long long HPHP::fni_posix_getegid()
_ZN4HPHP17fni_posix_getegidEv

(return value) => rax
*/

long long fh_posix_getegid() asm("_ZN4HPHP17fni_posix_getegidEv");

/*
long long HPHP::fni_posix_geteuid()
_ZN4HPHP17fni_posix_geteuidEv

(return value) => rax
*/

long long fh_posix_geteuid() asm("_ZN4HPHP17fni_posix_geteuidEv");

/*
long long HPHP::fni_posix_getgid()
_ZN4HPHP16fni_posix_getgidEv

(return value) => rax
*/

long long fh_posix_getgid() asm("_ZN4HPHP16fni_posix_getgidEv");

/*
HPHP::Variant HPHP::fni_posix_getlogin()
_ZN4HPHP18fni_posix_getloginEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getlogin(TypedValue* _rv) asm("_ZN4HPHP18fni_posix_getloginEv");

/*
HPHP::Variant HPHP::fni_posix_getpgid(int)
_ZN4HPHP17fni_posix_getpgidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getpgid(TypedValue* _rv, int pid) asm("_ZN4HPHP17fni_posix_getpgidEi");

/*
long long HPHP::fni_posix_getpgrp()
_ZN4HPHP17fni_posix_getpgrpEv

(return value) => rax
*/

long long fh_posix_getpgrp() asm("_ZN4HPHP17fni_posix_getpgrpEv");

/*
long long HPHP::fni_posix_getpid()
_ZN4HPHP16fni_posix_getpidEv

(return value) => rax
*/

long long fh_posix_getpid() asm("_ZN4HPHP16fni_posix_getpidEv");

/*
long long HPHP::fni_posix_getppid()
_ZN4HPHP17fni_posix_getppidEv

(return value) => rax
*/

long long fh_posix_getppid() asm("_ZN4HPHP17fni_posix_getppidEv");

/*
HPHP::Variant HPHP::fni_posix_getsid(int)
_ZN4HPHP16fni_posix_getsidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getsid(TypedValue* _rv, int pid) asm("_ZN4HPHP16fni_posix_getsidEi");

/*
long long HPHP::fni_posix_getuid()
_ZN4HPHP16fni_posix_getuidEv

(return value) => rax
*/

long long fh_posix_getuid() asm("_ZN4HPHP16fni_posix_getuidEv");

/*
bool HPHP::fni_posix_initgroups(HPHP::String const&, int)
_ZN4HPHP20fni_posix_initgroupsERKNS_6StringEi

(return value) => rax
name => rdi
base_group_id => rsi
*/

bool fh_posix_initgroups(Value* name, int base_group_id) asm("_ZN4HPHP20fni_posix_initgroupsERKNS_6StringEi");

/*
bool HPHP::fni_posix_kill(int, int)
_ZN4HPHP14fni_posix_killEii

(return value) => rax
pid => rdi
sig => rsi
*/

bool fh_posix_kill(int pid, int sig) asm("_ZN4HPHP14fni_posix_killEii");

/*
bool HPHP::fni_posix_mkfifo(HPHP::String const&, int)
_ZN4HPHP16fni_posix_mkfifoERKNS_6StringEi

(return value) => rax
pathname => rdi
mode => rsi
*/

bool fh_posix_mkfifo(Value* pathname, int mode) asm("_ZN4HPHP16fni_posix_mkfifoERKNS_6StringEi");

/*
bool HPHP::fni_posix_setegid(int)
_ZN4HPHP17fni_posix_setegidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setegid(int gid) asm("_ZN4HPHP17fni_posix_setegidEi");

/*
bool HPHP::fni_posix_seteuid(int)
_ZN4HPHP17fni_posix_seteuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_seteuid(int uid) asm("_ZN4HPHP17fni_posix_seteuidEi");

/*
bool HPHP::fni_posix_setgid(int)
_ZN4HPHP16fni_posix_setgidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setgid(int gid) asm("_ZN4HPHP16fni_posix_setgidEi");

/*
bool HPHP::fni_posix_setpgid(int, int)
_ZN4HPHP17fni_posix_setpgidEii

(return value) => rax
pid => rdi
pgid => rsi
*/

bool fh_posix_setpgid(int pid, int pgid) asm("_ZN4HPHP17fni_posix_setpgidEii");

/*
long long HPHP::fni_posix_setsid()
_ZN4HPHP16fni_posix_setsidEv

(return value) => rax
*/

long long fh_posix_setsid() asm("_ZN4HPHP16fni_posix_setsidEv");

/*
bool HPHP::fni_posix_setuid(int)
_ZN4HPHP16fni_posix_setuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_setuid(int uid) asm("_ZN4HPHP16fni_posix_setuidEi");

/*
HPHP::String HPHP::fni_posix_strerror(int)
_ZN4HPHP18fni_posix_strerrorEi

(return value) => rax
_rv => rdi
errnum => rsi
*/

Value* fh_posix_strerror(Value* _rv, int errnum) asm("_ZN4HPHP18fni_posix_strerrorEi");

/*
long long HPHP::fni_pcntl_alarm(int)
_ZN4HPHP15fni_pcntl_alarmEi

(return value) => rax
seconds => rdi
*/

long long fh_pcntl_alarm(int seconds) asm("_ZN4HPHP15fni_pcntl_alarmEi");

/*
long long HPHP::fni_pcntl_wexitstatus(int)
_ZN4HPHP21fni_pcntl_wexitstatusEi

(return value) => rax
status => rdi
*/

long long fh_pcntl_wexitstatus(int status) asm("_ZN4HPHP21fni_pcntl_wexitstatusEi");

/*
bool HPHP::fni_pcntl_wifexited(int)
_ZN4HPHP19fni_pcntl_wifexitedEi

(return value) => rax
status => rdi
*/

bool fh_pcntl_wifexited(int status) asm("_ZN4HPHP19fni_pcntl_wifexitedEi");

/*
bool HPHP::fni_pcntl_wifsignaled(int)
_ZN4HPHP21fni_pcntl_wifsignaledEi

(return value) => rax
status => rdi
*/

bool fh_pcntl_wifsignaled(int status) asm("_ZN4HPHP21fni_pcntl_wifsignaledEi");

/*
bool HPHP::fni_pcntl_wifstopped(int)
_ZN4HPHP20fni_pcntl_wifstoppedEi

(return value) => rax
status => rdi
*/

bool fh_pcntl_wifstopped(int status) asm("_ZN4HPHP20fni_pcntl_wifstoppedEi");

/*
long long HPHP::fni_pcntl_wstopsig(int)
_ZN4HPHP18fni_pcntl_wstopsigEi

(return value) => rax
status => rdi
*/

long long fh_pcntl_wstopsig(int status) asm("_ZN4HPHP18fni_pcntl_wstopsigEi");

/*
long long HPHP::fni_pcntl_wtermsig(int)
_ZN4HPHP18fni_pcntl_wtermsigEi

(return value) => rax
status => rdi
*/

long long fh_pcntl_wtermsig(int status) asm("_ZN4HPHP18fni_pcntl_wtermsigEi");

/*
long long HPHP::fni_hphp_object_pointer(HPHP::Object const&)
_ZN4HPHP23fni_hphp_object_pointerERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_object_pointer(Value* obj) asm("_ZN4HPHP23fni_hphp_object_pointerERKNS_6ObjectE");

/*
HPHP::Object HPHP::fni_stream_context_create(HPHP::Array const&, HPHP::Array const&)
_ZN4HPHP25fni_stream_context_createERKNS_5ArrayES2_

(return value) => rax
_rv => rdi
options => rsi
params => rdx
*/

Value* fh_stream_context_create(Value* _rv, Value* options, Value* params) asm("_ZN4HPHP25fni_stream_context_createERKNS_5ArrayES2_");

/*
HPHP::Object HPHP::fni_stream_context_get_default(HPHP::Array const&)
_ZN4HPHP30fni_stream_context_get_defaultERKNS_5ArrayE

(return value) => rax
_rv => rdi
options => rsi
*/

Value* fh_stream_context_get_default(Value* _rv, Value* options) asm("_ZN4HPHP30fni_stream_context_get_defaultERKNS_5ArrayE");

/*
HPHP::Variant HPHP::fni_stream_context_get_options(HPHP::Object const&)
_ZN4HPHP30fni_stream_context_get_optionsERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream_or_context => rsi
*/

TypedValue* fh_stream_context_get_options(TypedValue* _rv, Value* stream_or_context) asm("_ZN4HPHP30fni_stream_context_get_optionsERKNS_6ObjectE");

/*
bool HPHP::fni_stream_context_set_option(HPHP::Object const&, HPHP::Variant const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP29fni_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_

(return value) => rax
stream_or_context => rdi
wrapper => rsi
option => rdx
value => rcx
*/

bool fh_stream_context_set_option(Value* stream_or_context, TypedValue* wrapper, Value* option, TypedValue* value) asm("_ZN4HPHP29fni_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_");

/*
bool HPHP::fni_stream_context_set_param(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP28fni_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
stream_or_context => rdi
params => rsi
*/

bool fh_stream_context_set_param(Value* stream_or_context, Value* params) asm("_ZN4HPHP28fni_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE");

/*
bool HPHP::fni_stream_encoding(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19fni_stream_encodingERKNS_6ObjectERKNS_6StringE

(return value) => rax
stream => rdi
encoding => rsi
*/

bool fh_stream_encoding(Value* stream, Value* encoding) asm("_ZN4HPHP19fni_stream_encodingERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::fni_stream_bucket_append(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP24fni_stream_bucket_appendERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_append(Value* brigade, Value* bucket) asm("_ZN4HPHP24fni_stream_bucket_appendERKNS_6ObjectES2_");

/*
void HPHP::fni_stream_bucket_prepend(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP25fni_stream_bucket_prependERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_prepend(Value* brigade, Value* bucket) asm("_ZN4HPHP25fni_stream_bucket_prependERKNS_6ObjectES2_");

/*
HPHP::Object HPHP::fni_stream_bucket_make_writeable(HPHP::Object const&)
_ZN4HPHP32fni_stream_bucket_make_writeableERKNS_6ObjectE

(return value) => rax
_rv => rdi
brigade => rsi
*/

Value* fh_stream_bucket_make_writeable(Value* _rv, Value* brigade) asm("_ZN4HPHP32fni_stream_bucket_make_writeableERKNS_6ObjectE");

/*
HPHP::Object HPHP::fni_stream_bucket_new(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21fni_stream_bucket_newERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
stream => rsi
buffer => rdx
*/

Value* fh_stream_bucket_new(Value* _rv, Value* stream, Value* buffer) asm("_ZN4HPHP21fni_stream_bucket_newERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::fni_stream_filter_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26fni_stream_filter_registerERKNS_6StringES2_

(return value) => rax
filtername => rdi
classname => rsi
*/

bool fh_stream_filter_register(Value* filtername, Value* classname) asm("_ZN4HPHP26fni_stream_filter_registerERKNS_6StringES2_");

/*
bool HPHP::fni_stream_filter_remove(HPHP::Object const&)
_ZN4HPHP24fni_stream_filter_removeERKNS_6ObjectE

(return value) => rax
stream_filter => rdi
*/

bool fh_stream_filter_remove(Value* stream_filter) asm("_ZN4HPHP24fni_stream_filter_removeERKNS_6ObjectE");

/*
HPHP::Object HPHP::fni_stream_filter_append(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP24fni_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_append(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP24fni_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

/*
HPHP::Object HPHP::fni_stream_filter_prepend(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP25fni_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_prepend(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP25fni_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

/*
HPHP::Array HPHP::fni_stream_get_filters()
_ZN4HPHP22fni_stream_get_filtersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_filters(Value* _rv) asm("_ZN4HPHP22fni_stream_get_filtersEv");

/*
HPHP::Variant HPHP::fni_stream_get_meta_data(HPHP::Object const&)
_ZN4HPHP24fni_stream_get_meta_dataERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream => rsi
*/

TypedValue* fh_stream_get_meta_data(TypedValue* _rv, Value* stream) asm("_ZN4HPHP24fni_stream_get_meta_dataERKNS_6ObjectE");

/*
HPHP::Array HPHP::fni_stream_get_transports()
_ZN4HPHP25fni_stream_get_transportsEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_transports(Value* _rv) asm("_ZN4HPHP25fni_stream_get_transportsEv");

/*
HPHP::Array HPHP::fni_stream_get_wrappers()
_ZN4HPHP23fni_stream_get_wrappersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_wrappers(Value* _rv) asm("_ZN4HPHP23fni_stream_get_wrappersEv");

/*
bool HPHP::fni_stream_register_wrapper(HPHP::String const&, HPHP::String const&)
_ZN4HPHP27fni_stream_register_wrapperERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_register_wrapper(Value* protocol, Value* classname) asm("_ZN4HPHP27fni_stream_register_wrapperERKNS_6StringES2_");

/*
bool HPHP::fni_stream_wrapper_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP27fni_stream_wrapper_registerERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_wrapper_register(Value* protocol, Value* classname) asm("_ZN4HPHP27fni_stream_wrapper_registerERKNS_6StringES2_");

/*
bool HPHP::fni_stream_wrapper_restore(HPHP::String const&)
_ZN4HPHP26fni_stream_wrapper_restoreERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_restore(Value* protocol) asm("_ZN4HPHP26fni_stream_wrapper_restoreERKNS_6StringE");

/*
bool HPHP::fni_stream_wrapper_unregister(HPHP::String const&)
_ZN4HPHP29fni_stream_wrapper_unregisterERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_unregister(Value* protocol) asm("_ZN4HPHP29fni_stream_wrapper_unregisterERKNS_6StringE");

/*
HPHP::String HPHP::fni_stream_resolve_include_path(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP31fni_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
filename => rsi
context => rdx
*/

Value* fh_stream_resolve_include_path(Value* _rv, Value* filename, Value* context) asm("_ZN4HPHP31fni_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE");

/*
long long HPHP::fni_set_file_buffer(HPHP::Object const&, int)
_ZN4HPHP19fni_set_file_bufferERKNS_6ObjectEi

(return value) => rax
stream => rdi
buffer => rsi
*/

long long fh_set_file_buffer(Value* stream, int buffer) asm("_ZN4HPHP19fni_set_file_bufferERKNS_6ObjectEi");

/*
HPHP::Variant HPHP::fni_stream_socket_enable_crypto(HPHP::Object const&, bool, int, HPHP::Object const&)
_ZN4HPHP31fni_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_

(return value) => rax
_rv => rdi
stream => rsi
enable => rdx
crypto_type => rcx
session_stream => r8
*/

TypedValue* fh_stream_socket_enable_crypto(TypedValue* _rv, Value* stream, bool enable, int crypto_type, Value* session_stream) asm("_ZN4HPHP31fni_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_");

/*
HPHP::String HPHP::fni_addcslashes(HPHP::String const&, HPHP::String const&)
_ZN4HPHP15fni_addcslashesERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_addcslashes(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP15fni_addcslashesERKNS_6StringES2_");

/*
HPHP::String HPHP::fni_stripcslashes(HPHP::String const&)
_ZN4HPHP17fni_stripcslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_stripcslashes(Value* _rv, Value* str) asm("_ZN4HPHP17fni_stripcslashesERKNS_6StringE");

/*
HPHP::String HPHP::fni_addslashes(HPHP::String const&)
_ZN4HPHP14fni_addslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_addslashes(Value* _rv, Value* str) asm("_ZN4HPHP14fni_addslashesERKNS_6StringE");

/*
HPHP::String HPHP::fni_stripslashes(HPHP::String const&)
_ZN4HPHP16fni_stripslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_stripslashes(Value* _rv, Value* str) asm("_ZN4HPHP16fni_stripslashesERKNS_6StringE");

/*
HPHP::String HPHP::fni_bin2hex(HPHP::String const&)
_ZN4HPHP11fni_bin2hexERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_bin2hex(Value* _rv, Value* str) asm("_ZN4HPHP11fni_bin2hexERKNS_6StringE");

/*
HPHP::Variant HPHP::fni_hex2bin(HPHP::String const&)
_ZN4HPHP11fni_hex2binERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_hex2bin(TypedValue* _rv, Value* str) asm("_ZN4HPHP11fni_hex2binERKNS_6StringE");

/*
HPHP::String HPHP::fni_nl2br(HPHP::String const&)
_ZN4HPHP9fni_nl2brERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_nl2br(Value* _rv, Value* str) asm("_ZN4HPHP9fni_nl2brERKNS_6StringE");

/*
HPHP::String HPHP::fni_quotemeta(HPHP::String const&)
_ZN4HPHP13fni_quotemetaERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quotemeta(Value* _rv, Value* str) asm("_ZN4HPHP13fni_quotemetaERKNS_6StringE");

/*
HPHP::String HPHP::fni_str_shuffle(HPHP::String const&)
_ZN4HPHP15fni_str_shuffleERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_str_shuffle(Value* _rv, Value* str) asm("_ZN4HPHP15fni_str_shuffleERKNS_6StringE");

/*
HPHP::String HPHP::fni_strrev(HPHP::String const&)
_ZN4HPHP10fni_strrevERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strrev(Value* _rv, Value* str) asm("_ZN4HPHP10fni_strrevERKNS_6StringE");

/*
HPHP::String HPHP::fni_strtolower(HPHP::String const&)
_ZN4HPHP14fni_strtolowerERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strtolower(Value* _rv, Value* str) asm("_ZN4HPHP14fni_strtolowerERKNS_6StringE");

/*
HPHP::String HPHP::fni_strtoupper(HPHP::String const&)
_ZN4HPHP14fni_strtoupperERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strtoupper(Value* _rv, Value* str) asm("_ZN4HPHP14fni_strtoupperERKNS_6StringE");

/*
HPHP::String HPHP::fni_ucfirst(HPHP::String const&)
_ZN4HPHP11fni_ucfirstERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_ucfirst(Value* _rv, Value* str) asm("_ZN4HPHP11fni_ucfirstERKNS_6StringE");

/*
HPHP::String HPHP::fni_ucwords(HPHP::String const&)
_ZN4HPHP11fni_ucwordsERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_ucwords(Value* _rv, Value* str) asm("_ZN4HPHP11fni_ucwordsERKNS_6StringE");

/*
HPHP::String HPHP::fni_strip_tags(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14fni_strip_tagsERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
allowable_tags => rdx
*/

Value* fh_strip_tags(Value* _rv, Value* str, Value* allowable_tags) asm("_ZN4HPHP14fni_strip_tagsERKNS_6StringES2_");

/*
HPHP::String HPHP::fni_trim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8fni_trimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_trim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP8fni_trimERKNS_6StringES2_");

/*
HPHP::String HPHP::fni_ltrim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9fni_ltrimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_ltrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP9fni_ltrimERKNS_6StringES2_");

/*
HPHP::String HPHP::fni_rtrim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9fni_rtrimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_rtrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP9fni_rtrimERKNS_6StringES2_");

/*
HPHP::String HPHP::fni_chop(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8fni_chopERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_chop(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP8fni_chopERKNS_6StringES2_");

/*
HPHP::Variant HPHP::fni_explode(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP11fni_explodeERKNS_6StringES2_i

(return value) => rax
_rv => rdi
delimiter => rsi
str => rdx
limit => rcx
*/

TypedValue* fh_explode(TypedValue* _rv, Value* delimiter, Value* str, int limit) asm("_ZN4HPHP11fni_explodeERKNS_6StringES2_i");

/*
HPHP::String HPHP::fni_join(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP8fni_joinERKNS_7VariantES2_

(return value) => rax
_rv => rdi
glue => rsi
pieces => rdx
*/

Value* fh_join(Value* _rv, TypedValue* glue, TypedValue* pieces) asm("_ZN4HPHP8fni_joinERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::fni_str_split(HPHP::String const&, int)
_ZN4HPHP13fni_str_splitERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
split_length => rdx
*/

TypedValue* fh_str_split(TypedValue* _rv, Value* str, int split_length) asm("_ZN4HPHP13fni_str_splitERKNS_6StringEi");

/*
HPHP::Variant HPHP::fni_chunk_split(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP15fni_chunk_splitERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
body => rsi
chunklen => rdx
end => rcx
*/

TypedValue* fh_chunk_split(TypedValue* _rv, Value* body, int chunklen, Value* end) asm("_ZN4HPHP15fni_chunk_splitERKNS_6StringEiS2_");

/*
HPHP::Variant HPHP::fni_substr(HPHP::String const&, int, int)
_ZN4HPHP10fni_substrERKNS_6StringEii

(return value) => rax
_rv => rdi
str => rsi
start => rdx
length => rcx
*/

TypedValue* fh_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP10fni_substrERKNS_6StringEii");

/*
HPHP::String HPHP::fni_str_pad(HPHP::String const&, int, HPHP::String const&, int)
_ZN4HPHP11fni_str_padERKNS_6StringEiS2_i

(return value) => rax
_rv => rdi
input => rsi
pad_length => rdx
pad_string => rcx
pad_type => r8
*/

Value* fh_str_pad(Value* _rv, Value* input, int pad_length, Value* pad_string, int pad_type) asm("_ZN4HPHP11fni_str_padERKNS_6StringEiS2_i");

/*
HPHP::String HPHP::fni_str_repeat(HPHP::String const&, int)
_ZN4HPHP14fni_str_repeatERKNS_6StringEi

(return value) => rax
_rv => rdi
input => rsi
multiplier => rdx
*/

Value* fh_str_repeat(Value* _rv, Value* input, int multiplier) asm("_ZN4HPHP14fni_str_repeatERKNS_6StringEi");

/*
HPHP::Variant HPHP::fni_wordwrap(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP12fni_wordwrapERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
width => rdx
wordbreak => rcx
cut => r8
*/

TypedValue* fh_wordwrap(TypedValue* _rv, Value* str, int width, Value* wordbreak, bool cut) asm("_ZN4HPHP12fni_wordwrapERKNS_6StringEiS2_b");

/*
HPHP::String HPHP::fni_html_entity_decode(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP22fni_html_entity_decodeERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
*/

Value* fh_html_entity_decode(Value* _rv, Value* str, int quote_style, Value* charset) asm("_ZN4HPHP22fni_html_entity_decodeERKNS_6StringEiS2_");

/*
HPHP::String HPHP::fni_htmlentities(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP16fni_htmlentitiesERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
double_encode => r8
*/

Value* fh_htmlentities(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP16fni_htmlentitiesERKNS_6StringEiS2_b");

/*
HPHP::String HPHP::fni_htmlspecialchars_decode(HPHP::String const&, int)
_ZN4HPHP27fni_htmlspecialchars_decodeERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
*/

Value* fh_htmlspecialchars_decode(Value* _rv, Value* str, int quote_style) asm("_ZN4HPHP27fni_htmlspecialchars_decodeERKNS_6StringEi");

/*
HPHP::String HPHP::fni_htmlspecialchars(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP20fni_htmlspecialcharsERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
double_encode => r8
*/

Value* fh_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP20fni_htmlspecialcharsERKNS_6StringEiS2_b");

/*
HPHP::String HPHP::fni_fb_htmlspecialchars(HPHP::String const&, int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23fni_fb_htmlspecialcharsERKNS_6StringEiS2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
extra => r8
*/

Value* fh_fb_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, Value* extra) asm("_ZN4HPHP23fni_fb_htmlspecialcharsERKNS_6StringEiS2_RKNS_5ArrayE");

/*
HPHP::String HPHP::fni_quoted_printable_encode(HPHP::String const&)
_ZN4HPHP27fni_quoted_printable_encodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quoted_printable_encode(Value* _rv, Value* str) asm("_ZN4HPHP27fni_quoted_printable_encodeERKNS_6StringE");

/*
HPHP::String HPHP::fni_quoted_printable_decode(HPHP::String const&)
_ZN4HPHP27fni_quoted_printable_decodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quoted_printable_decode(Value* _rv, Value* str) asm("_ZN4HPHP27fni_quoted_printable_decodeERKNS_6StringE");

/*
HPHP::Variant HPHP::fni_convert_uudecode(HPHP::String const&)
_ZN4HPHP20fni_convert_uudecodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_convert_uudecode(TypedValue* _rv, Value* data) asm("_ZN4HPHP20fni_convert_uudecodeERKNS_6StringE");

/*
HPHP::Variant HPHP::fni_convert_uuencode(HPHP::String const&)
_ZN4HPHP20fni_convert_uuencodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_convert_uuencode(TypedValue* _rv, Value* data) asm("_ZN4HPHP20fni_convert_uuencodeERKNS_6StringE");

/*
HPHP::String HPHP::fni_str_rot13(HPHP::String const&)
_ZN4HPHP13fni_str_rot13ERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_str_rot13(Value* _rv, Value* str) asm("_ZN4HPHP13fni_str_rot13ERKNS_6StringE");

/*
long long HPHP::fni_crc32(HPHP::String const&)
_ZN4HPHP9fni_crc32ERKNS_6StringE

(return value) => rax
str => rdi
*/

long long fh_crc32(Value* str) asm("_ZN4HPHP9fni_crc32ERKNS_6StringE");

/*
HPHP::String HPHP::fni_crypt(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9fni_cryptERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
salt => rdx
*/

Value* fh_crypt(Value* _rv, Value* str, Value* salt) asm("_ZN4HPHP9fni_cryptERKNS_6StringES2_");

/*
HPHP::String HPHP::fni_md5(HPHP::String const&, bool)
_ZN4HPHP7fni_md5ERKNS_6StringEb

(return value) => rax
_rv => rdi
str => rsi
raw_output => rdx
*/

Value* fh_md5(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP7fni_md5ERKNS_6StringEb");

/*
HPHP::String HPHP::fni_sha1(HPHP::String const&, bool)
_ZN4HPHP8fni_sha1ERKNS_6StringEb

(return value) => rax
_rv => rdi
str => rsi
raw_output => rdx
*/

Value* fh_sha1(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP8fni_sha1ERKNS_6StringEb");

/*
HPHP::Array HPHP::fni_get_html_translation_table(int, int)
_ZN4HPHP30fni_get_html_translation_tableEii

(return value) => rax
_rv => rdi
table => rsi
quote_style => rdx
*/

Value* fh_get_html_translation_table(Value* _rv, int table, int quote_style) asm("_ZN4HPHP30fni_get_html_translation_tableEii");

/*
HPHP::String HPHP::fni_nl_langinfo(int)
_ZN4HPHP15fni_nl_langinfoEi

(return value) => rax
_rv => rdi
item => rsi
*/

Value* fh_nl_langinfo(Value* _rv, int item) asm("_ZN4HPHP15fni_nl_langinfoEi");

/*
HPHP::Variant HPHP::fni_printf(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP10fni_printfEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

TypedValue* fh_printf(TypedValue* _rv, long long _argc, Value* format, Value* _argv) asm("_ZN4HPHP10fni_printfEiRKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::fni_vprintf(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP11fni_vprintfERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
format => rsi
args => rdx
*/

TypedValue* fh_vprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP11fni_vprintfERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::fni_sprintf(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP11fni_sprintfEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

TypedValue* fh_sprintf(TypedValue* _rv, long long _argc, Value* format, Value* _argv) asm("_ZN4HPHP11fni_sprintfEiRKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::fni_vsprintf(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP12fni_vsprintfERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
format => rsi
args => rdx
*/

TypedValue* fh_vsprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP12fni_vsprintfERKNS_6StringERKNS_5ArrayE");

/*
HPHP::String HPHP::fni_chr(long long)
_ZN4HPHP7fni_chrEx

(return value) => rax
_rv => rdi
ascii => rsi
*/

Value* fh_chr(Value* _rv, long long ascii) asm("_ZN4HPHP7fni_chrEx");

/*
long long HPHP::fni_ord(HPHP::String const&)
_ZN4HPHP7fni_ordERKNS_6StringE

(return value) => rax
str => rdi
*/

long long fh_ord(Value* str) asm("_ZN4HPHP7fni_ordERKNS_6StringE");

/*
HPHP::Variant HPHP::fni_money_format(HPHP::String const&, double)
_ZN4HPHP16fni_money_formatERKNS_6StringEd

(return value) => rax
_rv => rdi
format => rsi
number => xmm0
*/

TypedValue* fh_money_format(TypedValue* _rv, Value* format, double number) asm("_ZN4HPHP16fni_money_formatERKNS_6StringEd");

/*
long long HPHP::fni_strcmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP10fni_strcmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strcmp(Value* str1, Value* str2) asm("_ZN4HPHP10fni_strcmpERKNS_6StringES2_");

/*
long long HPHP::fni_strncmp(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP11fni_strncmpERKNS_6StringES2_i

(return value) => rax
str1 => rdi
str2 => rsi
len => rdx
*/

long long fh_strncmp(Value* str1, Value* str2, int len) asm("_ZN4HPHP11fni_strncmpERKNS_6StringES2_i");

/*
long long HPHP::fni_strnatcmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13fni_strnatcmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strnatcmp(Value* str1, Value* str2) asm("_ZN4HPHP13fni_strnatcmpERKNS_6StringES2_");

/*
long long HPHP::fni_strcasecmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14fni_strcasecmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP14fni_strcasecmpERKNS_6StringES2_");

/*
long long HPHP::fni_strncasecmp(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP15fni_strncasecmpERKNS_6StringES2_i

(return value) => rax
str1 => rdi
str2 => rsi
len => rdx
*/

long long fh_strncasecmp(Value* str1, Value* str2, int len) asm("_ZN4HPHP15fni_strncasecmpERKNS_6StringES2_i");

/*
long long HPHP::fni_strnatcasecmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP17fni_strnatcasecmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strnatcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP17fni_strnatcasecmpERKNS_6StringES2_");

/*
long long HPHP::fni_strcoll(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11fni_strcollERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strcoll(Value* str1, Value* str2) asm("_ZN4HPHP11fni_strcollERKNS_6StringES2_");

/*
HPHP::Variant HPHP::fni_strchr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP10fni_strchrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_strchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP10fni_strchrERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_strlen(HPHP::Variant const&)
_ZN4HPHP10fni_strlenERKNS_7VariantE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_strlen(TypedValue* _rv, TypedValue* str) asm("_ZN4HPHP10fni_strlenERKNS_7VariantE");

/*
long long HPHP::fni_levenshtein(HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP15fni_levenshteinERKNS_6StringES2_iii

(return value) => rax
str1 => rdi
str2 => rsi
cost_ins => rdx
cost_rep => rcx
cost_del => r8
*/

long long fh_levenshtein(Value* str1, Value* str2, int cost_ins, int cost_rep, int cost_del) asm("_ZN4HPHP15fni_levenshteinERKNS_6StringES2_iii");

/*
long long HPHP::fni_similar_text(HPHP::String const&, HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP16fni_similar_textERKNS_6StringES2_RKNS_14VRefParamValueE

(return value) => rax
first => rdi
second => rsi
percent => rdx
*/

long long fh_similar_text(Value* first, Value* second, TypedValue* percent) asm("_ZN4HPHP16fni_similar_textERKNS_6StringES2_RKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::fni_soundex(HPHP::String const&)
_ZN4HPHP11fni_soundexERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_soundex(TypedValue* _rv, Value* str) asm("_ZN4HPHP11fni_soundexERKNS_6StringE");

/*
HPHP::Variant HPHP::fni_metaphone(HPHP::String const&, int)
_ZN4HPHP13fni_metaphoneERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
phones => rdx
*/

TypedValue* fh_metaphone(TypedValue* _rv, Value* str, int phones) asm("_ZN4HPHP13fni_metaphoneERKNS_6StringEi");

/*
HPHP::String HPHP::fni_rawurldecode(HPHP::String const&)
_ZN4HPHP16fni_rawurldecodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_rawurldecode(Value* _rv, Value* str) asm("_ZN4HPHP16fni_rawurldecodeERKNS_6StringE");

/*
HPHP::String HPHP::fni_rawurlencode(HPHP::String const&)
_ZN4HPHP16fni_rawurlencodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_rawurlencode(Value* _rv, Value* str) asm("_ZN4HPHP16fni_rawurlencodeERKNS_6StringE");

/*
HPHP::String HPHP::fni_urldecode(HPHP::String const&)
_ZN4HPHP13fni_urldecodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_urldecode(Value* _rv, Value* str) asm("_ZN4HPHP13fni_urldecodeERKNS_6StringE");

/*
HPHP::String HPHP::fni_urlencode(HPHP::String const&)
_ZN4HPHP13fni_urlencodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_urlencode(Value* _rv, Value* str) asm("_ZN4HPHP13fni_urlencodeERKNS_6StringE");

/*
bool HPHP::fni_is_bool(HPHP::Variant const&)
_ZN4HPHP11fni_is_boolERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_bool(TypedValue* var) asm("_ZN4HPHP11fni_is_boolERKNS_7VariantE");

/*
bool HPHP::fni_is_int(HPHP::Variant const&)
_ZN4HPHP10fni_is_intERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_int(TypedValue* var) asm("_ZN4HPHP10fni_is_intERKNS_7VariantE");

/*
bool HPHP::fni_is_integer(HPHP::Variant const&)
_ZN4HPHP14fni_is_integerERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_integer(TypedValue* var) asm("_ZN4HPHP14fni_is_integerERKNS_7VariantE");

/*
bool HPHP::fni_is_long(HPHP::Variant const&)
_ZN4HPHP11fni_is_longERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_long(TypedValue* var) asm("_ZN4HPHP11fni_is_longERKNS_7VariantE");

/*
bool HPHP::fni_is_double(HPHP::Variant const&)
_ZN4HPHP13fni_is_doubleERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_double(TypedValue* var) asm("_ZN4HPHP13fni_is_doubleERKNS_7VariantE");

/*
bool HPHP::fni_is_float(HPHP::Variant const&)
_ZN4HPHP12fni_is_floatERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_float(TypedValue* var) asm("_ZN4HPHP12fni_is_floatERKNS_7VariantE");

/*
bool HPHP::fni_is_numeric(HPHP::Variant const&)
_ZN4HPHP14fni_is_numericERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_numeric(TypedValue* var) asm("_ZN4HPHP14fni_is_numericERKNS_7VariantE");

/*
bool HPHP::fni_is_real(HPHP::Variant const&)
_ZN4HPHP11fni_is_realERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_real(TypedValue* var) asm("_ZN4HPHP11fni_is_realERKNS_7VariantE");

/*
bool HPHP::fni_is_string(HPHP::Variant const&)
_ZN4HPHP13fni_is_stringERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_string(TypedValue* var) asm("_ZN4HPHP13fni_is_stringERKNS_7VariantE");

/*
bool HPHP::fni_is_scalar(HPHP::Variant const&)
_ZN4HPHP13fni_is_scalarERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_scalar(TypedValue* var) asm("_ZN4HPHP13fni_is_scalarERKNS_7VariantE");

/*
bool HPHP::fni_is_array(HPHP::Variant const&)
_ZN4HPHP12fni_is_arrayERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_array(TypedValue* var) asm("_ZN4HPHP12fni_is_arrayERKNS_7VariantE");

/*
bool HPHP::fni_is_resource(HPHP::Variant const&)
_ZN4HPHP15fni_is_resourceERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_resource(TypedValue* var) asm("_ZN4HPHP15fni_is_resourceERKNS_7VariantE");

/*
bool HPHP::fni_is_null(HPHP::Variant const&)
_ZN4HPHP11fni_is_nullERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_null(TypedValue* var) asm("_ZN4HPHP11fni_is_nullERKNS_7VariantE");

/*
long long HPHP::fni_intval(HPHP::Variant const&, long long)
_ZN4HPHP10fni_intvalERKNS_7VariantEx

(return value) => rax
v => rdi
base => rsi
*/

long long fh_intval(TypedValue* v, long long base) asm("_ZN4HPHP10fni_intvalERKNS_7VariantEx");

/*
double HPHP::fni_doubleval(HPHP::Variant const&)
_ZN4HPHP13fni_doublevalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_doubleval(TypedValue* v) asm("_ZN4HPHP13fni_doublevalERKNS_7VariantE");

/*
double HPHP::fni_floatval(HPHP::Variant const&)
_ZN4HPHP12fni_floatvalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_floatval(TypedValue* v) asm("_ZN4HPHP12fni_floatvalERKNS_7VariantE");

/*
HPHP::String HPHP::fni_strval(HPHP::Variant const&)
_ZN4HPHP10fni_strvalERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_strval(Value* _rv, TypedValue* v) asm("_ZN4HPHP10fni_strvalERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_unserialize(HPHP::String const&)
_ZN4HPHP15fni_unserializeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_unserialize(TypedValue* _rv, Value* str) asm("_ZN4HPHP15fni_unserializeERKNS_6StringE");

/*
HPHP::String HPHP::fni_zlib_get_coding_type()
_ZN4HPHP24fni_zlib_get_coding_typeEv

(return value) => rax
_rv => rdi
*/

Value* fh_zlib_get_coding_type(Value* _rv) asm("_ZN4HPHP24fni_zlib_get_coding_typeEv");

/*
bool HPHP::fni_gzclose(HPHP::Object const&)
_ZN4HPHP11fni_gzcloseERKNS_6ObjectE

(return value) => rax
zp => rdi
*/

bool fh_gzclose(Value* zp) asm("_ZN4HPHP11fni_gzcloseERKNS_6ObjectE");

/*
bool HPHP::fni_gzrewind(HPHP::Object const&)
_ZN4HPHP12fni_gzrewindERKNS_6ObjectE

(return value) => rax
zp => rdi
*/

bool fh_gzrewind(Value* zp) asm("_ZN4HPHP12fni_gzrewindERKNS_6ObjectE");

/*
bool HPHP::fni_gzeof(HPHP::Object const&)
_ZN4HPHP9fni_gzeofERKNS_6ObjectE

(return value) => rax
zp => rdi
*/

bool fh_gzeof(Value* zp) asm("_ZN4HPHP9fni_gzeofERKNS_6ObjectE");

/*
HPHP::Variant HPHP::fni_gzgetc(HPHP::Object const&)
_ZN4HPHP10fni_gzgetcERKNS_6ObjectE

(return value) => rax
_rv => rdi
zp => rsi
*/

TypedValue* fh_gzgetc(TypedValue* _rv, Value* zp) asm("_ZN4HPHP10fni_gzgetcERKNS_6ObjectE");

/*
HPHP::Variant HPHP::fni_gzgets(HPHP::Object const&, long long)
_ZN4HPHP10fni_gzgetsERKNS_6ObjectEx

(return value) => rax
_rv => rdi
zp => rsi
length => rdx
*/

TypedValue* fh_gzgets(TypedValue* _rv, Value* zp, long long length) asm("_ZN4HPHP10fni_gzgetsERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::fni_gzgetss(HPHP::Object const&, long long, HPHP::String const&)
_ZN4HPHP11fni_gzgetssERKNS_6ObjectExRKNS_6StringE

(return value) => rax
_rv => rdi
zp => rsi
length => rdx
allowable_tags => rcx
*/

TypedValue* fh_gzgetss(TypedValue* _rv, Value* zp, long long length, Value* allowable_tags) asm("_ZN4HPHP11fni_gzgetssERKNS_6ObjectExRKNS_6StringE");

/*
HPHP::Variant HPHP::fni_gzread(HPHP::Object const&, long long)
_ZN4HPHP10fni_gzreadERKNS_6ObjectEx

(return value) => rax
_rv => rdi
zp => rsi
length => rdx
*/

TypedValue* fh_gzread(TypedValue* _rv, Value* zp, long long length) asm("_ZN4HPHP10fni_gzreadERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::fni_gzpassthru(HPHP::Object const&)
_ZN4HPHP14fni_gzpassthruERKNS_6ObjectE

(return value) => rax
_rv => rdi
zp => rsi
*/

TypedValue* fh_gzpassthru(TypedValue* _rv, Value* zp) asm("_ZN4HPHP14fni_gzpassthruERKNS_6ObjectE");

/*
HPHP::Variant HPHP::fni_gzseek(HPHP::Object const&, long long, long long)
_ZN4HPHP10fni_gzseekERKNS_6ObjectExx

(return value) => rax
_rv => rdi
zp => rsi
offset => rdx
whence => rcx
*/

TypedValue* fh_gzseek(TypedValue* _rv, Value* zp, long long offset, long long whence) asm("_ZN4HPHP10fni_gzseekERKNS_6ObjectExx");

/*
HPHP::Variant HPHP::fni_gztell(HPHP::Object const&)
_ZN4HPHP10fni_gztellERKNS_6ObjectE

(return value) => rax
_rv => rdi
zp => rsi
*/

TypedValue* fh_gztell(TypedValue* _rv, Value* zp) asm("_ZN4HPHP10fni_gztellERKNS_6ObjectE");

/*
HPHP::Variant HPHP::fni_gzwrite(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP11fni_gzwriteERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
zp => rsi
str => rdx
length => rcx
*/

TypedValue* fh_gzwrite(TypedValue* _rv, Value* zp, Value* str, long long length) asm("_ZN4HPHP11fni_gzwriteERKNS_6ObjectERKNS_6StringEx");

/*
HPHP::Variant HPHP::fni_gzputs(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP10fni_gzputsERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
zp => rsi
str => rdx
length => rcx
*/

TypedValue* fh_gzputs(TypedValue* _rv, Value* zp, Value* str, long long length) asm("_ZN4HPHP10fni_gzputsERKNS_6ObjectERKNS_6StringEx");


} // !HPHP

