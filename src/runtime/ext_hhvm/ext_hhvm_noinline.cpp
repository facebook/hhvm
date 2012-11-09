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
#include <runtime/ext/ext.h>

/*
 * This file contains non-inline definitions of some extension
 * functions that are only defined inline in hphpc.
 *
 * In HHVM we need actual addresses for these functions to jump to.
 */

namespace HPHP {

bool fni_apc_compile_file(String const& filename, bool atomic, long long cache_id) {
  return f_apc_compile_file(filename, atomic, cache_id);
}

bool fni_apc_define_constants(String const& key, String const& constants, bool case_sensitive, long long cache_id) {
  return f_apc_define_constants(key, constants, case_sensitive, cache_id);
}

bool fni_apc_load_constants(String const& key, bool case_sensitive, long long cache_id) {
  return f_apc_load_constants(key, case_sensitive, cache_id);
}

Array fni_apc_sma_info(bool limited) {
  return f_apc_sma_info(limited);
}

Array fni_apc_filehits() {
  return f_apc_filehits();
}

Variant fni_apc_delete_file(Variant const& keys, long long cache_id) {
  return f_apc_delete_file(keys, cache_id);
}

Variant fni_apc_bin_dump(long long cache_id, Variant const& filter) {
  return f_apc_bin_dump(cache_id, filter);
}

bool fni_apc_bin_load(String const& data, long long flags, long long cache_id) {
  return f_apc_bin_load(data, flags, cache_id);
}

Variant fni_apc_bin_dumpfile(long long cache_id, Variant const& filter, String const& filename, long long flags, Object const& context) {
  return f_apc_bin_dumpfile(cache_id, filter, filename, flags, context);
}

bool fni_apc_bin_loadfile(String const& filename, Object const& context, long long flags, long long cache_id) {
  return f_apc_bin_loadfile(filename, context, flags, cache_id);
}

Variant fni_array_fill(int start_index, int num, Variant const& value) {
  return f_array_fill(start_index, num, value);
}

bool fni_key_exists(Variant const& key, Variant const& search) {
  return f_key_exists(key, search);
}

Variant fni_array_pop(VRefParamValue const& array) {
  return f_array_pop(ref(array));
}

Variant fni_array_shift(VRefParamValue const& array) {
  return f_array_shift(ref(array));
}

int64 fni_sizeof(Variant const& var, bool recursive) {
  return f_sizeof(var, recursive);
}

Variant fni_each(VRefParamValue const& array) {
  return f_each(ref(array));
}

Variant fni_current(VRefParamValue const& array) {
  return f_current(ref(array));
}

Variant fni_hphp_current_ref(VRefParamValue const& array) {
  return strongBind(f_hphp_current_ref(ref(array)));
}

Variant fni_next(VRefParamValue const& array) {
  return f_next(ref(array));
}

Variant fni_pos(VRefParamValue const& array) {
  return f_pos(ref(array));
}

Variant fni_prev(VRefParamValue const& array) {
  return f_prev(ref(array));
}

Variant fni_reset(VRefParamValue const& array) {
  return f_reset(ref(array));
}

Variant fni_end(VRefParamValue const& array) {
  return f_end(ref(array));
}

Variant fni_key(VRefParamValue const& array) {
  return f_key(ref(array));
}

Variant fni_bzclose(Object const& bz) {
  return f_bzclose(bz);
}

Variant fni_bzread(Object const& bz, int length) {
  return f_bzread(bz, length);
}

Variant fni_bzwrite(Object const& bz, String const& data, int length) {
  return f_bzwrite(bz, data, length);
}

bool fni_checkdate(int month, int day, int year) {
  return f_checkdate(month, day, year);
}

Object fni_date_add(CObjRef datetime, CObjRef interval) {
  return f_date_add(datetime, interval);
}

Object fni_date_create_from_format(CStrRef format,
                                   CStrRef time,
                                   CObjRef timezone = null_object) {
  return f_date_create_from_format(format, time, timezone);
}

Object fni_date_create(String const& time, Object const& timezone) {
  return f_date_create(time, timezone);
}

void fni_date_date_set(Object const& object, int year, int month, int day) {
  return f_date_date_set(object, year, month, day);
}

String fni_date_default_timezone_get() {
  return f_date_default_timezone_get();
}

bool fni_date_default_timezone_set(String const& name) {
  return f_date_default_timezone_set(name);
}

Object fni_date_diff(CObjRef datetime, CObjRef datetime2,
                     bool absolute = false) {
  return f_date_diff(datetime, datetime2, absolute);
}

String fni_date_format(Object const& object, String const& format) {
  return f_date_format(object, format);
}

Array fni_date_get_last_errors() {
  return f_date_get_last_errors();
}

Object fni_date_interval_create_from_date_string(CStrRef time) {
  return f_date_interval_create_from_date_string(time);
}

String fni_date_interval_format(CObjRef interval, CStrRef format_spec) {
  return f_date_interval_format(interval, format_spec);
}

void fni_date_isodate_set(Object const& object, int year, int week, int day) {
  return f_date_isodate_set(object, year, week, day);
}

void fni_date_modify(Object const& object, String const& modify) {
  return f_date_modify(object, modify);
}

int64 fni_date_offset_get(Object const& object) {
  return f_date_offset_get(object);
}

Variant fni_date_parse(String const& date) {
  return f_date_parse(date);
}

Object fni_date_sub(CObjRef datetime, CObjRef interval) {
  return f_date_sub(datetime, interval);
}

Array fni_date_sun_info(long long ts, double latitude, double longitude) {
  return f_date_sun_info(ts, latitude, longitude);
}

Variant fni_date_sunrise(long long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) {
  return f_date_sunrise(timestamp, format, latitude, longitude, zenith, gmt_offset);
}

Variant fni_date_sunset(long long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) {
  return f_date_sunset(timestamp, format, latitude, longitude, zenith, gmt_offset);
}

void fni_date_time_set(Object const& object, int hour, int minute, int second) {
  return f_date_time_set(object, hour, minute, second);
}

int64 fni_date_timestamp_get(CObjRef datetime) {
  return f_date_timestamp_get(datetime);
}

Object fni_date_timestamp_set(CObjRef datetime, int64 timestamp) {
  return f_date_timestamp_set(datetime, timestamp);
}

Variant fni_date_timezone_get(Object const& object) {
  return f_date_timezone_get(object);
}

void fni_date_timezone_set(Object const& object, Object const& timezone) {
  return f_date_timezone_set(object, timezone);
}

Variant fni_date(String const& format, long long timestamp) {
  return f_date(format, timestamp);
}

Array fni_getdate(long long timestamp) {
  return f_getdate(timestamp);
}

Variant fni_gettimeofday(bool return_float) {
  return f_gettimeofday(return_float);
}

Variant fni_gmdate(String const& format, long long timestamp) {
  return f_gmdate(format, timestamp);
}

Variant fni_gmmktime(int hour, int minute, int second, int month, int day, int year) {
  return f_gmmktime(hour, minute, second, month, day, year);
}

String fni_gmstrftime(String const& format, long long timestamp) {
  return f_gmstrftime(format, timestamp);
}

Variant fni_idate(String const& format, long long timestamp) {
  return f_idate(format, timestamp);
}

Array fni_localtime(long long timestamp, bool is_associative) {
  return f_localtime(timestamp, is_associative);
}

Variant fni_microtime(bool get_as_float) {
  return f_microtime(get_as_float);
}

Variant fni_mktime(int hour, int minute, int second, int month, int day, int year) {
  return f_mktime(hour, minute, second, month, day, year);
}

Variant fni_strftime(String const& format, long long timestamp) {
  return f_strftime(format, timestamp);
}

Variant fni_strptime(String const& date, String const& format) {
  return f_strptime(date, format);
}

Variant fni_strtotime(String const& input, long long timestamp) {
  return f_strtotime(input, timestamp);
}

int64 fni_time() {
  return f_time();
}

Array fni_timezone_abbreviations_list() {
  return f_timezone_abbreviations_list();
}

Array fni_timezone_identifiers_list() {
  return f_timezone_identifiers_list();
}

Array fni_timezone_location_get(CObjRef timezone) {
  return f_timezone_location_get(timezone);
}

Variant fni_timezone_name_from_abbr(String const& abbr, int gmtoffset, bool isdst) {
  return f_timezone_name_from_abbr(abbr, gmtoffset, isdst);
}

String fni_timezone_name_get(Object const& object) {
  return f_timezone_name_get(object);
}

int64 fni_timezone_offset_get(Object const& object, Object const& dt) {
  return f_timezone_offset_get(object, dt);
}

Object fni_timezone_open(String const& timezone) {
  return f_timezone_open(timezone);
}

Array fni_timezone_transitions_get(Object const& object) {
  return f_timezone_transitions_get(object);
}

String fni_timezone_version_get() {
  return f_timezone_version_get();
}

#if FACEBOOK
void fni_fb_set_opcode(int opcode, String const& callback) {
  return f_fb_set_opcode(opcode, callback);
}

void fni_fb_reset_opcode(int opcode) {
  return f_fb_reset_opcode(opcode);
}

void fni_fb_config_coredump(bool enabled, int limit) {
  return f_fb_config_coredump(enabled, limit);
}

String fni_fb_backtrace(Variant const& exception) {
  return f_fb_backtrace(exception);
}

void fni_fb_add_included_file(String const& filepath) {
  return f_fb_add_included_file(filepath);
}

Variant fni_fb_request_timers() {
  return f_fb_request_timers();
}

String fni_fb_get_ape_version() {
  return f_fb_get_ape_version();
}
#endif

Variant fni_call_user_func_array(Variant const& function, Array const& params) {
  return f_call_user_func_array(function, params);
}

double fni_pi() {
  return f_pi();
}

bool fni_is_finite(double val) {
  return f_is_finite(val);
}

bool fni_is_infinite(double val) {
  return f_is_infinite(val);
}

bool fni_is_nan(double val) {
  return f_is_nan(val);
}

double fni_ceil(double value) {
  return f_ceil(value);
}

double fni_floor(double value) {
  return f_floor(value);
}

double fni_deg2rad(double number) {
  return f_deg2rad(number);
}

double fni_rad2deg(double number) {
  return f_rad2deg(number);
}

String fni_decbin(long long number) {
  return f_decbin(number);
}

String fni_dechex(long long number) {
  return f_dechex(number);
}

String fni_decoct(long long number) {
  return f_decoct(number);
}

Variant fni_bindec(String const& binary_string) {
  return f_bindec(binary_string);
}

Variant fni_hexdec(String const& hex_string) {
  return f_hexdec(hex_string);
}

Variant fni_octdec(String const& octal_string) {
  return f_octdec(octal_string);
}

double fni_exp(double arg) {
  return f_exp(arg);
}

double fni_expm1(double arg) {
  return f_expm1(arg);
}

double fni_log10(double arg) {
  return f_log10(arg);
}

double fni_log1p(double number) {
  return f_log1p(number);
}

double fni_log(double arg, double base) {
  return f_log(arg, base);
}

double fni_cos(double arg) {
  return f_cos(arg);
}

double fni_cosh(double arg) {
  return f_cosh(arg);
}

double fni_sin(double arg) {
  return f_sin(arg);
}

double fni_sinh(double arg) {
  return f_sinh(arg);
}

double fni_tan(double arg) {
  return f_tan(arg);
}

double fni_tanh(double arg) {
  return f_tanh(arg);
}

double fni_acos(double arg) {
  return f_acos(arg);
}

double fni_acosh(double arg) {
  return f_acosh(arg);
}

double fni_asin(double arg) {
  return f_asin(arg);
}

double fni_asinh(double arg) {
  return f_asinh(arg);
}

double fni_atan(double arg) {
  return f_atan(arg);
}

double fni_atanh(double arg) {
  return f_atanh(arg);
}

double fni_atan2(double y, double x) {
  return f_atan2(y, x);
}

double fni_hypot(double x, double y) {
  return f_hypot(x, y);
}

double fni_fmod(double x, double y) {
  return f_fmod(x, y);
}

double fni_sqrt(double arg) {
  return f_sqrt(arg);
}

long long fni_getrandmax() {
  return f_getrandmax();
}

long long fni_mt_getrandmax() {
  return f_mt_getrandmax();
}

long long fni_mt_rand(long long min, long long max) {
  return f_mt_rand(min, max);
}

double fni_lcg_value() {
  return f_lcg_value();
}

Variant fni_mysql_set_charset(String const& charset, Variant const& link_identifier) {
  return f_mysql_set_charset(charset, link_identifier);
}

Variant fni_mysql_ping(Variant const& link_identifier) {
  return f_mysql_ping(link_identifier);
}

Variant fni_mysql_client_encoding(Variant const& link_identifier) {
  return f_mysql_client_encoding(link_identifier);
}

Variant fni_mysql_close(Variant const& link_identifier) {
  return f_mysql_close(link_identifier);
}

String fni_mysql_get_client_info() {
  return f_mysql_get_client_info();
}

Variant fni_mysql_get_host_info(Variant const& link_identifier) {
  return f_mysql_get_host_info(link_identifier);
}

Variant fni_mysql_get_proto_info(Variant const& link_identifier) {
  return f_mysql_get_proto_info(link_identifier);
}

Variant fni_mysql_get_server_info(Variant const& link_identifier) {
  return f_mysql_get_server_info(link_identifier);
}

Variant fni_mysql_info(Variant const& link_identifier) {
  return f_mysql_info(link_identifier);
}

Variant fni_mysql_insert_id(Variant const& link_identifier) {
  return f_mysql_insert_id(link_identifier);
}

Variant fni_mysql_stat(Variant const& link_identifier) {
  return f_mysql_stat(link_identifier);
}

Variant fni_mysql_thread_id(Variant const& link_identifier) {
  return f_mysql_thread_id(link_identifier);
}

Variant fni_mysql_create_db(String const& db, Variant const& link_identifier) {
  return f_mysql_create_db(db, link_identifier);
}

Variant fni_mysql_select_db(String const& db, Variant const& link_identifier) {
  return f_mysql_select_db(db, link_identifier);
}

Variant fni_mysql_drop_db(String const& db, Variant const& link_identifier) {
  return f_mysql_drop_db(db, link_identifier);
}

Variant fni_mysql_affected_rows(Variant const& link_identifier) {
  return f_mysql_affected_rows(link_identifier);
}

Variant fni_mysql_db_query(String const& database, String const& query, Variant const& link_identifier) {
  return f_mysql_db_query(database, query, link_identifier);
}

Variant fni_mysql_list_fields(String const& database_name, String const& table_name, Variant const& link_identifier) {
  return f_mysql_list_fields(database_name, table_name, link_identifier);
}

Variant fni_mysql_db_name(Variant const& result, int row, Variant const& field) {
  return f_mysql_db_name(result, row, field);
}

Variant fni_mysql_tablename(Variant const& result, int i) {
  return f_mysql_tablename(result, i);
}

bool fni_checkdnsrr(String const& host, String const& type) {
  return f_checkdnsrr(host, type);
}

bool fni_getmxrr(String const& hostname, VRefParamValue const& mxhosts, VRefParamValue const& weight) {
  return f_getmxrr(hostname, ref(mxhosts), ref(weight));
}

Variant fni_socket_get_status(Object const& stream) {
  return f_socket_get_status(stream);
}

bool fni_socket_set_blocking(Object const& stream, int mode) {
  return f_socket_set_blocking(stream, mode);
}

bool fni_socket_set_timeout(Object const& stream, int seconds, int microseconds) {
  return f_socket_set_timeout(stream, seconds, microseconds);
}

void fni_define_syslog_variables() {
  return f_define_syslog_variables();
}

bool fni_openlog(String const& ident, int option, int facility) {
  return f_openlog(ident, option, facility);
}

bool fni_closelog() {
  return f_closelog();
}

bool fni_syslog(int priority, String const& message) {
  return f_syslog(priority, message);
}

int64 fni_cpu_get_count() {
  return f_cpu_get_count();
}

String fni_cpu_get_model() {
  return f_cpu_get_model();
}

bool fni_ob_start(Variant const& output_callback, int chunk_size, bool erase) {
  return f_ob_start(output_callback, chunk_size, erase);
}

void fni_ob_clean() {
  return f_ob_clean();
}

void fni_ob_flush() {
  return f_ob_flush();
}

bool fni_ob_end_clean() {
  return f_ob_end_clean();
}

bool fni_ob_end_flush() {
  return f_ob_end_flush();
}

void fni_flush() {
  return f_flush();
}

String fni_ob_get_clean() {
  return f_ob_get_clean();
}

String fni_ob_get_contents() {
  return f_ob_get_contents();
}

String fni_ob_get_flush() {
  return f_ob_get_flush();
}

int64 fni_ob_get_length() {
  return f_ob_get_length();
}

int64 fni_ob_get_level() {
  return f_ob_get_level();
}

Array fni_ob_get_status(bool full_status) {
  return f_ob_get_status(full_status);
}

String fni_ob_gzhandler(String const& buffer, int mode) {
  return f_ob_gzhandler(buffer, mode);
}

void fni_ob_implicit_flush(bool flag) {
  return f_ob_implicit_flush(flag);
}

Array fni_ob_list_handlers() {
  return f_ob_list_handlers();
}

bool fni_output_add_rewrite_var(String const& name, String const& value) {
  return f_output_add_rewrite_var(name, value);
}

bool fni_output_reset_rewrite_vars() {
  return f_output_reset_rewrite_vars();
}

void fni_hphp_stats(String const& name, long long value) {
  return f_hphp_stats(name, value);
}

long long fni_hphp_get_stats(String const& name) {
  return f_hphp_get_stats(name);
}

Array fni_hphp_get_iostatus() {
  return f_hphp_get_iostatus();
}

void fni_hphp_set_iostatus_address(String const& name) {
  return f_hphp_set_iostatus_address(name);
}

String fni_posix_ctermid() {
  return f_posix_ctermid();
}

int64 fni_posix_get_last_error() {
  return f_posix_get_last_error();
}

String fni_posix_getcwd() {
  return f_posix_getcwd();
}

int64 fni_posix_getegid() {
  return f_posix_getegid();
}

int64 fni_posix_geteuid() {
  return f_posix_geteuid();
}

int64 fni_posix_getgid() {
  return f_posix_getgid();
}

Variant fni_posix_getlogin() {
  return f_posix_getlogin();
}

Variant fni_posix_getpgid(int pid) {
  return f_posix_getpgid(pid);
}

int64 fni_posix_getpgrp() {
  return f_posix_getpgrp();
}

int64 fni_posix_getpid() {
  return f_posix_getpid();
}

int64 fni_posix_getppid() {
  return f_posix_getppid();
}

Variant fni_posix_getsid(int pid) {
  return f_posix_getsid(pid);
}

int64 fni_posix_getuid() {
  return f_posix_getuid();
}

bool fni_posix_initgroups(String const& name, int base_group_id) {
  return f_posix_initgroups(name, base_group_id);
}

bool fni_posix_kill(int pid, int sig) {
  return f_posix_kill(pid, sig);
}

bool fni_posix_mkfifo(String const& pathname, int mode) {
  return f_posix_mkfifo(pathname, mode);
}

bool fni_posix_setegid(int gid) {
  return f_posix_setegid(gid);
}

bool fni_posix_seteuid(int uid) {
  return f_posix_seteuid(uid);
}

bool fni_posix_setgid(int gid) {
  return f_posix_setgid(gid);
}

bool fni_posix_setpgid(int pid, int pgid) {
  return f_posix_setpgid(pid, pgid);
}

int64 fni_posix_setsid() {
  return f_posix_setsid();
}

bool fni_posix_setuid(int uid) {
  return f_posix_setuid(uid);
}

String fni_posix_strerror(int errnum) {
  return f_posix_strerror(errnum);
}

int64 fni_pcntl_alarm(int seconds) {
  return f_pcntl_alarm(seconds);
}

int64 fni_pcntl_wexitstatus(int status) {
  return f_pcntl_wexitstatus(status);
}

bool fni_pcntl_wifexited(int status) {
  return f_pcntl_wifexited(status);
}

bool fni_pcntl_wifsignaled(int status) {
  return f_pcntl_wifsignaled(status);
}

bool fni_pcntl_wifstopped(int status) {
  return f_pcntl_wifstopped(status);
}

int64 fni_pcntl_wstopsig(int status) {
  return f_pcntl_wstopsig(status);
}

int64 fni_pcntl_wtermsig(int status) {
  return f_pcntl_wtermsig(status);
}

long long fni_hphp_object_pointer(Object const& obj) {
  return f_hphp_object_pointer(obj);
}

Object fni_stream_context_create(Array const& options, Array const& params) {
  return f_stream_context_create(options, params);
}

Object fni_stream_context_get_default(Array const& options) {
  return f_stream_context_get_default(options);
}

Variant fni_stream_context_get_options(Object const& stream_or_context) {
  return f_stream_context_get_options(stream_or_context);
}

bool fni_stream_context_set_option(Object const& stream_or_context, Variant const& wrapper, String const& option, Variant const& value) {
  return f_stream_context_set_option(stream_or_context, wrapper, option, value);
}

bool fni_stream_context_set_param(Object const& stream_or_context, Array const& params) {
  return f_stream_context_set_param(stream_or_context, params);
}

bool fni_stream_encoding(Object const& stream, String const& encoding) {
  return f_stream_encoding(stream, encoding);
}

void fni_stream_bucket_append(Object const& brigade, Object const& bucket) {
  return f_stream_bucket_append(brigade, bucket);
}

void fni_stream_bucket_prepend(Object const& brigade, Object const& bucket) {
  return f_stream_bucket_prepend(brigade, bucket);
}

Object fni_stream_bucket_make_writeable(Object const& brigade) {
  return f_stream_bucket_make_writeable(brigade);
}

Object fni_stream_bucket_new(Object const& stream, String const& buffer) {
  return f_stream_bucket_new(stream, buffer);
}

bool fni_stream_filter_register(String const& filtername, String const& classname) {
  return f_stream_filter_register(filtername, classname);
}

bool fni_stream_filter_remove(Object const& stream_filter) {
  return f_stream_filter_remove(stream_filter);
}

Object fni_stream_filter_append(Object const& stream, String const& filtername, int read_write, Variant const& params) {
  return f_stream_filter_append(stream, filtername, read_write, params);
}

Object fni_stream_filter_prepend(Object const& stream, String const& filtername, int read_write, Variant const& params) {
  return f_stream_filter_prepend(stream, filtername, read_write, params);
}

Array fni_stream_get_filters() {
  return f_stream_get_filters();
}

Variant fni_stream_get_meta_data(Object const& stream) {
  return f_stream_get_meta_data(stream);
}

Array fni_stream_get_transports() {
  return f_stream_get_transports();
}

Array fni_stream_get_wrappers() {
  return f_stream_get_wrappers();
}

bool fni_stream_register_wrapper(String const& protocol, String const& classname) {
  return f_stream_register_wrapper(protocol, classname);
}

bool fni_stream_wrapper_register(String const& protocol, String const& classname) {
  return f_stream_wrapper_register(protocol, classname);
}

bool fni_stream_wrapper_restore(String const& protocol) {
  return f_stream_wrapper_restore(protocol);
}

bool fni_stream_wrapper_unregister(String const& protocol) {
  return f_stream_wrapper_unregister(protocol);
}

String fni_stream_resolve_include_path(String const& filename, Object const& context) {
  return f_stream_resolve_include_path(filename, context);
}

int64 fni_set_file_buffer(Object const& stream, int buffer) {
  return f_set_file_buffer(stream, buffer);
}

Variant fni_stream_socket_enable_crypto(Object const& stream, bool enable, int crypto_type, Object const& session_stream) {
  return f_stream_socket_enable_crypto(stream, enable, crypto_type, session_stream);
}

String fni_addcslashes(String const& str, String const& charlist) {
  return f_addcslashes(str, charlist);
}

String fni_stripcslashes(String const& str) {
  return f_stripcslashes(str);
}

String fni_addslashes(String const& str) {
  return f_addslashes(str);
}

String fni_stripslashes(String const& str) {
  return f_stripslashes(str);
}

String fni_bin2hex(String const& str) {
  return f_bin2hex(str);
}

Variant fni_hex2bin(String const& str) {
  return f_hex2bin(str);
}

String fni_nl2br(String const& str) {
  return f_nl2br(str);
}

String fni_quotemeta(String const& str) {
  return f_quotemeta(str);
}

String fni_str_shuffle(String const& str) {
  return f_str_shuffle(str);
}

String fni_strrev(String const& str) {
  return f_strrev(str);
}

String fni_strtolower(String const& str) {
  return f_strtolower(str);
}

String fni_strtoupper(String const& str) {
  return f_strtoupper(str);
}

String fni_ucfirst(String const& str) {
  return f_ucfirst(str);
}

String fni_ucwords(String const& str) {
  return f_ucwords(str);
}

String fni_strip_tags(String const& str, String const& allowable_tags) {
  return f_strip_tags(str, allowable_tags);
}

String fni_trim(String const& str, String const& charlist) {
  return f_trim(str, charlist);
}

String fni_ltrim(String const& str, String const& charlist) {
  return f_ltrim(str, charlist);
}

String fni_rtrim(String const& str, String const& charlist) {
  return f_rtrim(str, charlist);
}

String fni_chop(String const& str, String const& charlist) {
  return f_chop(str, charlist);
}

Variant fni_explode(String const& delimiter, String const& str, int limit) {
  return f_explode(delimiter, str, limit);
}

String fni_join(Variant const& glue, Variant const& pieces) {
  return f_join(glue, pieces);
}

Variant fni_str_split(String const& str, int split_length) {
  return f_str_split(str, split_length);
}

Variant fni_chunk_split(String const& body, int chunklen, String const& end) {
  return f_chunk_split(body, chunklen, end);
}

Variant fni_substr(String const& str, int start, int length) {
  return f_substr(str, start, length);
}

String fni_str_pad(String const& input, int pad_length, String const& pad_string, int pad_type) {
  return f_str_pad(input, pad_length, pad_string, pad_type);
}

String fni_str_repeat(String const& input, int multiplier) {
  return f_str_repeat(input, multiplier);
}

Variant fni_wordwrap(String const& str, int width, String const& wordbreak, bool cut) {
  return f_wordwrap(str, width, wordbreak, cut);
}

String fni_html_entity_decode(String const& str, int quote_style, String const& charset) {
  return f_html_entity_decode(str, quote_style, charset);
}

String fni_htmlentities(String const& str, int quote_style, String const& charset, bool double_encode) {
  return f_htmlentities(str, quote_style, charset, double_encode);
}

String fni_htmlspecialchars_decode(String const& str, int quote_style) {
  return f_htmlspecialchars_decode(str, quote_style);
}

String fni_htmlspecialchars(String const& str, int quote_style, String const& charset, bool double_encode) {
  return f_htmlspecialchars(str, quote_style, charset, double_encode);
}

String fni_fb_htmlspecialchars(String const& str, int quote_style, String const& charset, Array const& extra) {
  return f_fb_htmlspecialchars(str, quote_style, charset, extra);
}

String fni_quoted_printable_encode(String const& str) {
  return f_quoted_printable_encode(str);
}

String fni_quoted_printable_decode(String const& str) {
  return f_quoted_printable_decode(str);
}

Variant fni_convert_uudecode(String const& data) {
  return f_convert_uudecode(data);
}

Variant fni_convert_uuencode(String const& data) {
  return f_convert_uuencode(data);
}

String fni_str_rot13(String const& str) {
  return f_str_rot13(str);
}

long long fni_crc32(String const& str) {
  return f_crc32(str);
}

String fni_crypt(String const& str, String const& salt) {
  return f_crypt(str, salt);
}

String fni_md5(String const& str, bool raw_output) {
  return f_md5(str, raw_output);
}

String fni_sha1(String const& str, bool raw_output) {
  return f_sha1(str, raw_output);
}

Array fni_get_html_translation_table(int table, int quote_style) {
  return f_get_html_translation_table(table, quote_style);
}

String fni_nl_langinfo(int item) {
  return f_nl_langinfo(item);
}

Variant fni_printf(int _argc, String const& format, Array const& _argv) {
  return f_printf(_argc, format, _argv);
}

Variant fni_vprintf(String const& format, Array const& args) {
  return f_vprintf(format, args);
}

Variant fni_sprintf(int _argc, String const& format, Array const& _argv) {
  return f_sprintf(_argc, format, _argv);
}

Variant fni_vsprintf(String const& format, Array const& args) {
  return f_vsprintf(format, args);
}

String fni_chr(long long ascii) {
  return f_chr(ascii);
}

long long fni_ord(String const& str) {
  return f_ord(str);
}

Variant fni_money_format(String const& format, double number) {
  return f_money_format(format, number);
}

int64 fni_strcmp(String const& str1, String const& str2) {
  return f_strcmp(str1, str2);
}

int64 fni_strncmp(String const& str1, String const& str2, int len) {
  return f_strncmp(str1, str2, len);
}

int64 fni_strnatcmp(String const& str1, String const& str2) {
  return f_strnatcmp(str1, str2);
}

int64 fni_strcasecmp(String const& str1, String const& str2) {
  return f_strcasecmp(str1, str2);
}

int64 fni_strncasecmp(String const& str1, String const& str2, int len) {
  return f_strncasecmp(str1, str2, len);
}

int64 fni_strnatcasecmp(String const& str1, String const& str2) {
  return f_strnatcasecmp(str1, str2);
}

int64 fni_strcoll(String const& str1, String const& str2) {
  return f_strcoll(str1, str2);
}

Variant fni_strchr(String const& haystack, Variant const& needle) {
  return f_strchr(haystack, needle);
}

Variant fni_strlen(CVarRef str) {
  return f_strlen(str);
}

int64 fni_levenshtein(String const& str1, String const& str2, int cost_ins, int cost_rep, int cost_del) {
  return f_levenshtein(str1, str2, cost_ins, cost_rep, cost_del);
}

int64 fni_similar_text(String const& first, String const& second, VRefParamValue const& percent) {
  return f_similar_text(first, second, ref(percent));
}

Variant fni_soundex(String const& str) {
  return f_soundex(str);
}

Variant fni_metaphone(String const& str, int phones) {
  return f_metaphone(str, phones);
}

String fni_rawurldecode(String const& str) {
  return f_rawurldecode(str);
}

String fni_rawurlencode(String const& str) {
  return f_rawurlencode(str);
}

String fni_urldecode(String const& str) {
  return f_urldecode(str);
}

String fni_urlencode(String const& str) {
  return f_urlencode(str);
}

bool fni_is_bool(Variant const& var) {
  return f_is_bool(var);
}

bool fni_is_int(Variant const& var) {
  return f_is_int(var);
}

bool fni_is_integer(Variant const& var) {
  return f_is_integer(var);
}

bool fni_is_long(Variant const& var) {
  return f_is_long(var);
}

bool fni_is_double(Variant const& var) {
  return f_is_double(var);
}

bool fni_is_float(Variant const& var) {
  return f_is_float(var);
}

bool fni_is_numeric(Variant const& var) {
  return f_is_numeric(var);
}

bool fni_is_real(Variant const& var) {
  return f_is_real(var);
}

bool fni_is_string(Variant const& var) {
  return f_is_string(var);
}

bool fni_is_scalar(Variant const& var) {
  return f_is_scalar(var);
}

bool fni_is_array(Variant const& var) {
  return f_is_array(var);
}

bool fni_is_resource(Variant const& var) {
  return f_is_resource(var);
}

bool fni_is_null(Variant const& var) {
  return f_is_null(var);
}

long long fni_intval(Variant const& v, long long base) {
  return f_intval(v, base);
}

double fni_doubleval(Variant const& v) {
  return f_doubleval(v);
}

double fni_floatval(Variant const& v) {
  return f_floatval(v);
}

String fni_strval(Variant const& v) {
  return f_strval(v);
}

Variant fni_unserialize(String const& str) {
  return f_unserialize(str);
}

String fni_zlib_get_coding_type() {
  return f_zlib_get_coding_type();
}

bool fni_gzclose(Object const& zp) {
  return f_gzclose(zp);
}

bool fni_gzrewind(Object const& zp) {
  return f_gzrewind(zp);
}

bool fni_gzeof(Object const& zp) {
  return f_gzeof(zp);
}

Variant fni_gzgetc(Object const& zp) {
  return f_gzgetc(zp);
}

Variant fni_gzgets(Object const& zp, long long length) {
  return f_gzgets(zp, length);
}

Variant fni_gzgetss(Object const& zp, long long length, String const& allowable_tags) {
  return f_gzgetss(zp, length, allowable_tags);
}

Variant fni_gzread(Object const& zp, long long length) {
  return f_gzread(zp, length);
}

Variant fni_gzpassthru(Object const& zp) {
  return f_gzpassthru(zp);
}

Variant fni_gzseek(Object const& zp, long long offset, long long whence) {
  return f_gzseek(zp, offset, whence);
}

Variant fni_gztell(Object const& zp) {
  return f_gztell(zp);
}

Variant fni_gzwrite(Object const& zp, String const& str, long long length) {
  return f_gzwrite(zp, str, length);
}

Variant fni_gzputs(Object const& zp, String const& str, long long length) {
  return f_gzputs(zp, str, length);
}

}
