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
#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/ext/ext.h"
#include "ext_hhvm_infotabs.h"
namespace HPHP {
  struct TypedValue;
  struct ActRec;
  namespace VM { struct Class; }
}

namespace HPHP {
TypedValue* fg_apache_note(ActRec* ar);
TypedValue* fg_apache_request_headers(ActRec* ar);
TypedValue* fg_apache_response_headers(ActRec* ar);
TypedValue* fg_apache_setenv(ActRec* ar);
TypedValue* fg_getallheaders(ActRec* ar);
TypedValue* fg_virtual(ActRec* ar);
TypedValue* fg_apache_get_config(ActRec* ar);
TypedValue* fg_apache_get_scoreboard(ActRec* ar);
TypedValue* fg_apache_get_rewrite_rules(ActRec* ar);
TypedValue* fg_apc_add(ActRec* ar);
TypedValue* fg_apc_store(ActRec* ar);
TypedValue* fg_apc_fetch(ActRec* ar);
TypedValue* fg_apc_delete(ActRec* ar);
TypedValue* fg_apc_compile_file(ActRec* ar);
TypedValue* fg_apc_cache_info(ActRec* ar);
TypedValue* fg_apc_clear_cache(ActRec* ar);
TypedValue* fg_apc_define_constants(ActRec* ar);
TypedValue* fg_apc_load_constants(ActRec* ar);
TypedValue* fg_apc_sma_info(ActRec* ar);
TypedValue* fg_apc_filehits(ActRec* ar);
TypedValue* fg_apc_delete_file(ActRec* ar);
TypedValue* fg_apc_inc(ActRec* ar);
TypedValue* fg_apc_dec(ActRec* ar);
TypedValue* fg_apc_cas(ActRec* ar);
TypedValue* fg_apc_exists(ActRec* ar);
TypedValue* fg_apc_bin_dump(ActRec* ar);
TypedValue* fg_apc_bin_load(ActRec* ar);
TypedValue* fg_apc_bin_dumpfile(ActRec* ar);
TypedValue* fg_apc_bin_loadfile(ActRec* ar);
TypedValue* fg_override_function(ActRec* ar);
TypedValue* fg_rename_function(ActRec* ar);
TypedValue* fg_apd_set_browser_trace(ActRec* ar);
TypedValue* fg_apd_set_pprof_trace(ActRec* ar);
TypedValue* fg_apd_set_session_trace_socket(ActRec* ar);
TypedValue* fg_apd_stop_trace(ActRec* ar);
TypedValue* fg_apd_breakpoint(ActRec* ar);
TypedValue* fg_apd_continue(ActRec* ar);
TypedValue* fg_apd_echo(ActRec* ar);
TypedValue* fg_array_change_key_case(ActRec* ar);
TypedValue* fg_array_chunk(ActRec* ar);
TypedValue* fg_array_column(ActRec* ar);
TypedValue* fg_array_combine(ActRec* ar);
TypedValue* fg_array_count_values(ActRec* ar);
TypedValue* fg_array_fill_keys(ActRec* ar);
TypedValue* fg_array_fill(ActRec* ar);
TypedValue* fg_array_filter(ActRec* ar);
TypedValue* fg_array_flip(ActRec* ar);
TypedValue* fg_array_key_exists(ActRec* ar);
TypedValue* fg_key_exists(ActRec* ar);
TypedValue* fg_array_keys(ActRec* ar);
TypedValue* fg_array_map(ActRec* ar);
TypedValue* fg_array_merge_recursive(ActRec* ar);
TypedValue* fg_array_merge(ActRec* ar);
TypedValue* fg_array_replace_recursive(ActRec* ar);
TypedValue* fg_array_replace(ActRec* ar);
TypedValue* fg_array_multisort(ActRec* ar);
TypedValue* fg_array_pad(ActRec* ar);
TypedValue* fg_array_pop(ActRec* ar);
TypedValue* fg_array_product(ActRec* ar);
TypedValue* fg_array_push(ActRec* ar);
TypedValue* fg_array_rand(ActRec* ar);
TypedValue* fg_array_reduce(ActRec* ar);
TypedValue* fg_array_reverse(ActRec* ar);
TypedValue* fg_array_search(ActRec* ar);
TypedValue* fg_array_shift(ActRec* ar);
TypedValue* fg_array_slice(ActRec* ar);
TypedValue* fg_array_splice(ActRec* ar);
TypedValue* fg_array_sum(ActRec* ar);
TypedValue* fg_array_unique(ActRec* ar);
TypedValue* fg_array_unshift(ActRec* ar);
TypedValue* fg_array_values(ActRec* ar);
TypedValue* fg_array_walk_recursive(ActRec* ar);
TypedValue* fg_array_walk(ActRec* ar);
TypedValue* fg_compact(ActRec* ar);
TypedValue* fg_shuffle(ActRec* ar);
TypedValue* fg_count(ActRec* ar);
TypedValue* fg_sizeof(ActRec* ar);
TypedValue* fg_each(ActRec* ar);
TypedValue* fg_current(ActRec* ar);
TypedValue* fg_hphp_current_ref(ActRec* ar);
TypedValue* fg_next(ActRec* ar);
TypedValue* fg_pos(ActRec* ar);
TypedValue* fg_prev(ActRec* ar);
TypedValue* fg_reset(ActRec* ar);
TypedValue* fg_end(ActRec* ar);
TypedValue* fg_key(ActRec* ar);
TypedValue* fg_hphp_get_iterator(ActRec* ar);
TypedValue* fg_hphp_get_mutable_iterator(ActRec* ar);
TypedValue* fg_in_array(ActRec* ar);
TypedValue* fg_range(ActRec* ar);
TypedValue* fg_array_diff(ActRec* ar);
TypedValue* fg_array_udiff(ActRec* ar);
TypedValue* fg_array_diff_assoc(ActRec* ar);
TypedValue* fg_array_diff_uassoc(ActRec* ar);
TypedValue* fg_array_udiff_assoc(ActRec* ar);
TypedValue* fg_array_udiff_uassoc(ActRec* ar);
TypedValue* fg_array_diff_key(ActRec* ar);
TypedValue* fg_array_diff_ukey(ActRec* ar);
TypedValue* fg_array_intersect(ActRec* ar);
TypedValue* fg_array_uintersect(ActRec* ar);
TypedValue* fg_array_intersect_assoc(ActRec* ar);
TypedValue* fg_array_intersect_uassoc(ActRec* ar);
TypedValue* fg_array_uintersect_assoc(ActRec* ar);
TypedValue* fg_array_uintersect_uassoc(ActRec* ar);
TypedValue* fg_array_intersect_key(ActRec* ar);
TypedValue* fg_array_intersect_ukey(ActRec* ar);
TypedValue* fg_sort(ActRec* ar);
TypedValue* fg_rsort(ActRec* ar);
TypedValue* fg_asort(ActRec* ar);
TypedValue* fg_arsort(ActRec* ar);
TypedValue* fg_ksort(ActRec* ar);
TypedValue* fg_krsort(ActRec* ar);
TypedValue* fg_usort(ActRec* ar);
TypedValue* fg_uasort(ActRec* ar);
TypedValue* fg_uksort(ActRec* ar);
TypedValue* fg_natsort(ActRec* ar);
TypedValue* fg_natcasesort(ActRec* ar);
TypedValue* fg_i18n_loc_get_default(ActRec* ar);
TypedValue* fg_i18n_loc_set_default(ActRec* ar);
TypedValue* fg_i18n_loc_set_attribute(ActRec* ar);
TypedValue* fg_i18n_loc_set_strength(ActRec* ar);
TypedValue* fg_i18n_loc_get_error_code(ActRec* ar);
TypedValue* fg_asio_enter_context(ActRec* ar);
TypedValue* fg_asio_exit_context(ActRec* ar);
TypedValue* fg_asio_get_current_context_idx(ActRec* ar);
TypedValue* fg_asio_get_running_in_context(ActRec* ar);
TypedValue* fg_asio_get_running(ActRec* ar);
TypedValue* fg_asio_get_current(ActRec* ar);
TypedValue* fg_asio_set_on_failed_callback(ActRec* ar);
TypedValue* fg_asio_set_on_started_callback(ActRec* ar);
TypedValue* fg_bcscale(ActRec* ar);
TypedValue* fg_bcadd(ActRec* ar);
TypedValue* fg_bcsub(ActRec* ar);
TypedValue* fg_bccomp(ActRec* ar);
TypedValue* fg_bcmul(ActRec* ar);
TypedValue* fg_bcdiv(ActRec* ar);
TypedValue* fg_bcmod(ActRec* ar);
TypedValue* fg_bcpow(ActRec* ar);
TypedValue* fg_bcpowmod(ActRec* ar);
TypedValue* fg_bcsqrt(ActRec* ar);
TypedValue* fg_bzclose(ActRec* ar);
TypedValue* fg_bzopen(ActRec* ar);
TypedValue* fg_bzread(ActRec* ar);
TypedValue* fg_bzwrite(ActRec* ar);
TypedValue* fg_bzflush(ActRec* ar);
TypedValue* fg_bzerrstr(ActRec* ar);
TypedValue* fg_bzerror(ActRec* ar);
TypedValue* fg_bzerrno(ActRec* ar);
TypedValue* fg_bzcompress(ActRec* ar);
TypedValue* fg_bzdecompress(ActRec* ar);
TypedValue* fg_get_declared_classes(ActRec* ar);
TypedValue* fg_get_declared_interfaces(ActRec* ar);
TypedValue* fg_get_declared_traits(ActRec* ar);
TypedValue* fg_class_exists(ActRec* ar);
TypedValue* fg_interface_exists(ActRec* ar);
TypedValue* fg_trait_exists(ActRec* ar);
TypedValue* fg_get_class_methods(ActRec* ar);
TypedValue* fg_get_class_vars(ActRec* ar);
TypedValue* fg_get_class_constants(ActRec* ar);
TypedValue* fg_get_class(ActRec* ar);
TypedValue* fg_get_parent_class(ActRec* ar);
TypedValue* fg_is_a(ActRec* ar);
TypedValue* fg_is_subclass_of(ActRec* ar);
TypedValue* fg_method_exists(ActRec* ar);
TypedValue* fg_property_exists(ActRec* ar);
TypedValue* fg_get_object_vars(ActRec* ar);
TypedValue* fg_call_user_method_array(ActRec* ar);
TypedValue* fg_call_user_method(ActRec* ar);
TypedValue* fg_hphp_create_continuation(ActRec* ar);
TypedValue* fg_ctype_alnum(ActRec* ar);
TypedValue* fg_ctype_alpha(ActRec* ar);
TypedValue* fg_ctype_cntrl(ActRec* ar);
TypedValue* fg_ctype_digit(ActRec* ar);
TypedValue* fg_ctype_graph(ActRec* ar);
TypedValue* fg_ctype_lower(ActRec* ar);
TypedValue* fg_ctype_print(ActRec* ar);
TypedValue* fg_ctype_punct(ActRec* ar);
TypedValue* fg_ctype_space(ActRec* ar);
TypedValue* fg_ctype_upper(ActRec* ar);
TypedValue* fg_ctype_xdigit(ActRec* ar);
TypedValue* fg_curl_init(ActRec* ar);
TypedValue* fg_curl_copy_handle(ActRec* ar);
TypedValue* fg_curl_version(ActRec* ar);
TypedValue* fg_curl_setopt(ActRec* ar);
TypedValue* fg_curl_setopt_array(ActRec* ar);
TypedValue* fg_fb_curl_getopt(ActRec* ar);
TypedValue* fg_curl_exec(ActRec* ar);
TypedValue* fg_curl_getinfo(ActRec* ar);
TypedValue* fg_curl_errno(ActRec* ar);
TypedValue* fg_curl_error(ActRec* ar);
TypedValue* fg_curl_close(ActRec* ar);
TypedValue* fg_curl_multi_init(ActRec* ar);
TypedValue* fg_curl_multi_add_handle(ActRec* ar);
TypedValue* fg_curl_multi_remove_handle(ActRec* ar);
TypedValue* fg_curl_multi_exec(ActRec* ar);
TypedValue* fg_curl_multi_select(ActRec* ar);
TypedValue* fg_fb_curl_multi_fdset(ActRec* ar);
TypedValue* fg_curl_multi_getcontent(ActRec* ar);
TypedValue* fg_curl_multi_info_read(ActRec* ar);
TypedValue* fg_curl_multi_close(ActRec* ar);
TypedValue* fg_evhttp_set_cache(ActRec* ar);
TypedValue* fg_evhttp_get(ActRec* ar);
TypedValue* fg_evhttp_post(ActRec* ar);
TypedValue* fg_evhttp_async_get(ActRec* ar);
TypedValue* fg_evhttp_async_post(ActRec* ar);
TypedValue* fg_evhttp_recv(ActRec* ar);
TypedValue* fg_checkdate(ActRec* ar);
TypedValue* fg_date_add(ActRec* ar);
TypedValue* fg_date_create_from_format(ActRec* ar);
TypedValue* fg_date_create(ActRec* ar);
TypedValue* fg_date_date_set(ActRec* ar);
TypedValue* fg_date_default_timezone_get(ActRec* ar);
TypedValue* fg_date_default_timezone_set(ActRec* ar);
TypedValue* fg_date_diff(ActRec* ar);
TypedValue* fg_date_format(ActRec* ar);
TypedValue* fg_date_get_last_errors(ActRec* ar);
TypedValue* fg_date_interval_create_from_date_string(ActRec* ar);
TypedValue* fg_date_interval_format(ActRec* ar);
TypedValue* fg_date_isodate_set(ActRec* ar);
TypedValue* fg_date_modify(ActRec* ar);
TypedValue* fg_date_offset_get(ActRec* ar);
TypedValue* fg_date_parse(ActRec* ar);
TypedValue* fg_date_sub(ActRec* ar);
TypedValue* fg_date_sun_info(ActRec* ar);
TypedValue* fg_date_sunrise(ActRec* ar);
TypedValue* fg_date_sunset(ActRec* ar);
TypedValue* fg_date_time_set(ActRec* ar);
TypedValue* fg_date_timestamp_get(ActRec* ar);
TypedValue* fg_date_timestamp_set(ActRec* ar);
TypedValue* fg_date_timezone_get(ActRec* ar);
TypedValue* fg_date_timezone_set(ActRec* ar);
TypedValue* fg_date(ActRec* ar);
TypedValue* fg_getdate(ActRec* ar);
TypedValue* fg_gettimeofday(ActRec* ar);
TypedValue* fg_gmdate(ActRec* ar);
TypedValue* fg_gmmktime(ActRec* ar);
TypedValue* fg_gmstrftime(ActRec* ar);
TypedValue* fg_idate(ActRec* ar);
TypedValue* fg_localtime(ActRec* ar);
TypedValue* fg_microtime(ActRec* ar);
TypedValue* fg_mktime(ActRec* ar);
TypedValue* fg_strftime(ActRec* ar);
TypedValue* fg_strptime(ActRec* ar);
TypedValue* fg_strtotime(ActRec* ar);
TypedValue* fg_time(ActRec* ar);
TypedValue* fg_timezone_abbreviations_list(ActRec* ar);
TypedValue* fg_timezone_identifiers_list(ActRec* ar);
TypedValue* fg_timezone_location_get(ActRec* ar);
TypedValue* fg_timezone_name_from_abbr(ActRec* ar);
TypedValue* fg_timezone_name_get(ActRec* ar);
TypedValue* fg_timezone_offset_get(ActRec* ar);
TypedValue* fg_timezone_open(ActRec* ar);
TypedValue* fg_timezone_transitions_get(ActRec* ar);
TypedValue* fg_timezone_version_get(ActRec* ar);
TypedValue* fg_hphpd_install_user_command(ActRec* ar);
TypedValue* fg_hphpd_get_user_commands(ActRec* ar);
TypedValue* fg_hphpd_break(ActRec* ar);
TypedValue* fg_hphpd_get_client(ActRec* ar);
TypedValue* fg_hphpd_client_ctrl(ActRec* ar);
TypedValue* fg_dom_document_create_element(ActRec* ar);
TypedValue* fg_dom_document_create_document_fragment(ActRec* ar);
TypedValue* fg_dom_document_create_text_node(ActRec* ar);
TypedValue* fg_dom_document_create_comment(ActRec* ar);
TypedValue* fg_dom_document_create_cdatasection(ActRec* ar);
TypedValue* fg_dom_document_create_processing_instruction(ActRec* ar);
TypedValue* fg_dom_document_create_attribute(ActRec* ar);
TypedValue* fg_dom_document_create_entity_reference(ActRec* ar);
TypedValue* fg_dom_document_get_elements_by_tag_name(ActRec* ar);
TypedValue* fg_dom_document_import_node(ActRec* ar);
TypedValue* fg_dom_document_create_element_ns(ActRec* ar);
TypedValue* fg_dom_document_create_attribute_ns(ActRec* ar);
TypedValue* fg_dom_document_get_elements_by_tag_name_ns(ActRec* ar);
TypedValue* fg_dom_document_get_element_by_id(ActRec* ar);
TypedValue* fg_dom_document_normalize_document(ActRec* ar);
TypedValue* fg_dom_document_save(ActRec* ar);
TypedValue* fg_dom_document_savexml(ActRec* ar);
TypedValue* fg_dom_document_validate(ActRec* ar);
TypedValue* fg_dom_document_xinclude(ActRec* ar);
TypedValue* fg_dom_document_save_html(ActRec* ar);
TypedValue* fg_dom_document_save_html_file(ActRec* ar);
TypedValue* fg_dom_document_schema_validate_file(ActRec* ar);
TypedValue* fg_dom_document_schema_validate_xml(ActRec* ar);
TypedValue* fg_dom_document_relaxng_validate_file(ActRec* ar);
TypedValue* fg_dom_document_relaxng_validate_xml(ActRec* ar);
TypedValue* fg_dom_node_insert_before(ActRec* ar);
TypedValue* fg_dom_node_replace_child(ActRec* ar);
TypedValue* fg_dom_node_remove_child(ActRec* ar);
TypedValue* fg_dom_node_append_child(ActRec* ar);
TypedValue* fg_dom_node_has_child_nodes(ActRec* ar);
TypedValue* fg_dom_node_clone_node(ActRec* ar);
TypedValue* fg_dom_node_normalize(ActRec* ar);
TypedValue* fg_dom_node_is_supported(ActRec* ar);
TypedValue* fg_dom_node_has_attributes(ActRec* ar);
TypedValue* fg_dom_node_is_same_node(ActRec* ar);
TypedValue* fg_dom_node_lookup_prefix(ActRec* ar);
TypedValue* fg_dom_node_is_default_namespace(ActRec* ar);
TypedValue* fg_dom_node_lookup_namespace_uri(ActRec* ar);
TypedValue* fg_dom_nodelist_item(ActRec* ar);
TypedValue* fg_dom_namednodemap_get_named_item(ActRec* ar);
TypedValue* fg_dom_namednodemap_item(ActRec* ar);
TypedValue* fg_dom_namednodemap_get_named_item_ns(ActRec* ar);
TypedValue* fg_dom_characterdata_substring_data(ActRec* ar);
TypedValue* fg_dom_characterdata_append_data(ActRec* ar);
TypedValue* fg_dom_characterdata_insert_data(ActRec* ar);
TypedValue* fg_dom_characterdata_delete_data(ActRec* ar);
TypedValue* fg_dom_characterdata_replace_data(ActRec* ar);
TypedValue* fg_dom_attr_is_id(ActRec* ar);
TypedValue* fg_dom_element_get_attribute(ActRec* ar);
TypedValue* fg_dom_element_set_attribute(ActRec* ar);
TypedValue* fg_dom_element_remove_attribute(ActRec* ar);
TypedValue* fg_dom_element_get_attribute_node(ActRec* ar);
TypedValue* fg_dom_element_set_attribute_node(ActRec* ar);
TypedValue* fg_dom_element_remove_attribute_node(ActRec* ar);
TypedValue* fg_dom_element_get_elements_by_tag_name(ActRec* ar);
TypedValue* fg_dom_element_get_attribute_ns(ActRec* ar);
TypedValue* fg_dom_element_set_attribute_ns(ActRec* ar);
TypedValue* fg_dom_element_remove_attribute_ns(ActRec* ar);
TypedValue* fg_dom_element_get_attribute_node_ns(ActRec* ar);
TypedValue* fg_dom_element_set_attribute_node_ns(ActRec* ar);
TypedValue* fg_dom_element_get_elements_by_tag_name_ns(ActRec* ar);
TypedValue* fg_dom_element_has_attribute(ActRec* ar);
TypedValue* fg_dom_element_has_attribute_ns(ActRec* ar);
TypedValue* fg_dom_element_set_id_attribute(ActRec* ar);
TypedValue* fg_dom_element_set_id_attribute_ns(ActRec* ar);
TypedValue* fg_dom_element_set_id_attribute_node(ActRec* ar);
TypedValue* fg_dom_text_split_text(ActRec* ar);
TypedValue* fg_dom_text_is_whitespace_in_element_content(ActRec* ar);
TypedValue* fg_dom_xpath_register_ns(ActRec* ar);
TypedValue* fg_dom_xpath_query(ActRec* ar);
TypedValue* fg_dom_xpath_evaluate(ActRec* ar);
TypedValue* fg_dom_xpath_register_php_functions(ActRec* ar);
TypedValue* fg_debug_backtrace(ActRec* ar);
TypedValue* fg_debug_print_backtrace(ActRec* ar);
TypedValue* fg_error_get_last(ActRec* ar);
TypedValue* fg_error_log(ActRec* ar);
TypedValue* fg_error_reporting(ActRec* ar);
TypedValue* fg_restore_error_handler(ActRec* ar);
TypedValue* fg_restore_exception_handler(ActRec* ar);
TypedValue* fg_set_error_handler(ActRec* ar);
TypedValue* fg_set_exception_handler(ActRec* ar);
TypedValue* fg_hphp_set_error_page(ActRec* ar);
TypedValue* fg_hphp_throw_fatal_error(ActRec* ar);
TypedValue* fg_hphp_clear_unflushed(ActRec* ar);
TypedValue* fg_hphp_debug_caller_info(ActRec* ar);
TypedValue* fg_trigger_error(ActRec* ar);
TypedValue* fg_user_error(ActRec* ar);
TypedValue* fg_fb_thrift_serialize(ActRec* ar);
TypedValue* fg_fb_thrift_unserialize(ActRec* ar);
TypedValue* fg_fb_serialize(ActRec* ar);
TypedValue* fg_fb_unserialize(ActRec* ar);
TypedValue* fg_fb_compact_serialize(ActRec* ar);
TypedValue* fg_fb_compact_unserialize(ActRec* ar);
TypedValue* fg_fb_could_include(ActRec* ar);
TypedValue* fg_fb_intercept(ActRec* ar);
TypedValue* fg_fb_stubout_intercept_handler(ActRec* ar);
TypedValue* fg_fb_rpc_intercept_handler(ActRec* ar);
TypedValue* fg_fb_renamed_functions(ActRec* ar);
TypedValue* fg_fb_rename_function(ActRec* ar);
TypedValue* fg_fb_autoload_map(ActRec* ar);
TypedValue* fg_fb_utf8ize(ActRec* ar);
TypedValue* fg_fb_utf8_strlen_deprecated(ActRec* ar);
TypedValue* fg_fb_utf8_strlen(ActRec* ar);
TypedValue* fg_fb_utf8_substr(ActRec* ar);
TypedValue* fg_fb_call_user_func_safe(ActRec* ar);
TypedValue* fg_fb_call_user_func_safe_return(ActRec* ar);
TypedValue* fg_fb_call_user_func_array_safe(ActRec* ar);
TypedValue* fg_fb_get_code_coverage(ActRec* ar);
TypedValue* fg_fb_enable_code_coverage(ActRec* ar);
TypedValue* fg_fb_disable_code_coverage(ActRec* ar);
TypedValue* fg_xhprof_enable(ActRec* ar);
TypedValue* fg_xhprof_disable(ActRec* ar);
TypedValue* fg_xhprof_network_enable(ActRec* ar);
TypedValue* fg_xhprof_network_disable(ActRec* ar);
TypedValue* fg_xhprof_frame_begin(ActRec* ar);
TypedValue* fg_xhprof_frame_end(ActRec* ar);
TypedValue* fg_xhprof_run_trace(ActRec* ar);
TypedValue* fg_xhprof_sample_enable(ActRec* ar);
TypedValue* fg_xhprof_sample_disable(ActRec* ar);
TypedValue* fg_fb_load_local_databases(ActRec* ar);
TypedValue* fg_fb_parallel_query(ActRec* ar);
TypedValue* fg_fb_crossall_query(ActRec* ar);
TypedValue* fg_fb_const_fetch(ActRec* ar);
TypedValue* fg_fb_output_compression(ActRec* ar);
TypedValue* fg_fb_set_exit_callback(ActRec* ar);
TypedValue* fg_fb_get_flush_stat(ActRec* ar);
TypedValue* fg_fb_get_last_flush_size(ActRec* ar);
TypedValue* fg_fb_lazy_stat(ActRec* ar);
TypedValue* fg_fb_lazy_lstat(ActRec* ar);
TypedValue* fg_fb_lazy_realpath(ActRec* ar);
TypedValue* fg_fb_setprofile(ActRec* ar);
TypedValue* fg_fb_gc_collect_cycles(ActRec* ar);
TypedValue* fg_fb_gc_detect_cycles(ActRec* ar);
TypedValue* fg_fopen(ActRec* ar);
TypedValue* fg_popen(ActRec* ar);
TypedValue* fg_fclose(ActRec* ar);
TypedValue* fg_pclose(ActRec* ar);
TypedValue* fg_fseek(ActRec* ar);
TypedValue* fg_rewind(ActRec* ar);
TypedValue* fg_ftell(ActRec* ar);
TypedValue* fg_feof(ActRec* ar);
TypedValue* fg_fstat(ActRec* ar);
TypedValue* fg_fread(ActRec* ar);
TypedValue* fg_fgetc(ActRec* ar);
TypedValue* fg_fgets(ActRec* ar);
TypedValue* fg_fgetss(ActRec* ar);
TypedValue* fg_fscanf(ActRec* ar);
TypedValue* fg_fpassthru(ActRec* ar);
TypedValue* fg_fwrite(ActRec* ar);
TypedValue* fg_fputs(ActRec* ar);
TypedValue* fg_fprintf(ActRec* ar);
TypedValue* fg_vfprintf(ActRec* ar);
TypedValue* fg_fflush(ActRec* ar);
TypedValue* fg_ftruncate(ActRec* ar);
TypedValue* fg_flock(ActRec* ar);
TypedValue* fg_fputcsv(ActRec* ar);
TypedValue* fg_fgetcsv(ActRec* ar);
TypedValue* fg_file_get_contents(ActRec* ar);
TypedValue* fg_file_put_contents(ActRec* ar);
TypedValue* fg_file(ActRec* ar);
TypedValue* fg_readfile(ActRec* ar);
TypedValue* fg_move_uploaded_file(ActRec* ar);
TypedValue* fg_parse_ini_file(ActRec* ar);
TypedValue* fg_parse_ini_string(ActRec* ar);
TypedValue* fg_parse_hdf_file(ActRec* ar);
TypedValue* fg_parse_hdf_string(ActRec* ar);
TypedValue* fg_write_hdf_file(ActRec* ar);
TypedValue* fg_write_hdf_string(ActRec* ar);
TypedValue* fg_md5_file(ActRec* ar);
TypedValue* fg_sha1_file(ActRec* ar);
TypedValue* fg_chmod(ActRec* ar);
TypedValue* fg_chown(ActRec* ar);
TypedValue* fg_lchown(ActRec* ar);
TypedValue* fg_chgrp(ActRec* ar);
TypedValue* fg_lchgrp(ActRec* ar);
TypedValue* fg_touch(ActRec* ar);
TypedValue* fg_copy(ActRec* ar);
TypedValue* fg_rename(ActRec* ar);
TypedValue* fg_umask(ActRec* ar);
TypedValue* fg_unlink(ActRec* ar);
TypedValue* fg_link(ActRec* ar);
TypedValue* fg_symlink(ActRec* ar);
TypedValue* fg_basename(ActRec* ar);
TypedValue* fg_fnmatch(ActRec* ar);
TypedValue* fg_glob(ActRec* ar);
TypedValue* fg_tempnam(ActRec* ar);
TypedValue* fg_tmpfile(ActRec* ar);
TypedValue* fg_fileperms(ActRec* ar);
TypedValue* fg_fileinode(ActRec* ar);
TypedValue* fg_filesize(ActRec* ar);
TypedValue* fg_fileowner(ActRec* ar);
TypedValue* fg_filegroup(ActRec* ar);
TypedValue* fg_fileatime(ActRec* ar);
TypedValue* fg_filemtime(ActRec* ar);
TypedValue* fg_filectime(ActRec* ar);
TypedValue* fg_filetype(ActRec* ar);
TypedValue* fg_linkinfo(ActRec* ar);
TypedValue* fg_is_writable(ActRec* ar);
TypedValue* fg_is_writeable(ActRec* ar);
TypedValue* fg_is_readable(ActRec* ar);
TypedValue* fg_is_executable(ActRec* ar);
TypedValue* fg_is_file(ActRec* ar);
TypedValue* fg_is_dir(ActRec* ar);
TypedValue* fg_is_link(ActRec* ar);
TypedValue* fg_is_uploaded_file(ActRec* ar);
TypedValue* fg_file_exists(ActRec* ar);
TypedValue* fg_stat(ActRec* ar);
TypedValue* fg_lstat(ActRec* ar);
TypedValue* fg_clearstatcache(ActRec* ar);
TypedValue* fg_readlink(ActRec* ar);
TypedValue* fg_realpath(ActRec* ar);
TypedValue* fg_pathinfo(ActRec* ar);
TypedValue* fg_disk_free_space(ActRec* ar);
TypedValue* fg_diskfreespace(ActRec* ar);
TypedValue* fg_disk_total_space(ActRec* ar);
TypedValue* fg_mkdir(ActRec* ar);
TypedValue* fg_rmdir(ActRec* ar);
TypedValue* fg_dirname(ActRec* ar);
TypedValue* fg_getcwd(ActRec* ar);
TypedValue* fg_chdir(ActRec* ar);
TypedValue* fg_chroot(ActRec* ar);
TypedValue* fg_dir(ActRec* ar);
TypedValue* fg_opendir(ActRec* ar);
TypedValue* fg_readdir(ActRec* ar);
TypedValue* fg_rewinddir(ActRec* ar);
TypedValue* fg_scandir(ActRec* ar);
TypedValue* fg_closedir(ActRec* ar);
TypedValue* fg_get_defined_functions(ActRec* ar);
TypedValue* fg_function_exists(ActRec* ar);
TypedValue* fg_is_callable(ActRec* ar);
TypedValue* fg_call_user_func_array(ActRec* ar);
TypedValue* fg_call_user_func(ActRec* ar);
TypedValue* fg_call_user_func_array_async(ActRec* ar);
TypedValue* fg_call_user_func_async(ActRec* ar);
TypedValue* fg_check_user_func_async(ActRec* ar);
TypedValue* fg_end_user_func_async(ActRec* ar);
TypedValue* fg_call_user_func_serialized(ActRec* ar);
TypedValue* fg_call_user_func_array_rpc(ActRec* ar);
TypedValue* fg_call_user_func_rpc(ActRec* ar);
TypedValue* fg_forward_static_call_array(ActRec* ar);
TypedValue* fg_forward_static_call(ActRec* ar);
TypedValue* fg_get_called_class(ActRec* ar);
TypedValue* fg_create_function(ActRec* ar);
TypedValue* fg_func_get_arg(ActRec* ar);
TypedValue* fg_func_get_args(ActRec* ar);
TypedValue* fg_func_num_args(ActRec* ar);
TypedValue* fg_register_postsend_function(ActRec* ar);
TypedValue* fg_register_shutdown_function(ActRec* ar);
TypedValue* fg_register_cleanup_function(ActRec* ar);
TypedValue* fg_register_tick_function(ActRec* ar);
TypedValue* fg_unregister_tick_function(ActRec* ar);
TypedValue* fg_hash(ActRec* ar);
TypedValue* fg_hash_algos(ActRec* ar);
TypedValue* fg_hash_init(ActRec* ar);
TypedValue* fg_hash_file(ActRec* ar);
TypedValue* fg_hash_final(ActRec* ar);
TypedValue* fg_hash_hmac_file(ActRec* ar);
TypedValue* fg_hash_hmac(ActRec* ar);
TypedValue* fg_hash_update_file(ActRec* ar);
TypedValue* fg_hash_update_stream(ActRec* ar);
TypedValue* fg_hash_update(ActRec* ar);
TypedValue* fg_furchash_hphp_ext(ActRec* ar);
TypedValue* fg_furchash_hphp_ext_supported(ActRec* ar);
TypedValue* fg_hphp_murmurhash(ActRec* ar);
TypedValue* fg_iconv_mime_encode(ActRec* ar);
TypedValue* fg_iconv_mime_decode(ActRec* ar);
TypedValue* fg_iconv_mime_decode_headers(ActRec* ar);
TypedValue* fg_iconv_get_encoding(ActRec* ar);
TypedValue* fg_iconv_set_encoding(ActRec* ar);
TypedValue* fg_iconv(ActRec* ar);
TypedValue* fg_iconv_strlen(ActRec* ar);
TypedValue* fg_iconv_strpos(ActRec* ar);
TypedValue* fg_iconv_strrpos(ActRec* ar);
TypedValue* fg_iconv_substr(ActRec* ar);
TypedValue* fg_ob_iconv_handler(ActRec* ar);
TypedValue* fg_icu_match(ActRec* ar);
TypedValue* fg_icu_transliterate(ActRec* ar);
TypedValue* fg_icu_tokenize(ActRec* ar);
TypedValue* fg_gd_info(ActRec* ar);
TypedValue* fg_getimagesize(ActRec* ar);
TypedValue* fg_image_type_to_extension(ActRec* ar);
TypedValue* fg_image_type_to_mime_type(ActRec* ar);
TypedValue* fg_image2wbmp(ActRec* ar);
TypedValue* fg_imagealphablending(ActRec* ar);
TypedValue* fg_imageantialias(ActRec* ar);
TypedValue* fg_imagearc(ActRec* ar);
TypedValue* fg_imagechar(ActRec* ar);
TypedValue* fg_imagecharup(ActRec* ar);
TypedValue* fg_imagecolorallocate(ActRec* ar);
TypedValue* fg_imagecolorallocatealpha(ActRec* ar);
TypedValue* fg_imagecolorat(ActRec* ar);
TypedValue* fg_imagecolorclosest(ActRec* ar);
TypedValue* fg_imagecolorclosestalpha(ActRec* ar);
TypedValue* fg_imagecolorclosesthwb(ActRec* ar);
TypedValue* fg_imagecolordeallocate(ActRec* ar);
TypedValue* fg_imagecolorexact(ActRec* ar);
TypedValue* fg_imagecolorexactalpha(ActRec* ar);
TypedValue* fg_imagecolormatch(ActRec* ar);
TypedValue* fg_imagecolorresolve(ActRec* ar);
TypedValue* fg_imagecolorresolvealpha(ActRec* ar);
TypedValue* fg_imagecolorset(ActRec* ar);
TypedValue* fg_imagecolorsforindex(ActRec* ar);
TypedValue* fg_imagecolorstotal(ActRec* ar);
TypedValue* fg_imagecolortransparent(ActRec* ar);
TypedValue* fg_imageconvolution(ActRec* ar);
TypedValue* fg_imagecopy(ActRec* ar);
TypedValue* fg_imagecopymerge(ActRec* ar);
TypedValue* fg_imagecopymergegray(ActRec* ar);
TypedValue* fg_imagecopyresampled(ActRec* ar);
TypedValue* fg_imagecopyresized(ActRec* ar);
TypedValue* fg_imagecreate(ActRec* ar);
TypedValue* fg_imagecreatefromgd2part(ActRec* ar);
TypedValue* fg_imagecreatefromgd(ActRec* ar);
TypedValue* fg_imagecreatefromgd2(ActRec* ar);
TypedValue* fg_imagecreatefromgif(ActRec* ar);
TypedValue* fg_imagecreatefromjpeg(ActRec* ar);
TypedValue* fg_imagecreatefrompng(ActRec* ar);
TypedValue* fg_imagecreatefromstring(ActRec* ar);
TypedValue* fg_imagecreatefromwbmp(ActRec* ar);
TypedValue* fg_imagecreatefromxbm(ActRec* ar);
TypedValue* fg_imagecreatefromxpm(ActRec* ar);
TypedValue* fg_imagecreatetruecolor(ActRec* ar);
TypedValue* fg_imagedashedline(ActRec* ar);
TypedValue* fg_imagedestroy(ActRec* ar);
TypedValue* fg_imageellipse(ActRec* ar);
TypedValue* fg_imagefill(ActRec* ar);
TypedValue* fg_imagefilledarc(ActRec* ar);
TypedValue* fg_imagefilledellipse(ActRec* ar);
TypedValue* fg_imagefilledpolygon(ActRec* ar);
TypedValue* fg_imagefilledrectangle(ActRec* ar);
TypedValue* fg_imagefilltoborder(ActRec* ar);
TypedValue* fg_imagefilter(ActRec* ar);
TypedValue* fg_imagefontheight(ActRec* ar);
TypedValue* fg_imagefontwidth(ActRec* ar);
TypedValue* fg_imageftbbox(ActRec* ar);
TypedValue* fg_imagefttext(ActRec* ar);
TypedValue* fg_imagegammacorrect(ActRec* ar);
TypedValue* fg_imagegd2(ActRec* ar);
TypedValue* fg_imagegd(ActRec* ar);
TypedValue* fg_imagegif(ActRec* ar);
TypedValue* fg_imagegrabscreen(ActRec* ar);
TypedValue* fg_imagegrabwindow(ActRec* ar);
TypedValue* fg_imageinterlace(ActRec* ar);
TypedValue* fg_imageistruecolor(ActRec* ar);
TypedValue* fg_imagejpeg(ActRec* ar);
TypedValue* fg_imagelayereffect(ActRec* ar);
TypedValue* fg_imageline(ActRec* ar);
TypedValue* fg_imageloadfont(ActRec* ar);
TypedValue* fg_imagepalettecopy(ActRec* ar);
TypedValue* fg_imagepng(ActRec* ar);
TypedValue* fg_imagepolygon(ActRec* ar);
TypedValue* fg_imagepsbbox(ActRec* ar);
TypedValue* fg_imagepsencodefont(ActRec* ar);
TypedValue* fg_imagepsextendfont(ActRec* ar);
TypedValue* fg_imagepsfreefont(ActRec* ar);
TypedValue* fg_imagepsloadfont(ActRec* ar);
TypedValue* fg_imagepsslantfont(ActRec* ar);
TypedValue* fg_imagepstext(ActRec* ar);
TypedValue* fg_imagerectangle(ActRec* ar);
TypedValue* fg_imagerotate(ActRec* ar);
TypedValue* fg_imagesavealpha(ActRec* ar);
TypedValue* fg_imagesetbrush(ActRec* ar);
TypedValue* fg_imagesetpixel(ActRec* ar);
TypedValue* fg_imagesetstyle(ActRec* ar);
TypedValue* fg_imagesetthickness(ActRec* ar);
TypedValue* fg_imagesettile(ActRec* ar);
TypedValue* fg_imagestring(ActRec* ar);
TypedValue* fg_imagestringup(ActRec* ar);
TypedValue* fg_imagesx(ActRec* ar);
TypedValue* fg_imagesy(ActRec* ar);
TypedValue* fg_imagetruecolortopalette(ActRec* ar);
TypedValue* fg_imagettfbbox(ActRec* ar);
TypedValue* fg_imagettftext(ActRec* ar);
TypedValue* fg_imagetypes(ActRec* ar);
TypedValue* fg_imagewbmp(ActRec* ar);
TypedValue* fg_imagexbm(ActRec* ar);
TypedValue* fg_iptcembed(ActRec* ar);
TypedValue* fg_iptcparse(ActRec* ar);
TypedValue* fg_jpeg2wbmp(ActRec* ar);
TypedValue* fg_png2wbmp(ActRec* ar);
TypedValue* fg_exif_imagetype(ActRec* ar);
TypedValue* fg_exif_read_data(ActRec* ar);
TypedValue* fg_read_exif_data(ActRec* ar);
TypedValue* fg_exif_tagname(ActRec* ar);
TypedValue* fg_exif_thumbnail(ActRec* ar);
TypedValue* fg_imap_8bit(ActRec* ar);
TypedValue* fg_imap_alerts(ActRec* ar);
TypedValue* fg_imap_append(ActRec* ar);
TypedValue* fg_imap_base64(ActRec* ar);
TypedValue* fg_imap_binary(ActRec* ar);
TypedValue* fg_imap_body(ActRec* ar);
TypedValue* fg_imap_bodystruct(ActRec* ar);
TypedValue* fg_imap_check(ActRec* ar);
TypedValue* fg_imap_clearflag_full(ActRec* ar);
TypedValue* fg_imap_close(ActRec* ar);
TypedValue* fg_imap_createmailbox(ActRec* ar);
TypedValue* fg_imap_delete(ActRec* ar);
TypedValue* fg_imap_deletemailbox(ActRec* ar);
TypedValue* fg_imap_errors(ActRec* ar);
TypedValue* fg_imap_expunge(ActRec* ar);
TypedValue* fg_imap_fetch_overview(ActRec* ar);
TypedValue* fg_imap_fetchbody(ActRec* ar);
TypedValue* fg_imap_fetchheader(ActRec* ar);
TypedValue* fg_imap_fetchstructure(ActRec* ar);
TypedValue* fg_imap_gc(ActRec* ar);
TypedValue* fg_imap_get_quota(ActRec* ar);
TypedValue* fg_imap_get_quotaroot(ActRec* ar);
TypedValue* fg_imap_getacl(ActRec* ar);
TypedValue* fg_imap_getmailboxes(ActRec* ar);
TypedValue* fg_imap_getsubscribed(ActRec* ar);
TypedValue* fg_imap_header(ActRec* ar);
TypedValue* fg_imap_headerinfo(ActRec* ar);
TypedValue* fg_imap_headers(ActRec* ar);
TypedValue* fg_imap_last_error(ActRec* ar);
TypedValue* fg_imap_list(ActRec* ar);
TypedValue* fg_imap_listmailbox(ActRec* ar);
TypedValue* fg_imap_listscan(ActRec* ar);
TypedValue* fg_imap_listsubscribed(ActRec* ar);
TypedValue* fg_imap_lsub(ActRec* ar);
TypedValue* fg_imap_mail_compose(ActRec* ar);
TypedValue* fg_imap_mail_copy(ActRec* ar);
TypedValue* fg_imap_mail_move(ActRec* ar);
TypedValue* fg_imap_mail(ActRec* ar);
TypedValue* fg_imap_mailboxmsginfo(ActRec* ar);
TypedValue* fg_imap_mime_header_decode(ActRec* ar);
TypedValue* fg_imap_msgno(ActRec* ar);
TypedValue* fg_imap_num_msg(ActRec* ar);
TypedValue* fg_imap_num_recent(ActRec* ar);
TypedValue* fg_imap_open(ActRec* ar);
TypedValue* fg_imap_ping(ActRec* ar);
TypedValue* fg_imap_qprint(ActRec* ar);
TypedValue* fg_imap_renamemailbox(ActRec* ar);
TypedValue* fg_imap_reopen(ActRec* ar);
TypedValue* fg_imap_rfc822_parse_adrlist(ActRec* ar);
TypedValue* fg_imap_rfc822_parse_headers(ActRec* ar);
TypedValue* fg_imap_rfc822_write_address(ActRec* ar);
TypedValue* fg_imap_savebody(ActRec* ar);
TypedValue* fg_imap_scanmailbox(ActRec* ar);
TypedValue* fg_imap_search(ActRec* ar);
TypedValue* fg_imap_set_quota(ActRec* ar);
TypedValue* fg_imap_setacl(ActRec* ar);
TypedValue* fg_imap_setflag_full(ActRec* ar);
TypedValue* fg_imap_sort(ActRec* ar);
TypedValue* fg_imap_status(ActRec* ar);
TypedValue* fg_imap_subscribe(ActRec* ar);
TypedValue* fg_imap_thread(ActRec* ar);
TypedValue* fg_imap_timeout(ActRec* ar);
TypedValue* fg_imap_uid(ActRec* ar);
TypedValue* fg_imap_undelete(ActRec* ar);
TypedValue* fg_imap_unsubscribe(ActRec* ar);
TypedValue* fg_imap_utf7_decode(ActRec* ar);
TypedValue* fg_imap_utf7_encode(ActRec* ar);
TypedValue* fg_imap_utf8(ActRec* ar);
TypedValue* fg_intl_get_error_code(ActRec* ar);
TypedValue* fg_intl_get_error_message(ActRec* ar);
TypedValue* fg_intl_error_name(ActRec* ar);
TypedValue* fg_intl_is_failure(ActRec* ar);
TypedValue* fg_collator_asort(ActRec* ar);
TypedValue* fg_collator_compare(ActRec* ar);
TypedValue* fg_collator_create(ActRec* ar);
TypedValue* fg_collator_get_attribute(ActRec* ar);
TypedValue* fg_collator_get_error_code(ActRec* ar);
TypedValue* fg_collator_get_error_message(ActRec* ar);
TypedValue* fg_collator_get_locale(ActRec* ar);
TypedValue* fg_collator_get_strength(ActRec* ar);
TypedValue* fg_collator_set_attribute(ActRec* ar);
TypedValue* fg_collator_set_strength(ActRec* ar);
TypedValue* fg_collator_sort_with_sort_keys(ActRec* ar);
TypedValue* fg_collator_sort(ActRec* ar);
TypedValue* fg_idn_to_ascii(ActRec* ar);
TypedValue* fg_idn_to_unicode(ActRec* ar);
TypedValue* fg_idn_to_utf8(ActRec* ar);
TypedValue* fg_ftok(ActRec* ar);
TypedValue* fg_msg_get_queue(ActRec* ar);
TypedValue* fg_msg_queue_exists(ActRec* ar);
TypedValue* fg_msg_send(ActRec* ar);
TypedValue* fg_msg_receive(ActRec* ar);
TypedValue* fg_msg_remove_queue(ActRec* ar);
TypedValue* fg_msg_set_queue(ActRec* ar);
TypedValue* fg_msg_stat_queue(ActRec* ar);
TypedValue* fg_sem_acquire(ActRec* ar);
TypedValue* fg_sem_get(ActRec* ar);
TypedValue* fg_sem_release(ActRec* ar);
TypedValue* fg_sem_remove(ActRec* ar);
TypedValue* fg_shm_attach(ActRec* ar);
TypedValue* fg_shm_detach(ActRec* ar);
TypedValue* fg_shm_remove(ActRec* ar);
TypedValue* fg_shm_get_var(ActRec* ar);
TypedValue* fg_shm_has_var(ActRec* ar);
TypedValue* fg_shm_put_var(ActRec* ar);
TypedValue* fg_shm_remove_var(ActRec* ar);
TypedValue* fg_hphp_recursiveiteratoriterator___construct(ActRec* ar);
TypedValue* fg_hphp_recursiveiteratoriterator_getinneriterator(ActRec* ar);
TypedValue* fg_hphp_recursiveiteratoriterator_current(ActRec* ar);
TypedValue* fg_hphp_recursiveiteratoriterator_key(ActRec* ar);
TypedValue* fg_hphp_recursiveiteratoriterator_next(ActRec* ar);
TypedValue* fg_hphp_recursiveiteratoriterator_rewind(ActRec* ar);
TypedValue* fg_hphp_recursiveiteratoriterator_valid(ActRec* ar);
TypedValue* fg_hphp_directoryiterator___construct(ActRec* ar);
TypedValue* fg_hphp_directoryiterator_key(ActRec* ar);
TypedValue* fg_hphp_directoryiterator_next(ActRec* ar);
TypedValue* fg_hphp_directoryiterator_rewind(ActRec* ar);
TypedValue* fg_hphp_directoryiterator_seek(ActRec* ar);
TypedValue* fg_hphp_directoryiterator_current(ActRec* ar);
TypedValue* fg_hphp_directoryiterator___tostring(ActRec* ar);
TypedValue* fg_hphp_directoryiterator_valid(ActRec* ar);
TypedValue* fg_hphp_directoryiterator_isdot(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator___construct(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_key(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_next(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_rewind(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_seek(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_current(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator___tostring(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_valid(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_haschildren(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_getchildren(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_getsubpath(ActRec* ar);
TypedValue* fg_hphp_recursivedirectoryiterator_getsubpathname(ActRec* ar);
TypedValue* fg_json_encode(ActRec* ar);
TypedValue* fg_json_decode(ActRec* ar);
TypedValue* fg_ldap_connect(ActRec* ar);
TypedValue* fg_ldap_explode_dn(ActRec* ar);
TypedValue* fg_ldap_dn2ufn(ActRec* ar);
TypedValue* fg_ldap_err2str(ActRec* ar);
TypedValue* fg_ldap_add(ActRec* ar);
TypedValue* fg_ldap_mod_add(ActRec* ar);
TypedValue* fg_ldap_mod_del(ActRec* ar);
TypedValue* fg_ldap_mod_replace(ActRec* ar);
TypedValue* fg_ldap_modify(ActRec* ar);
TypedValue* fg_ldap_bind(ActRec* ar);
TypedValue* fg_ldap_set_rebind_proc(ActRec* ar);
TypedValue* fg_ldap_sort(ActRec* ar);
TypedValue* fg_ldap_start_tls(ActRec* ar);
TypedValue* fg_ldap_unbind(ActRec* ar);
TypedValue* fg_ldap_get_option(ActRec* ar);
TypedValue* fg_ldap_set_option(ActRec* ar);
TypedValue* fg_ldap_close(ActRec* ar);
TypedValue* fg_ldap_list(ActRec* ar);
TypedValue* fg_ldap_read(ActRec* ar);
TypedValue* fg_ldap_search(ActRec* ar);
TypedValue* fg_ldap_rename(ActRec* ar);
TypedValue* fg_ldap_delete(ActRec* ar);
TypedValue* fg_ldap_compare(ActRec* ar);
TypedValue* fg_ldap_errno(ActRec* ar);
TypedValue* fg_ldap_error(ActRec* ar);
TypedValue* fg_ldap_get_dn(ActRec* ar);
TypedValue* fg_ldap_count_entries(ActRec* ar);
TypedValue* fg_ldap_get_entries(ActRec* ar);
TypedValue* fg_ldap_first_entry(ActRec* ar);
TypedValue* fg_ldap_next_entry(ActRec* ar);
TypedValue* fg_ldap_get_attributes(ActRec* ar);
TypedValue* fg_ldap_first_attribute(ActRec* ar);
TypedValue* fg_ldap_next_attribute(ActRec* ar);
TypedValue* fg_ldap_first_reference(ActRec* ar);
TypedValue* fg_ldap_next_reference(ActRec* ar);
TypedValue* fg_ldap_parse_reference(ActRec* ar);
TypedValue* fg_ldap_parse_result(ActRec* ar);
TypedValue* fg_ldap_free_result(ActRec* ar);
TypedValue* fg_ldap_get_values_len(ActRec* ar);
TypedValue* fg_ldap_get_values(ActRec* ar);
TypedValue* fg_magickgetcopyright(ActRec* ar);
TypedValue* fg_magickgethomeurl(ActRec* ar);
TypedValue* fg_magickgetpackagename(ActRec* ar);
TypedValue* fg_magickgetquantumdepth(ActRec* ar);
TypedValue* fg_magickgetreleasedate(ActRec* ar);
TypedValue* fg_magickgetresourcelimit(ActRec* ar);
TypedValue* fg_magickgetversion(ActRec* ar);
TypedValue* fg_magickgetversionnumber(ActRec* ar);
TypedValue* fg_magickgetversionstring(ActRec* ar);
TypedValue* fg_magickqueryconfigureoption(ActRec* ar);
TypedValue* fg_magickqueryconfigureoptions(ActRec* ar);
TypedValue* fg_magickqueryfonts(ActRec* ar);
TypedValue* fg_magickqueryformats(ActRec* ar);
TypedValue* fg_magicksetresourcelimit(ActRec* ar);
TypedValue* fg_newdrawingwand(ActRec* ar);
TypedValue* fg_newmagickwand(ActRec* ar);
TypedValue* fg_newpixeliterator(ActRec* ar);
TypedValue* fg_newpixelregioniterator(ActRec* ar);
TypedValue* fg_newpixelwand(ActRec* ar);
TypedValue* fg_newpixelwandarray(ActRec* ar);
TypedValue* fg_newpixelwands(ActRec* ar);
TypedValue* fg_destroydrawingwand(ActRec* ar);
TypedValue* fg_destroymagickwand(ActRec* ar);
TypedValue* fg_destroypixeliterator(ActRec* ar);
TypedValue* fg_destroypixelwand(ActRec* ar);
TypedValue* fg_destroypixelwandarray(ActRec* ar);
TypedValue* fg_destroypixelwands(ActRec* ar);
TypedValue* fg_isdrawingwand(ActRec* ar);
TypedValue* fg_ismagickwand(ActRec* ar);
TypedValue* fg_ispixeliterator(ActRec* ar);
TypedValue* fg_ispixelwand(ActRec* ar);
TypedValue* fg_cleardrawingwand(ActRec* ar);
TypedValue* fg_clearmagickwand(ActRec* ar);
TypedValue* fg_clearpixeliterator(ActRec* ar);
TypedValue* fg_clearpixelwand(ActRec* ar);
TypedValue* fg_clonedrawingwand(ActRec* ar);
TypedValue* fg_clonemagickwand(ActRec* ar);
TypedValue* fg_wandgetexception(ActRec* ar);
TypedValue* fg_wandgetexceptionstring(ActRec* ar);
TypedValue* fg_wandgetexceptiontype(ActRec* ar);
TypedValue* fg_wandhasexception(ActRec* ar);
TypedValue* fg_drawaffine(ActRec* ar);
TypedValue* fg_drawannotation(ActRec* ar);
TypedValue* fg_drawarc(ActRec* ar);
TypedValue* fg_drawbezier(ActRec* ar);
TypedValue* fg_drawcircle(ActRec* ar);
TypedValue* fg_drawcolor(ActRec* ar);
TypedValue* fg_drawcomment(ActRec* ar);
TypedValue* fg_drawcomposite(ActRec* ar);
TypedValue* fg_drawellipse(ActRec* ar);
TypedValue* fg_drawgetclippath(ActRec* ar);
TypedValue* fg_drawgetcliprule(ActRec* ar);
TypedValue* fg_drawgetclipunits(ActRec* ar);
TypedValue* fg_drawgetexception(ActRec* ar);
TypedValue* fg_drawgetexceptionstring(ActRec* ar);
TypedValue* fg_drawgetexceptiontype(ActRec* ar);
TypedValue* fg_drawgetfillalpha(ActRec* ar);
TypedValue* fg_drawgetfillcolor(ActRec* ar);
TypedValue* fg_drawgetfillopacity(ActRec* ar);
TypedValue* fg_drawgetfillrule(ActRec* ar);
TypedValue* fg_drawgetfont(ActRec* ar);
TypedValue* fg_drawgetfontfamily(ActRec* ar);
TypedValue* fg_drawgetfontsize(ActRec* ar);
TypedValue* fg_drawgetfontstretch(ActRec* ar);
TypedValue* fg_drawgetfontstyle(ActRec* ar);
TypedValue* fg_drawgetfontweight(ActRec* ar);
TypedValue* fg_drawgetgravity(ActRec* ar);
TypedValue* fg_drawgetstrokealpha(ActRec* ar);
TypedValue* fg_drawgetstrokeantialias(ActRec* ar);
TypedValue* fg_drawgetstrokecolor(ActRec* ar);
TypedValue* fg_drawgetstrokedasharray(ActRec* ar);
TypedValue* fg_drawgetstrokedashoffset(ActRec* ar);
TypedValue* fg_drawgetstrokelinecap(ActRec* ar);
TypedValue* fg_drawgetstrokelinejoin(ActRec* ar);
TypedValue* fg_drawgetstrokemiterlimit(ActRec* ar);
TypedValue* fg_drawgetstrokeopacity(ActRec* ar);
TypedValue* fg_drawgetstrokewidth(ActRec* ar);
TypedValue* fg_drawgettextalignment(ActRec* ar);
TypedValue* fg_drawgettextantialias(ActRec* ar);
TypedValue* fg_drawgettextdecoration(ActRec* ar);
TypedValue* fg_drawgettextencoding(ActRec* ar);
TypedValue* fg_drawgettextundercolor(ActRec* ar);
TypedValue* fg_drawgetvectorgraphics(ActRec* ar);
TypedValue* fg_drawline(ActRec* ar);
TypedValue* fg_drawmatte(ActRec* ar);
TypedValue* fg_drawpathclose(ActRec* ar);
TypedValue* fg_drawpathcurvetoabsolute(ActRec* ar);
TypedValue* fg_drawpathcurvetoquadraticbezierabsolute(ActRec* ar);
TypedValue* fg_drawpathcurvetoquadraticbezierrelative(ActRec* ar);
TypedValue* fg_drawpathcurvetoquadraticbeziersmoothabsolute(ActRec* ar);
TypedValue* fg_drawpathcurvetoquadraticbeziersmoothrelative(ActRec* ar);
TypedValue* fg_drawpathcurvetorelative(ActRec* ar);
TypedValue* fg_drawpathcurvetosmoothabsolute(ActRec* ar);
TypedValue* fg_drawpathcurvetosmoothrelative(ActRec* ar);
TypedValue* fg_drawpathellipticarcabsolute(ActRec* ar);
TypedValue* fg_drawpathellipticarcrelative(ActRec* ar);
TypedValue* fg_drawpathfinish(ActRec* ar);
TypedValue* fg_drawpathlinetoabsolute(ActRec* ar);
TypedValue* fg_drawpathlinetohorizontalabsolute(ActRec* ar);
TypedValue* fg_drawpathlinetohorizontalrelative(ActRec* ar);
TypedValue* fg_drawpathlinetorelative(ActRec* ar);
TypedValue* fg_drawpathlinetoverticalabsolute(ActRec* ar);
TypedValue* fg_drawpathlinetoverticalrelative(ActRec* ar);
TypedValue* fg_drawpathmovetoabsolute(ActRec* ar);
TypedValue* fg_drawpathmovetorelative(ActRec* ar);
TypedValue* fg_drawpathstart(ActRec* ar);
TypedValue* fg_drawpoint(ActRec* ar);
TypedValue* fg_drawpolygon(ActRec* ar);
TypedValue* fg_drawpolyline(ActRec* ar);
TypedValue* fg_drawrectangle(ActRec* ar);
TypedValue* fg_drawrender(ActRec* ar);
TypedValue* fg_drawrotate(ActRec* ar);
TypedValue* fg_drawroundrectangle(ActRec* ar);
TypedValue* fg_drawscale(ActRec* ar);
TypedValue* fg_drawsetclippath(ActRec* ar);
TypedValue* fg_drawsetcliprule(ActRec* ar);
TypedValue* fg_drawsetclipunits(ActRec* ar);
TypedValue* fg_drawsetfillalpha(ActRec* ar);
TypedValue* fg_drawsetfillcolor(ActRec* ar);
TypedValue* fg_drawsetfillopacity(ActRec* ar);
TypedValue* fg_drawsetfillpatternurl(ActRec* ar);
TypedValue* fg_drawsetfillrule(ActRec* ar);
TypedValue* fg_drawsetfont(ActRec* ar);
TypedValue* fg_drawsetfontfamily(ActRec* ar);
TypedValue* fg_drawsetfontsize(ActRec* ar);
TypedValue* fg_drawsetfontstretch(ActRec* ar);
TypedValue* fg_drawsetfontstyle(ActRec* ar);
TypedValue* fg_drawsetfontweight(ActRec* ar);
TypedValue* fg_drawsetgravity(ActRec* ar);
TypedValue* fg_drawsetstrokealpha(ActRec* ar);
TypedValue* fg_drawsetstrokeantialias(ActRec* ar);
TypedValue* fg_drawsetstrokecolor(ActRec* ar);
TypedValue* fg_drawsetstrokedasharray(ActRec* ar);
TypedValue* fg_drawsetstrokedashoffset(ActRec* ar);
TypedValue* fg_drawsetstrokelinecap(ActRec* ar);
TypedValue* fg_drawsetstrokelinejoin(ActRec* ar);
TypedValue* fg_drawsetstrokemiterlimit(ActRec* ar);
TypedValue* fg_drawsetstrokeopacity(ActRec* ar);
TypedValue* fg_drawsetstrokepatternurl(ActRec* ar);
TypedValue* fg_drawsetstrokewidth(ActRec* ar);
TypedValue* fg_drawsettextalignment(ActRec* ar);
TypedValue* fg_drawsettextantialias(ActRec* ar);
TypedValue* fg_drawsettextdecoration(ActRec* ar);
TypedValue* fg_drawsettextencoding(ActRec* ar);
TypedValue* fg_drawsettextundercolor(ActRec* ar);
TypedValue* fg_drawsetvectorgraphics(ActRec* ar);
TypedValue* fg_drawsetviewbox(ActRec* ar);
TypedValue* fg_drawskewx(ActRec* ar);
TypedValue* fg_drawskewy(ActRec* ar);
TypedValue* fg_drawtranslate(ActRec* ar);
TypedValue* fg_pushdrawingwand(ActRec* ar);
TypedValue* fg_drawpushclippath(ActRec* ar);
TypedValue* fg_drawpushdefs(ActRec* ar);
TypedValue* fg_drawpushpattern(ActRec* ar);
TypedValue* fg_popdrawingwand(ActRec* ar);
TypedValue* fg_drawpopclippath(ActRec* ar);
TypedValue* fg_drawpopdefs(ActRec* ar);
TypedValue* fg_drawpoppattern(ActRec* ar);
TypedValue* fg_magickadaptivethresholdimage(ActRec* ar);
TypedValue* fg_magickaddimage(ActRec* ar);
TypedValue* fg_magickaddnoiseimage(ActRec* ar);
TypedValue* fg_magickaffinetransformimage(ActRec* ar);
TypedValue* fg_magickannotateimage(ActRec* ar);
TypedValue* fg_magickappendimages(ActRec* ar);
TypedValue* fg_magickaverageimages(ActRec* ar);
TypedValue* fg_magickblackthresholdimage(ActRec* ar);
TypedValue* fg_magickblurimage(ActRec* ar);
TypedValue* fg_magickborderimage(ActRec* ar);
TypedValue* fg_magickcharcoalimage(ActRec* ar);
TypedValue* fg_magickchopimage(ActRec* ar);
TypedValue* fg_magickclipimage(ActRec* ar);
TypedValue* fg_magickclippathimage(ActRec* ar);
TypedValue* fg_magickcoalesceimages(ActRec* ar);
TypedValue* fg_magickcolorfloodfillimage(ActRec* ar);
TypedValue* fg_magickcolorizeimage(ActRec* ar);
TypedValue* fg_magickcombineimages(ActRec* ar);
TypedValue* fg_magickcommentimage(ActRec* ar);
TypedValue* fg_magickcompareimages(ActRec* ar);
TypedValue* fg_magickcompositeimage(ActRec* ar);
TypedValue* fg_magickconstituteimage(ActRec* ar);
TypedValue* fg_magickcontrastimage(ActRec* ar);
TypedValue* fg_magickconvolveimage(ActRec* ar);
TypedValue* fg_magickcropimage(ActRec* ar);
TypedValue* fg_magickcyclecolormapimage(ActRec* ar);
TypedValue* fg_magickdeconstructimages(ActRec* ar);
TypedValue* fg_magickdescribeimage(ActRec* ar);
TypedValue* fg_magickdespeckleimage(ActRec* ar);
TypedValue* fg_magickdrawimage(ActRec* ar);
TypedValue* fg_magickechoimageblob(ActRec* ar);
TypedValue* fg_magickechoimagesblob(ActRec* ar);
TypedValue* fg_magickedgeimage(ActRec* ar);
TypedValue* fg_magickembossimage(ActRec* ar);
TypedValue* fg_magickenhanceimage(ActRec* ar);
TypedValue* fg_magickequalizeimage(ActRec* ar);
TypedValue* fg_magickevaluateimage(ActRec* ar);
TypedValue* fg_magickflattenimages(ActRec* ar);
TypedValue* fg_magickflipimage(ActRec* ar);
TypedValue* fg_magickflopimage(ActRec* ar);
TypedValue* fg_magickframeimage(ActRec* ar);
TypedValue* fg_magickfximage(ActRec* ar);
TypedValue* fg_magickgammaimage(ActRec* ar);
TypedValue* fg_magickgaussianblurimage(ActRec* ar);
TypedValue* fg_magickgetcharheight(ActRec* ar);
TypedValue* fg_magickgetcharwidth(ActRec* ar);
TypedValue* fg_magickgetexception(ActRec* ar);
TypedValue* fg_magickgetexceptionstring(ActRec* ar);
TypedValue* fg_magickgetexceptiontype(ActRec* ar);
TypedValue* fg_magickgetfilename(ActRec* ar);
TypedValue* fg_magickgetformat(ActRec* ar);
TypedValue* fg_magickgetimage(ActRec* ar);
TypedValue* fg_magickgetimagebackgroundcolor(ActRec* ar);
TypedValue* fg_magickgetimageblob(ActRec* ar);
TypedValue* fg_magickgetimageblueprimary(ActRec* ar);
TypedValue* fg_magickgetimagebordercolor(ActRec* ar);
TypedValue* fg_magickgetimagechannelmean(ActRec* ar);
TypedValue* fg_magickgetimagecolormapcolor(ActRec* ar);
TypedValue* fg_magickgetimagecolors(ActRec* ar);
TypedValue* fg_magickgetimagecolorspace(ActRec* ar);
TypedValue* fg_magickgetimagecompose(ActRec* ar);
TypedValue* fg_magickgetimagecompression(ActRec* ar);
TypedValue* fg_magickgetimagecompressionquality(ActRec* ar);
TypedValue* fg_magickgetimagedelay(ActRec* ar);
TypedValue* fg_magickgetimagedepth(ActRec* ar);
TypedValue* fg_magickgetimagedispose(ActRec* ar);
TypedValue* fg_magickgetimageextrema(ActRec* ar);
TypedValue* fg_magickgetimagefilename(ActRec* ar);
TypedValue* fg_magickgetimageformat(ActRec* ar);
TypedValue* fg_magickgetimagegamma(ActRec* ar);
TypedValue* fg_magickgetimagegreenprimary(ActRec* ar);
TypedValue* fg_magickgetimageheight(ActRec* ar);
TypedValue* fg_magickgetimagehistogram(ActRec* ar);
TypedValue* fg_magickgetimageindex(ActRec* ar);
TypedValue* fg_magickgetimageinterlacescheme(ActRec* ar);
TypedValue* fg_magickgetimageiterations(ActRec* ar);
TypedValue* fg_magickgetimagemattecolor(ActRec* ar);
TypedValue* fg_magickgetimagemimetype(ActRec* ar);
TypedValue* fg_magickgetimagepixels(ActRec* ar);
TypedValue* fg_magickgetimageprofile(ActRec* ar);
TypedValue* fg_magickgetimageredprimary(ActRec* ar);
TypedValue* fg_magickgetimagerenderingintent(ActRec* ar);
TypedValue* fg_magickgetimageresolution(ActRec* ar);
TypedValue* fg_magickgetimagescene(ActRec* ar);
TypedValue* fg_magickgetimagesignature(ActRec* ar);
TypedValue* fg_magickgetimagesize(ActRec* ar);
TypedValue* fg_magickgetimagetype(ActRec* ar);
TypedValue* fg_magickgetimageunits(ActRec* ar);
TypedValue* fg_magickgetimagevirtualpixelmethod(ActRec* ar);
TypedValue* fg_magickgetimagewhitepoint(ActRec* ar);
TypedValue* fg_magickgetimagewidth(ActRec* ar);
TypedValue* fg_magickgetimagesblob(ActRec* ar);
TypedValue* fg_magickgetinterlacescheme(ActRec* ar);
TypedValue* fg_magickgetmaxtextadvance(ActRec* ar);
TypedValue* fg_magickgetmimetype(ActRec* ar);
TypedValue* fg_magickgetnumberimages(ActRec* ar);
TypedValue* fg_magickgetsamplingfactors(ActRec* ar);
TypedValue* fg_magickgetsize(ActRec* ar);
TypedValue* fg_magickgetstringheight(ActRec* ar);
TypedValue* fg_magickgetstringwidth(ActRec* ar);
TypedValue* fg_magickgettextascent(ActRec* ar);
TypedValue* fg_magickgettextdescent(ActRec* ar);
TypedValue* fg_magickgetwandsize(ActRec* ar);
TypedValue* fg_magickhasnextimage(ActRec* ar);
TypedValue* fg_magickhaspreviousimage(ActRec* ar);
TypedValue* fg_magickimplodeimage(ActRec* ar);
TypedValue* fg_magicklabelimage(ActRec* ar);
TypedValue* fg_magicklevelimage(ActRec* ar);
TypedValue* fg_magickmagnifyimage(ActRec* ar);
TypedValue* fg_magickmapimage(ActRec* ar);
TypedValue* fg_magickmattefloodfillimage(ActRec* ar);
TypedValue* fg_magickmedianfilterimage(ActRec* ar);
TypedValue* fg_magickminifyimage(ActRec* ar);
TypedValue* fg_magickmodulateimage(ActRec* ar);
TypedValue* fg_magickmontageimage(ActRec* ar);
TypedValue* fg_magickmorphimages(ActRec* ar);
TypedValue* fg_magickmosaicimages(ActRec* ar);
TypedValue* fg_magickmotionblurimage(ActRec* ar);
TypedValue* fg_magicknegateimage(ActRec* ar);
TypedValue* fg_magicknewimage(ActRec* ar);
TypedValue* fg_magicknextimage(ActRec* ar);
TypedValue* fg_magicknormalizeimage(ActRec* ar);
TypedValue* fg_magickoilpaintimage(ActRec* ar);
TypedValue* fg_magickpaintopaqueimage(ActRec* ar);
TypedValue* fg_magickpainttransparentimage(ActRec* ar);
TypedValue* fg_magickpingimage(ActRec* ar);
TypedValue* fg_magickposterizeimage(ActRec* ar);
TypedValue* fg_magickpreviewimages(ActRec* ar);
TypedValue* fg_magickpreviousimage(ActRec* ar);
TypedValue* fg_magickprofileimage(ActRec* ar);
TypedValue* fg_magickquantizeimage(ActRec* ar);
TypedValue* fg_magickquantizeimages(ActRec* ar);
TypedValue* fg_magickqueryfontmetrics(ActRec* ar);
TypedValue* fg_magickradialblurimage(ActRec* ar);
TypedValue* fg_magickraiseimage(ActRec* ar);
TypedValue* fg_magickreadimage(ActRec* ar);
TypedValue* fg_magickreadimageblob(ActRec* ar);
TypedValue* fg_magickreadimagefile(ActRec* ar);
TypedValue* fg_magickreadimages(ActRec* ar);
TypedValue* fg_magickreducenoiseimage(ActRec* ar);
TypedValue* fg_magickremoveimage(ActRec* ar);
TypedValue* fg_magickremoveimageprofile(ActRec* ar);
TypedValue* fg_magickremoveimageprofiles(ActRec* ar);
TypedValue* fg_magickresampleimage(ActRec* ar);
TypedValue* fg_magickresetiterator(ActRec* ar);
TypedValue* fg_magickresizeimage(ActRec* ar);
TypedValue* fg_magickrollimage(ActRec* ar);
TypedValue* fg_magickrotateimage(ActRec* ar);
TypedValue* fg_magicksampleimage(ActRec* ar);
TypedValue* fg_magickscaleimage(ActRec* ar);
TypedValue* fg_magickseparateimagechannel(ActRec* ar);
TypedValue* fg_magicksetcompressionquality(ActRec* ar);
TypedValue* fg_magicksetfilename(ActRec* ar);
TypedValue* fg_magicksetfirstiterator(ActRec* ar);
TypedValue* fg_magicksetformat(ActRec* ar);
TypedValue* fg_magicksetimage(ActRec* ar);
TypedValue* fg_magicksetimagebackgroundcolor(ActRec* ar);
TypedValue* fg_magicksetimagebias(ActRec* ar);
TypedValue* fg_magicksetimageblueprimary(ActRec* ar);
TypedValue* fg_magicksetimagebordercolor(ActRec* ar);
TypedValue* fg_magicksetimagecolormapcolor(ActRec* ar);
TypedValue* fg_magicksetimagecolorspace(ActRec* ar);
TypedValue* fg_magicksetimagecompose(ActRec* ar);
TypedValue* fg_magicksetimagecompression(ActRec* ar);
TypedValue* fg_magicksetimagecompressionquality(ActRec* ar);
TypedValue* fg_magicksetimagedelay(ActRec* ar);
TypedValue* fg_magicksetimagedepth(ActRec* ar);
TypedValue* fg_magicksetimagedispose(ActRec* ar);
TypedValue* fg_magicksetimagefilename(ActRec* ar);
TypedValue* fg_magicksetimageformat(ActRec* ar);
TypedValue* fg_magicksetimagegamma(ActRec* ar);
TypedValue* fg_magicksetimagegreenprimary(ActRec* ar);
TypedValue* fg_magicksetimageindex(ActRec* ar);
TypedValue* fg_magicksetimageinterlacescheme(ActRec* ar);
TypedValue* fg_magicksetimageiterations(ActRec* ar);
TypedValue* fg_magicksetimagemattecolor(ActRec* ar);
TypedValue* fg_magicksetimageoption(ActRec* ar);
TypedValue* fg_magicksetimagepixels(ActRec* ar);
TypedValue* fg_magicksetimageprofile(ActRec* ar);
TypedValue* fg_magicksetimageredprimary(ActRec* ar);
TypedValue* fg_magicksetimagerenderingintent(ActRec* ar);
TypedValue* fg_magicksetimageresolution(ActRec* ar);
TypedValue* fg_magicksetimagescene(ActRec* ar);
TypedValue* fg_magicksetimagetype(ActRec* ar);
TypedValue* fg_magicksetimageunits(ActRec* ar);
TypedValue* fg_magicksetimagevirtualpixelmethod(ActRec* ar);
TypedValue* fg_magicksetimagewhitepoint(ActRec* ar);
TypedValue* fg_magicksetinterlacescheme(ActRec* ar);
TypedValue* fg_magicksetlastiterator(ActRec* ar);
TypedValue* fg_magicksetpassphrase(ActRec* ar);
TypedValue* fg_magicksetresolution(ActRec* ar);
TypedValue* fg_magicksetsamplingfactors(ActRec* ar);
TypedValue* fg_magicksetsize(ActRec* ar);
TypedValue* fg_magicksetwandsize(ActRec* ar);
TypedValue* fg_magicksharpenimage(ActRec* ar);
TypedValue* fg_magickshaveimage(ActRec* ar);
TypedValue* fg_magickshearimage(ActRec* ar);
TypedValue* fg_magicksolarizeimage(ActRec* ar);
TypedValue* fg_magickspliceimage(ActRec* ar);
TypedValue* fg_magickspreadimage(ActRec* ar);
TypedValue* fg_magicksteganoimage(ActRec* ar);
TypedValue* fg_magickstereoimage(ActRec* ar);
TypedValue* fg_magickstripimage(ActRec* ar);
TypedValue* fg_magickswirlimage(ActRec* ar);
TypedValue* fg_magicktextureimage(ActRec* ar);
TypedValue* fg_magickthresholdimage(ActRec* ar);
TypedValue* fg_magicktintimage(ActRec* ar);
TypedValue* fg_magicktransformimage(ActRec* ar);
TypedValue* fg_magicktrimimage(ActRec* ar);
TypedValue* fg_magickunsharpmaskimage(ActRec* ar);
TypedValue* fg_magickwaveimage(ActRec* ar);
TypedValue* fg_magickwhitethresholdimage(ActRec* ar);
TypedValue* fg_magickwriteimage(ActRec* ar);
TypedValue* fg_magickwriteimagefile(ActRec* ar);
TypedValue* fg_magickwriteimages(ActRec* ar);
TypedValue* fg_magickwriteimagesfile(ActRec* ar);
TypedValue* fg_pixelgetalpha(ActRec* ar);
TypedValue* fg_pixelgetalphaquantum(ActRec* ar);
TypedValue* fg_pixelgetblack(ActRec* ar);
TypedValue* fg_pixelgetblackquantum(ActRec* ar);
TypedValue* fg_pixelgetblue(ActRec* ar);
TypedValue* fg_pixelgetbluequantum(ActRec* ar);
TypedValue* fg_pixelgetcolorasstring(ActRec* ar);
TypedValue* fg_pixelgetcolorcount(ActRec* ar);
TypedValue* fg_pixelgetcyan(ActRec* ar);
TypedValue* fg_pixelgetcyanquantum(ActRec* ar);
TypedValue* fg_pixelgetexception(ActRec* ar);
TypedValue* fg_pixelgetexceptionstring(ActRec* ar);
TypedValue* fg_pixelgetexceptiontype(ActRec* ar);
TypedValue* fg_pixelgetgreen(ActRec* ar);
TypedValue* fg_pixelgetgreenquantum(ActRec* ar);
TypedValue* fg_pixelgetindex(ActRec* ar);
TypedValue* fg_pixelgetmagenta(ActRec* ar);
TypedValue* fg_pixelgetmagentaquantum(ActRec* ar);
TypedValue* fg_pixelgetopacity(ActRec* ar);
TypedValue* fg_pixelgetopacityquantum(ActRec* ar);
TypedValue* fg_pixelgetquantumcolor(ActRec* ar);
TypedValue* fg_pixelgetred(ActRec* ar);
TypedValue* fg_pixelgetredquantum(ActRec* ar);
TypedValue* fg_pixelgetyellow(ActRec* ar);
TypedValue* fg_pixelgetyellowquantum(ActRec* ar);
TypedValue* fg_pixelsetalpha(ActRec* ar);
TypedValue* fg_pixelsetalphaquantum(ActRec* ar);
TypedValue* fg_pixelsetblack(ActRec* ar);
TypedValue* fg_pixelsetblackquantum(ActRec* ar);
TypedValue* fg_pixelsetblue(ActRec* ar);
TypedValue* fg_pixelsetbluequantum(ActRec* ar);
TypedValue* fg_pixelsetcolor(ActRec* ar);
TypedValue* fg_pixelsetcolorcount(ActRec* ar);
TypedValue* fg_pixelsetcyan(ActRec* ar);
TypedValue* fg_pixelsetcyanquantum(ActRec* ar);
TypedValue* fg_pixelsetgreen(ActRec* ar);
TypedValue* fg_pixelsetgreenquantum(ActRec* ar);
TypedValue* fg_pixelsetindex(ActRec* ar);
TypedValue* fg_pixelsetmagenta(ActRec* ar);
TypedValue* fg_pixelsetmagentaquantum(ActRec* ar);
TypedValue* fg_pixelsetopacity(ActRec* ar);
TypedValue* fg_pixelsetopacityquantum(ActRec* ar);
TypedValue* fg_pixelsetquantumcolor(ActRec* ar);
TypedValue* fg_pixelsetred(ActRec* ar);
TypedValue* fg_pixelsetredquantum(ActRec* ar);
TypedValue* fg_pixelsetyellow(ActRec* ar);
TypedValue* fg_pixelsetyellowquantum(ActRec* ar);
TypedValue* fg_pixelgetiteratorexception(ActRec* ar);
TypedValue* fg_pixelgetiteratorexceptionstring(ActRec* ar);
TypedValue* fg_pixelgetiteratorexceptiontype(ActRec* ar);
TypedValue* fg_pixelgetnextiteratorrow(ActRec* ar);
TypedValue* fg_pixelresetiterator(ActRec* ar);
TypedValue* fg_pixelsetiteratorrow(ActRec* ar);
TypedValue* fg_pixelsynciterator(ActRec* ar);
TypedValue* fg_mail(ActRec* ar);
TypedValue* fg_ezmlm_hash(ActRec* ar);
TypedValue* fg_mailparse_msg_create(ActRec* ar);
TypedValue* fg_mailparse_msg_free(ActRec* ar);
TypedValue* fg_mailparse_msg_parse_file(ActRec* ar);
TypedValue* fg_mailparse_msg_parse(ActRec* ar);
TypedValue* fg_mailparse_msg_extract_part_file(ActRec* ar);
TypedValue* fg_mailparse_msg_extract_whole_part_file(ActRec* ar);
TypedValue* fg_mailparse_msg_extract_part(ActRec* ar);
TypedValue* fg_mailparse_msg_get_part_data(ActRec* ar);
TypedValue* fg_mailparse_msg_get_part(ActRec* ar);
TypedValue* fg_mailparse_msg_get_structure(ActRec* ar);
TypedValue* fg_mailparse_rfc822_parse_addresses(ActRec* ar);
TypedValue* fg_mailparse_stream_encode(ActRec* ar);
TypedValue* fg_mailparse_uudecode_all(ActRec* ar);
TypedValue* fg_mailparse_determine_best_xfer_encoding(ActRec* ar);
TypedValue* fg_pi(ActRec* ar);
TypedValue* fg_min(ActRec* ar);
TypedValue* fg_max(ActRec* ar);
TypedValue* fg_abs(ActRec* ar);
TypedValue* fg_is_finite(ActRec* ar);
TypedValue* fg_is_infinite(ActRec* ar);
TypedValue* fg_is_nan(ActRec* ar);
TypedValue* fg_ceil(ActRec* ar);
TypedValue* fg_floor(ActRec* ar);
TypedValue* fg_round(ActRec* ar);
TypedValue* fg_deg2rad(ActRec* ar);
TypedValue* fg_rad2deg(ActRec* ar);
TypedValue* fg_decbin(ActRec* ar);
TypedValue* fg_dechex(ActRec* ar);
TypedValue* fg_decoct(ActRec* ar);
TypedValue* fg_bindec(ActRec* ar);
TypedValue* fg_hexdec(ActRec* ar);
TypedValue* fg_octdec(ActRec* ar);
TypedValue* fg_base_convert(ActRec* ar);
TypedValue* fg_pow(ActRec* ar);
TypedValue* fg_exp(ActRec* ar);
TypedValue* fg_expm1(ActRec* ar);
TypedValue* fg_log10(ActRec* ar);
TypedValue* fg_log1p(ActRec* ar);
TypedValue* fg_log(ActRec* ar);
TypedValue* fg_cos(ActRec* ar);
TypedValue* fg_cosh(ActRec* ar);
TypedValue* fg_sin(ActRec* ar);
TypedValue* fg_sinh(ActRec* ar);
TypedValue* fg_tan(ActRec* ar);
TypedValue* fg_tanh(ActRec* ar);
TypedValue* fg_acos(ActRec* ar);
TypedValue* fg_acosh(ActRec* ar);
TypedValue* fg_asin(ActRec* ar);
TypedValue* fg_asinh(ActRec* ar);
TypedValue* fg_atan(ActRec* ar);
TypedValue* fg_atanh(ActRec* ar);
TypedValue* fg_atan2(ActRec* ar);
TypedValue* fg_hypot(ActRec* ar);
TypedValue* fg_fmod(ActRec* ar);
TypedValue* fg_sqrt(ActRec* ar);
TypedValue* fg_getrandmax(ActRec* ar);
TypedValue* fg_srand(ActRec* ar);
TypedValue* fg_rand(ActRec* ar);
TypedValue* fg_mt_getrandmax(ActRec* ar);
TypedValue* fg_mt_srand(ActRec* ar);
TypedValue* fg_mt_rand(ActRec* ar);
TypedValue* fg_lcg_value(ActRec* ar);
TypedValue* fg_mb_list_encodings(ActRec* ar);
TypedValue* fg_mb_list_encodings_alias_names(ActRec* ar);
TypedValue* fg_mb_list_mime_names(ActRec* ar);
TypedValue* fg_mb_check_encoding(ActRec* ar);
TypedValue* fg_mb_convert_case(ActRec* ar);
TypedValue* fg_mb_convert_encoding(ActRec* ar);
TypedValue* fg_mb_convert_kana(ActRec* ar);
TypedValue* fg_mb_convert_variables(ActRec* ar);
TypedValue* fg_mb_decode_mimeheader(ActRec* ar);
TypedValue* fg_mb_decode_numericentity(ActRec* ar);
TypedValue* fg_mb_detect_encoding(ActRec* ar);
TypedValue* fg_mb_detect_order(ActRec* ar);
TypedValue* fg_mb_encode_mimeheader(ActRec* ar);
TypedValue* fg_mb_encode_numericentity(ActRec* ar);
TypedValue* fg_mb_ereg_match(ActRec* ar);
TypedValue* fg_mb_ereg_replace(ActRec* ar);
TypedValue* fg_mb_ereg_search_getpos(ActRec* ar);
TypedValue* fg_mb_ereg_search_getregs(ActRec* ar);
TypedValue* fg_mb_ereg_search_init(ActRec* ar);
TypedValue* fg_mb_ereg_search_pos(ActRec* ar);
TypedValue* fg_mb_ereg_search_regs(ActRec* ar);
TypedValue* fg_mb_ereg_search_setpos(ActRec* ar);
TypedValue* fg_mb_ereg_search(ActRec* ar);
TypedValue* fg_mb_ereg(ActRec* ar);
TypedValue* fg_mb_eregi_replace(ActRec* ar);
TypedValue* fg_mb_eregi(ActRec* ar);
TypedValue* fg_mb_get_info(ActRec* ar);
TypedValue* fg_mb_http_input(ActRec* ar);
TypedValue* fg_mb_http_output(ActRec* ar);
TypedValue* fg_mb_internal_encoding(ActRec* ar);
TypedValue* fg_mb_language(ActRec* ar);
TypedValue* fg_mb_output_handler(ActRec* ar);
TypedValue* fg_mb_parse_str(ActRec* ar);
TypedValue* fg_mb_preferred_mime_name(ActRec* ar);
TypedValue* fg_mb_regex_encoding(ActRec* ar);
TypedValue* fg_mb_regex_set_options(ActRec* ar);
TypedValue* fg_mb_send_mail(ActRec* ar);
TypedValue* fg_mb_split(ActRec* ar);
TypedValue* fg_mb_strcut(ActRec* ar);
TypedValue* fg_mb_strimwidth(ActRec* ar);
TypedValue* fg_mb_stripos(ActRec* ar);
TypedValue* fg_mb_stristr(ActRec* ar);
TypedValue* fg_mb_strlen(ActRec* ar);
TypedValue* fg_mb_strpos(ActRec* ar);
TypedValue* fg_mb_strrchr(ActRec* ar);
TypedValue* fg_mb_strrichr(ActRec* ar);
TypedValue* fg_mb_strripos(ActRec* ar);
TypedValue* fg_mb_strrpos(ActRec* ar);
TypedValue* fg_mb_strstr(ActRec* ar);
TypedValue* fg_mb_strtolower(ActRec* ar);
TypedValue* fg_mb_strtoupper(ActRec* ar);
TypedValue* fg_mb_strwidth(ActRec* ar);
TypedValue* fg_mb_substitute_character(ActRec* ar);
TypedValue* fg_mb_substr_count(ActRec* ar);
TypedValue* fg_mb_substr(ActRec* ar);
TypedValue* fg_mcrypt_module_open(ActRec* ar);
TypedValue* fg_mcrypt_module_close(ActRec* ar);
TypedValue* fg_mcrypt_list_algorithms(ActRec* ar);
TypedValue* fg_mcrypt_list_modes(ActRec* ar);
TypedValue* fg_mcrypt_module_get_algo_block_size(ActRec* ar);
TypedValue* fg_mcrypt_module_get_algo_key_size(ActRec* ar);
TypedValue* fg_mcrypt_module_get_supported_key_sizes(ActRec* ar);
TypedValue* fg_mcrypt_module_is_block_algorithm_mode(ActRec* ar);
TypedValue* fg_mcrypt_module_is_block_algorithm(ActRec* ar);
TypedValue* fg_mcrypt_module_is_block_mode(ActRec* ar);
TypedValue* fg_mcrypt_module_self_test(ActRec* ar);
TypedValue* fg_mcrypt_create_iv(ActRec* ar);
TypedValue* fg_mcrypt_encrypt(ActRec* ar);
TypedValue* fg_mcrypt_decrypt(ActRec* ar);
TypedValue* fg_mcrypt_cbc(ActRec* ar);
TypedValue* fg_mcrypt_cfb(ActRec* ar);
TypedValue* fg_mcrypt_ecb(ActRec* ar);
TypedValue* fg_mcrypt_ofb(ActRec* ar);
TypedValue* fg_mcrypt_get_block_size(ActRec* ar);
TypedValue* fg_mcrypt_get_cipher_name(ActRec* ar);
TypedValue* fg_mcrypt_get_iv_size(ActRec* ar);
TypedValue* fg_mcrypt_get_key_size(ActRec* ar);
TypedValue* fg_mcrypt_enc_get_algorithms_name(ActRec* ar);
TypedValue* fg_mcrypt_enc_get_block_size(ActRec* ar);
TypedValue* fg_mcrypt_enc_get_iv_size(ActRec* ar);
TypedValue* fg_mcrypt_enc_get_key_size(ActRec* ar);
TypedValue* fg_mcrypt_enc_get_modes_name(ActRec* ar);
TypedValue* fg_mcrypt_enc_get_supported_key_sizes(ActRec* ar);
TypedValue* fg_mcrypt_enc_is_block_algorithm_mode(ActRec* ar);
TypedValue* fg_mcrypt_enc_is_block_algorithm(ActRec* ar);
TypedValue* fg_mcrypt_enc_is_block_mode(ActRec* ar);
TypedValue* fg_mcrypt_enc_self_test(ActRec* ar);
TypedValue* fg_mcrypt_generic(ActRec* ar);
TypedValue* fg_mcrypt_generic_init(ActRec* ar);
TypedValue* fg_mdecrypt_generic(ActRec* ar);
TypedValue* fg_mcrypt_generic_deinit(ActRec* ar);
TypedValue* fg_mcrypt_generic_end(ActRec* ar);
TypedValue* fg_memcache_connect(ActRec* ar);
TypedValue* fg_memcache_pconnect(ActRec* ar);
TypedValue* fg_memcache_add(ActRec* ar);
TypedValue* fg_memcache_set(ActRec* ar);
TypedValue* fg_memcache_replace(ActRec* ar);
TypedValue* fg_memcache_get(ActRec* ar);
TypedValue* fg_memcache_delete(ActRec* ar);
TypedValue* fg_memcache_increment(ActRec* ar);
TypedValue* fg_memcache_decrement(ActRec* ar);
TypedValue* fg_memcache_close(ActRec* ar);
TypedValue* fg_memcache_debug(ActRec* ar);
TypedValue* fg_memcache_get_version(ActRec* ar);
TypedValue* fg_memcache_flush(ActRec* ar);
TypedValue* fg_memcache_setoptimeout(ActRec* ar);
TypedValue* fg_memcache_get_server_status(ActRec* ar);
TypedValue* fg_memcache_set_compress_threshold(ActRec* ar);
TypedValue* fg_memcache_get_stats(ActRec* ar);
TypedValue* fg_memcache_get_extended_stats(ActRec* ar);
TypedValue* fg_memcache_set_server_params(ActRec* ar);
TypedValue* fg_memcache_add_server(ActRec* ar);
TypedValue* fg_connection_aborted(ActRec* ar);
TypedValue* fg_connection_status(ActRec* ar);
TypedValue* fg_connection_timeout(ActRec* ar);
TypedValue* fg_constant(ActRec* ar);
TypedValue* fg_define(ActRec* ar);
TypedValue* fg_defined(ActRec* ar);
TypedValue* fg_die(ActRec* ar);
TypedValue* fg_exit(ActRec* ar);
TypedValue* fg_eval(ActRec* ar);
TypedValue* fg_get_browser(ActRec* ar);
TypedValue* fg___halt_compiler(ActRec* ar);
TypedValue* fg_highlight_file(ActRec* ar);
TypedValue* fg_show_source(ActRec* ar);
TypedValue* fg_highlight_string(ActRec* ar);
TypedValue* fg_ignore_user_abort(ActRec* ar);
TypedValue* fg_pack(ActRec* ar);
TypedValue* fg_php_check_syntax(ActRec* ar);
TypedValue* fg_php_strip_whitespace(ActRec* ar);
TypedValue* fg_sleep(ActRec* ar);
TypedValue* fg_usleep(ActRec* ar);
TypedValue* fg_time_nanosleep(ActRec* ar);
TypedValue* fg_time_sleep_until(ActRec* ar);
TypedValue* fg_uniqid(ActRec* ar);
TypedValue* fg_unpack(ActRec* ar);
TypedValue* fg_sys_getloadavg(ActRec* ar);
TypedValue* fg_token_get_all(ActRec* ar);
TypedValue* fg_token_name(ActRec* ar);
TypedValue* fg_hphp_process_abort(ActRec* ar);
TypedValue* fg_hphp_to_string(ActRec* ar);
TypedValue* fg_mysql_connect(ActRec* ar);
TypedValue* fg_mysql_async_connect_start(ActRec* ar);
TypedValue* fg_mysql_async_connect_completed(ActRec* ar);
TypedValue* fg_mysql_async_query_start(ActRec* ar);
TypedValue* fg_mysql_async_query_result(ActRec* ar);
TypedValue* fg_mysql_async_query_completed(ActRec* ar);
TypedValue* fg_mysql_async_fetch_array(ActRec* ar);
TypedValue* fg_mysql_async_wait_actionable(ActRec* ar);
TypedValue* fg_mysql_async_status(ActRec* ar);
TypedValue* fg_mysql_pconnect(ActRec* ar);
TypedValue* fg_mysql_connect_with_db(ActRec* ar);
TypedValue* fg_mysql_pconnect_with_db(ActRec* ar);
TypedValue* fg_mysql_set_charset(ActRec* ar);
TypedValue* fg_mysql_ping(ActRec* ar);
TypedValue* fg_mysql_escape_string(ActRec* ar);
TypedValue* fg_mysql_real_escape_string(ActRec* ar);
TypedValue* fg_mysql_client_encoding(ActRec* ar);
TypedValue* fg_mysql_close(ActRec* ar);
TypedValue* fg_mysql_errno(ActRec* ar);
TypedValue* fg_mysql_error(ActRec* ar);
TypedValue* fg_mysql_warning_count(ActRec* ar);
TypedValue* fg_mysql_get_client_info(ActRec* ar);
TypedValue* fg_mysql_get_host_info(ActRec* ar);
TypedValue* fg_mysql_get_proto_info(ActRec* ar);
TypedValue* fg_mysql_get_server_info(ActRec* ar);
TypedValue* fg_mysql_info(ActRec* ar);
TypedValue* fg_mysql_insert_id(ActRec* ar);
TypedValue* fg_mysql_stat(ActRec* ar);
TypedValue* fg_mysql_thread_id(ActRec* ar);
TypedValue* fg_mysql_create_db(ActRec* ar);
TypedValue* fg_mysql_select_db(ActRec* ar);
TypedValue* fg_mysql_drop_db(ActRec* ar);
TypedValue* fg_mysql_affected_rows(ActRec* ar);
TypedValue* fg_mysql_set_timeout(ActRec* ar);
TypedValue* fg_mysql_query(ActRec* ar);
TypedValue* fg_mysql_multi_query(ActRec* ar);
TypedValue* fg_mysql_next_result(ActRec* ar);
TypedValue* fg_mysql_more_results(ActRec* ar);
TypedValue* fg_mysql_fetch_result(ActRec* ar);
TypedValue* fg_mysql_unbuffered_query(ActRec* ar);
TypedValue* fg_mysql_db_query(ActRec* ar);
TypedValue* fg_mysql_list_dbs(ActRec* ar);
TypedValue* fg_mysql_list_tables(ActRec* ar);
TypedValue* fg_mysql_list_fields(ActRec* ar);
TypedValue* fg_mysql_list_processes(ActRec* ar);
TypedValue* fg_mysql_db_name(ActRec* ar);
TypedValue* fg_mysql_tablename(ActRec* ar);
TypedValue* fg_mysql_num_fields(ActRec* ar);
TypedValue* fg_mysql_num_rows(ActRec* ar);
TypedValue* fg_mysql_free_result(ActRec* ar);
TypedValue* fg_mysql_data_seek(ActRec* ar);
TypedValue* fg_mysql_fetch_row(ActRec* ar);
TypedValue* fg_mysql_fetch_assoc(ActRec* ar);
TypedValue* fg_mysql_fetch_array(ActRec* ar);
TypedValue* fg_mysql_fetch_lengths(ActRec* ar);
TypedValue* fg_mysql_fetch_object(ActRec* ar);
TypedValue* fg_mysql_result(ActRec* ar);
TypedValue* fg_mysql_fetch_field(ActRec* ar);
TypedValue* fg_mysql_field_seek(ActRec* ar);
TypedValue* fg_mysql_field_name(ActRec* ar);
TypedValue* fg_mysql_field_table(ActRec* ar);
TypedValue* fg_mysql_field_len(ActRec* ar);
TypedValue* fg_mysql_field_type(ActRec* ar);
TypedValue* fg_mysql_field_flags(ActRec* ar);
TypedValue* fg_gethostname(ActRec* ar);
TypedValue* fg_gethostbyaddr(ActRec* ar);
TypedValue* fg_gethostbyname(ActRec* ar);
TypedValue* fg_gethostbynamel(ActRec* ar);
TypedValue* fg_getprotobyname(ActRec* ar);
TypedValue* fg_getprotobynumber(ActRec* ar);
TypedValue* fg_getservbyname(ActRec* ar);
TypedValue* fg_getservbyport(ActRec* ar);
TypedValue* fg_inet_ntop(ActRec* ar);
TypedValue* fg_inet_pton(ActRec* ar);
TypedValue* fg_ip2long(ActRec* ar);
TypedValue* fg_long2ip(ActRec* ar);
TypedValue* fg_dns_check_record(ActRec* ar);
TypedValue* fg_checkdnsrr(ActRec* ar);
TypedValue* fg_dns_get_record(ActRec* ar);
TypedValue* fg_dns_get_mx(ActRec* ar);
TypedValue* fg_getmxrr(ActRec* ar);
TypedValue* fg_fsockopen(ActRec* ar);
TypedValue* fg_pfsockopen(ActRec* ar);
TypedValue* fg_socket_get_status(ActRec* ar);
TypedValue* fg_socket_set_blocking(ActRec* ar);
TypedValue* fg_socket_set_timeout(ActRec* ar);
TypedValue* fg_header(ActRec* ar);
TypedValue* fg_http_response_code(ActRec* ar);
TypedValue* fg_headers_list(ActRec* ar);
TypedValue* fg_get_http_request_size(ActRec* ar);
TypedValue* fg_headers_sent(ActRec* ar);
TypedValue* fg_header_register_callback(ActRec* ar);
TypedValue* fg_header_remove(ActRec* ar);
TypedValue* fg_setcookie(ActRec* ar);
TypedValue* fg_setrawcookie(ActRec* ar);
TypedValue* fg_define_syslog_variables(ActRec* ar);
TypedValue* fg_openlog(ActRec* ar);
TypedValue* fg_closelog(ActRec* ar);
TypedValue* fg_syslog(ActRec* ar);
TypedValue* fg_openssl_csr_export_to_file(ActRec* ar);
TypedValue* fg_openssl_csr_export(ActRec* ar);
TypedValue* fg_openssl_csr_get_public_key(ActRec* ar);
TypedValue* fg_openssl_csr_get_subject(ActRec* ar);
TypedValue* fg_openssl_csr_new(ActRec* ar);
TypedValue* fg_openssl_csr_sign(ActRec* ar);
TypedValue* fg_openssl_error_string(ActRec* ar);
TypedValue* fg_openssl_open(ActRec* ar);
TypedValue* fg_openssl_pkcs12_export_to_file(ActRec* ar);
TypedValue* fg_openssl_pkcs12_export(ActRec* ar);
TypedValue* fg_openssl_pkcs12_read(ActRec* ar);
TypedValue* fg_openssl_pkcs7_decrypt(ActRec* ar);
TypedValue* fg_openssl_pkcs7_encrypt(ActRec* ar);
TypedValue* fg_openssl_pkcs7_sign(ActRec* ar);
TypedValue* fg_openssl_pkcs7_verify(ActRec* ar);
TypedValue* fg_openssl_pkey_export_to_file(ActRec* ar);
TypedValue* fg_openssl_pkey_export(ActRec* ar);
TypedValue* fg_openssl_pkey_free(ActRec* ar);
TypedValue* fg_openssl_free_key(ActRec* ar);
TypedValue* fg_openssl_pkey_get_details(ActRec* ar);
TypedValue* fg_openssl_pkey_get_private(ActRec* ar);
TypedValue* fg_openssl_get_privatekey(ActRec* ar);
TypedValue* fg_openssl_pkey_get_public(ActRec* ar);
TypedValue* fg_openssl_get_publickey(ActRec* ar);
TypedValue* fg_openssl_pkey_new(ActRec* ar);
TypedValue* fg_openssl_private_decrypt(ActRec* ar);
TypedValue* fg_openssl_private_encrypt(ActRec* ar);
TypedValue* fg_openssl_public_decrypt(ActRec* ar);
TypedValue* fg_openssl_public_encrypt(ActRec* ar);
TypedValue* fg_openssl_seal(ActRec* ar);
TypedValue* fg_openssl_sign(ActRec* ar);
TypedValue* fg_openssl_verify(ActRec* ar);
TypedValue* fg_openssl_x509_check_private_key(ActRec* ar);
TypedValue* fg_openssl_x509_checkpurpose(ActRec* ar);
TypedValue* fg_openssl_x509_export_to_file(ActRec* ar);
TypedValue* fg_openssl_x509_export(ActRec* ar);
TypedValue* fg_openssl_x509_free(ActRec* ar);
TypedValue* fg_openssl_x509_parse(ActRec* ar);
TypedValue* fg_openssl_x509_read(ActRec* ar);
TypedValue* fg_openssl_random_pseudo_bytes(ActRec* ar);
TypedValue* fg_openssl_cipher_iv_length(ActRec* ar);
TypedValue* fg_openssl_encrypt(ActRec* ar);
TypedValue* fg_openssl_decrypt(ActRec* ar);
TypedValue* fg_openssl_digest(ActRec* ar);
TypedValue* fg_openssl_get_cipher_methods(ActRec* ar);
TypedValue* fg_openssl_get_md_methods(ActRec* ar);
TypedValue* fg_assert_options(ActRec* ar);
TypedValue* fg_assert(ActRec* ar);
TypedValue* fg_dl(ActRec* ar);
TypedValue* fg_extension_loaded(ActRec* ar);
TypedValue* fg_get_loaded_extensions(ActRec* ar);
TypedValue* fg_get_extension_funcs(ActRec* ar);
TypedValue* fg_get_cfg_var(ActRec* ar);
TypedValue* fg_get_current_user(ActRec* ar);
TypedValue* fg_get_defined_constants(ActRec* ar);
TypedValue* fg_get_include_path(ActRec* ar);
TypedValue* fg_restore_include_path(ActRec* ar);
TypedValue* fg_set_include_path(ActRec* ar);
TypedValue* fg_get_included_files(ActRec* ar);
TypedValue* fg_inclued_get_data(ActRec* ar);
TypedValue* fg_get_magic_quotes_gpc(ActRec* ar);
TypedValue* fg_get_magic_quotes_runtime(ActRec* ar);
TypedValue* fg_get_required_files(ActRec* ar);
TypedValue* fg_getenv(ActRec* ar);
TypedValue* fg_getlastmod(ActRec* ar);
TypedValue* fg_getmygid(ActRec* ar);
TypedValue* fg_getmyinode(ActRec* ar);
TypedValue* fg_getmypid(ActRec* ar);
TypedValue* fg_getmyuid(ActRec* ar);
TypedValue* fg_getopt(ActRec* ar);
TypedValue* fg_getrusage(ActRec* ar);
TypedValue* fg_clock_getres(ActRec* ar);
TypedValue* fg_clock_gettime(ActRec* ar);
TypedValue* fg_clock_settime(ActRec* ar);
TypedValue* fg_cpu_get_count(ActRec* ar);
TypedValue* fg_cpu_get_model(ActRec* ar);
TypedValue* fg_ini_alter(ActRec* ar);
TypedValue* fg_ini_get_all(ActRec* ar);
TypedValue* fg_ini_get(ActRec* ar);
TypedValue* fg_ini_restore(ActRec* ar);
TypedValue* fg_ini_set(ActRec* ar);
TypedValue* fg_memory_get_allocation(ActRec* ar);
TypedValue* fg_memory_get_peak_usage(ActRec* ar);
TypedValue* fg_memory_get_usage(ActRec* ar);
TypedValue* fg_php_ini_scanned_files(ActRec* ar);
TypedValue* fg_php_logo_guid(ActRec* ar);
TypedValue* fg_php_sapi_name(ActRec* ar);
TypedValue* fg_php_uname(ActRec* ar);
TypedValue* fg_phpcredits(ActRec* ar);
TypedValue* fg_phpinfo(ActRec* ar);
TypedValue* fg_phpversion(ActRec* ar);
TypedValue* fg_putenv(ActRec* ar);
TypedValue* fg_set_magic_quotes_runtime(ActRec* ar);
TypedValue* fg_set_time_limit(ActRec* ar);
TypedValue* fg_sys_get_temp_dir(ActRec* ar);
TypedValue* fg_version_compare(ActRec* ar);
TypedValue* fg_gc_enabled(ActRec* ar);
TypedValue* fg_gc_enable(ActRec* ar);
TypedValue* fg_gc_disable(ActRec* ar);
TypedValue* fg_gc_collect_cycles(ActRec* ar);
TypedValue* fg_zend_logo_guid(ActRec* ar);
TypedValue* fg_zend_thread_id(ActRec* ar);
TypedValue* fg_zend_version(ActRec* ar);
TypedValue* fg_ob_start(ActRec* ar);
TypedValue* fg_ob_clean(ActRec* ar);
TypedValue* fg_ob_flush(ActRec* ar);
TypedValue* fg_ob_end_clean(ActRec* ar);
TypedValue* fg_ob_end_flush(ActRec* ar);
TypedValue* fg_flush(ActRec* ar);
TypedValue* fg_ob_get_clean(ActRec* ar);
TypedValue* fg_ob_get_contents(ActRec* ar);
TypedValue* fg_ob_get_flush(ActRec* ar);
TypedValue* fg_ob_get_length(ActRec* ar);
TypedValue* fg_ob_get_level(ActRec* ar);
TypedValue* fg_ob_get_status(ActRec* ar);
TypedValue* fg_ob_gzhandler(ActRec* ar);
TypedValue* fg_ob_implicit_flush(ActRec* ar);
TypedValue* fg_ob_list_handlers(ActRec* ar);
TypedValue* fg_output_add_rewrite_var(ActRec* ar);
TypedValue* fg_output_reset_rewrite_vars(ActRec* ar);
TypedValue* fg_hphp_crash_log(ActRec* ar);
TypedValue* fg_hphp_stats(ActRec* ar);
TypedValue* fg_hphp_get_stats(ActRec* ar);
TypedValue* fg_hphp_get_status(ActRec* ar);
TypedValue* fg_hphp_get_iostatus(ActRec* ar);
TypedValue* fg_hphp_set_iostatus_address(ActRec* ar);
TypedValue* fg_hphp_get_timers(ActRec* ar);
TypedValue* fg_hphp_output_global_state(ActRec* ar);
TypedValue* fg_hphp_instruction_counter(ActRec* ar);
TypedValue* fg_hphp_get_hardware_counters(ActRec* ar);
TypedValue* fg_hphp_set_hardware_events(ActRec* ar);
TypedValue* fg_hphp_clear_hardware_events(ActRec* ar);
TypedValue* fg_pdo_drivers(ActRec* ar);
TypedValue* fg_posix_access(ActRec* ar);
TypedValue* fg_posix_ctermid(ActRec* ar);
TypedValue* fg_posix_get_last_error(ActRec* ar);
TypedValue* fg_posix_getcwd(ActRec* ar);
TypedValue* fg_posix_getegid(ActRec* ar);
TypedValue* fg_posix_geteuid(ActRec* ar);
TypedValue* fg_posix_getgid(ActRec* ar);
TypedValue* fg_posix_getgrgid(ActRec* ar);
TypedValue* fg_posix_getgrnam(ActRec* ar);
TypedValue* fg_posix_getgroups(ActRec* ar);
TypedValue* fg_posix_getlogin(ActRec* ar);
TypedValue* fg_posix_getpgid(ActRec* ar);
TypedValue* fg_posix_getpgrp(ActRec* ar);
TypedValue* fg_posix_getpid(ActRec* ar);
TypedValue* fg_posix_getppid(ActRec* ar);
TypedValue* fg_posix_getpwnam(ActRec* ar);
TypedValue* fg_posix_getpwuid(ActRec* ar);
TypedValue* fg_posix_getrlimit(ActRec* ar);
TypedValue* fg_posix_getsid(ActRec* ar);
TypedValue* fg_posix_getuid(ActRec* ar);
TypedValue* fg_posix_initgroups(ActRec* ar);
TypedValue* fg_posix_isatty(ActRec* ar);
TypedValue* fg_posix_kill(ActRec* ar);
TypedValue* fg_posix_mkfifo(ActRec* ar);
TypedValue* fg_posix_mknod(ActRec* ar);
TypedValue* fg_posix_setegid(ActRec* ar);
TypedValue* fg_posix_seteuid(ActRec* ar);
TypedValue* fg_posix_setgid(ActRec* ar);
TypedValue* fg_posix_setpgid(ActRec* ar);
TypedValue* fg_posix_setsid(ActRec* ar);
TypedValue* fg_posix_setuid(ActRec* ar);
TypedValue* fg_posix_strerror(ActRec* ar);
TypedValue* fg_posix_times(ActRec* ar);
TypedValue* fg_posix_ttyname(ActRec* ar);
TypedValue* fg_posix_uname(ActRec* ar);
TypedValue* fg_preg_grep(ActRec* ar);
TypedValue* fg_preg_match(ActRec* ar);
TypedValue* fg_preg_match_all(ActRec* ar);
TypedValue* fg_preg_replace(ActRec* ar);
TypedValue* fg_preg_replace_callback(ActRec* ar);
TypedValue* fg_preg_split(ActRec* ar);
TypedValue* fg_preg_quote(ActRec* ar);
TypedValue* fg_preg_last_error(ActRec* ar);
TypedValue* fg_ereg_replace(ActRec* ar);
TypedValue* fg_eregi_replace(ActRec* ar);
TypedValue* fg_ereg(ActRec* ar);
TypedValue* fg_eregi(ActRec* ar);
TypedValue* fg_split(ActRec* ar);
TypedValue* fg_spliti(ActRec* ar);
TypedValue* fg_sql_regcase(ActRec* ar);
TypedValue* fg_pcntl_alarm(ActRec* ar);
TypedValue* fg_pcntl_exec(ActRec* ar);
TypedValue* fg_pcntl_fork(ActRec* ar);
TypedValue* fg_pcntl_getpriority(ActRec* ar);
TypedValue* fg_pcntl_setpriority(ActRec* ar);
TypedValue* fg_pcntl_signal(ActRec* ar);
TypedValue* fg_pcntl_wait(ActRec* ar);
TypedValue* fg_pcntl_waitpid(ActRec* ar);
TypedValue* fg_pcntl_wexitstatus(ActRec* ar);
TypedValue* fg_pcntl_wifexited(ActRec* ar);
TypedValue* fg_pcntl_wifsignaled(ActRec* ar);
TypedValue* fg_pcntl_wifstopped(ActRec* ar);
TypedValue* fg_pcntl_wstopsig(ActRec* ar);
TypedValue* fg_pcntl_wtermsig(ActRec* ar);
TypedValue* fg_pcntl_signal_dispatch(ActRec* ar);
TypedValue* fg_shell_exec(ActRec* ar);
TypedValue* fg_exec(ActRec* ar);
TypedValue* fg_passthru(ActRec* ar);
TypedValue* fg_system(ActRec* ar);
TypedValue* fg_proc_open(ActRec* ar);
TypedValue* fg_proc_terminate(ActRec* ar);
TypedValue* fg_proc_close(ActRec* ar);
TypedValue* fg_proc_get_status(ActRec* ar);
TypedValue* fg_proc_nice(ActRec* ar);
TypedValue* fg_escapeshellarg(ActRec* ar);
TypedValue* fg_escapeshellcmd(ActRec* ar);
TypedValue* fg_hphp_get_extension_info(ActRec* ar);
TypedValue* fg_hphp_get_method_info(ActRec* ar);
TypedValue* fg_hphp_get_closure_info(ActRec* ar);
TypedValue* fg_hphp_get_class_constant(ActRec* ar);
TypedValue* fg_hphp_get_class_info(ActRec* ar);
TypedValue* fg_hphp_get_function_info(ActRec* ar);
TypedValue* fg_hphp_invoke(ActRec* ar);
TypedValue* fg_hphp_invoke_method(ActRec* ar);
TypedValue* fg_hphp_instanceof(ActRec* ar);
TypedValue* fg_hphp_create_object(ActRec* ar);
TypedValue* fg_hphp_create_object_without_constructor(ActRec* ar);
TypedValue* fg_hphp_get_property(ActRec* ar);
TypedValue* fg_hphp_set_property(ActRec* ar);
TypedValue* fg_hphp_get_static_property(ActRec* ar);
TypedValue* fg_hphp_set_static_property(ActRec* ar);
TypedValue* fg_hphp_get_original_class_name(ActRec* ar);
TypedValue* fg_hphp_scalar_typehints_enabled(ActRec* ar);
TypedValue* fg_dangling_server_proxy_old_request(ActRec* ar);
TypedValue* fg_dangling_server_proxy_new_request(ActRec* ar);
TypedValue* fg_pagelet_server_is_enabled(ActRec* ar);
TypedValue* fg_pagelet_server_task_start(ActRec* ar);
TypedValue* fg_pagelet_server_task_status(ActRec* ar);
TypedValue* fg_pagelet_server_task_result(ActRec* ar);
TypedValue* fg_pagelet_server_flush(ActRec* ar);
TypedValue* fg_xbox_send_message(ActRec* ar);
TypedValue* fg_xbox_post_message(ActRec* ar);
TypedValue* fg_xbox_task_start(ActRec* ar);
TypedValue* fg_xbox_task_status(ActRec* ar);
TypedValue* fg_xbox_task_result(ActRec* ar);
TypedValue* fg_xbox_process_call_message(ActRec* ar);
TypedValue* fg_xbox_get_thread_timeout(ActRec* ar);
TypedValue* fg_xbox_set_thread_timeout(ActRec* ar);
TypedValue* fg_xbox_schedule_thread_reset(ActRec* ar);
TypedValue* fg_xbox_get_thread_time(ActRec* ar);
TypedValue* fg_session_set_cookie_params(ActRec* ar);
TypedValue* fg_session_get_cookie_params(ActRec* ar);
TypedValue* fg_session_name(ActRec* ar);
TypedValue* fg_session_module_name(ActRec* ar);
TypedValue* fg_session_set_save_handler(ActRec* ar);
TypedValue* fg_session_save_path(ActRec* ar);
TypedValue* fg_session_id(ActRec* ar);
TypedValue* fg_session_regenerate_id(ActRec* ar);
TypedValue* fg_session_cache_limiter(ActRec* ar);
TypedValue* fg_session_cache_expire(ActRec* ar);
TypedValue* fg_session_encode(ActRec* ar);
TypedValue* fg_session_decode(ActRec* ar);
TypedValue* fg_session_start(ActRec* ar);
TypedValue* fg_session_destroy(ActRec* ar);
TypedValue* fg_session_unset(ActRec* ar);
TypedValue* fg_session_commit(ActRec* ar);
TypedValue* fg_session_write_close(ActRec* ar);
TypedValue* fg_session_register(ActRec* ar);
TypedValue* fg_session_unregister(ActRec* ar);
TypedValue* fg_session_is_registered(ActRec* ar);
TypedValue* fg_simplexml_load_string(ActRec* ar);
TypedValue* fg_simplexml_load_file(ActRec* ar);
TypedValue* fg_libxml_get_errors(ActRec* ar);
TypedValue* fg_libxml_get_last_error(ActRec* ar);
TypedValue* fg_libxml_clear_errors(ActRec* ar);
TypedValue* fg_libxml_use_internal_errors(ActRec* ar);
TypedValue* fg_libxml_set_streams_context(ActRec* ar);
TypedValue* fg_libxml_disable_entity_loader(ActRec* ar);
TypedValue* fg_use_soap_error_handler(ActRec* ar);
TypedValue* fg_is_soap_fault(ActRec* ar);
TypedValue* fg__soap_active_version(ActRec* ar);
TypedValue* fg_socket_create(ActRec* ar);
TypedValue* fg_socket_create_listen(ActRec* ar);
TypedValue* fg_socket_create_pair(ActRec* ar);
TypedValue* fg_socket_get_option(ActRec* ar);
TypedValue* fg_socket_getpeername(ActRec* ar);
TypedValue* fg_socket_getsockname(ActRec* ar);
TypedValue* fg_socket_set_block(ActRec* ar);
TypedValue* fg_socket_set_nonblock(ActRec* ar);
TypedValue* fg_socket_set_option(ActRec* ar);
TypedValue* fg_socket_connect(ActRec* ar);
TypedValue* fg_socket_bind(ActRec* ar);
TypedValue* fg_socket_listen(ActRec* ar);
TypedValue* fg_socket_select(ActRec* ar);
TypedValue* fg_socket_server(ActRec* ar);
TypedValue* fg_socket_accept(ActRec* ar);
TypedValue* fg_socket_read(ActRec* ar);
TypedValue* fg_socket_write(ActRec* ar);
TypedValue* fg_socket_send(ActRec* ar);
TypedValue* fg_socket_sendto(ActRec* ar);
TypedValue* fg_socket_recv(ActRec* ar);
TypedValue* fg_socket_recvfrom(ActRec* ar);
TypedValue* fg_socket_shutdown(ActRec* ar);
TypedValue* fg_socket_close(ActRec* ar);
TypedValue* fg_socket_strerror(ActRec* ar);
TypedValue* fg_socket_last_error(ActRec* ar);
TypedValue* fg_socket_clear_error(ActRec* ar);
TypedValue* fg_getaddrinfo(ActRec* ar);
TypedValue* fg_spl_classes(ActRec* ar);
TypedValue* fg_spl_object_hash(ActRec* ar);
TypedValue* fg_hphp_object_pointer(ActRec* ar);
TypedValue* fg_hphp_get_this(ActRec* ar);
TypedValue* fg_class_implements(ActRec* ar);
TypedValue* fg_class_parents(ActRec* ar);
TypedValue* fg_class_uses(ActRec* ar);
TypedValue* fg_iterator_apply(ActRec* ar);
TypedValue* fg_iterator_count(ActRec* ar);
TypedValue* fg_iterator_to_array(ActRec* ar);
TypedValue* fg_spl_autoload_call(ActRec* ar);
TypedValue* fg_spl_autoload_extensions(ActRec* ar);
TypedValue* fg_spl_autoload_functions(ActRec* ar);
TypedValue* fg_spl_autoload_register(ActRec* ar);
TypedValue* fg_spl_autoload_unregister(ActRec* ar);
TypedValue* fg_spl_autoload(ActRec* ar);
TypedValue* fg_hphp_splfileinfo___construct(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getatime(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getbasename(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getctime(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getfileinfo(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getfilename(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getgroup(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getinode(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getlinktarget(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getmtime(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getowner(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getpath(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getpathinfo(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getpathname(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getperms(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getrealpath(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_getsize(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_gettype(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_isdir(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_isexecutable(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_isfile(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_islink(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_isreadable(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_iswritable(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_openfile(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_setfileclass(ActRec* ar);
TypedValue* fg_hphp_splfileinfo_setinfoclass(ActRec* ar);
TypedValue* fg_hphp_splfileinfo___tostring(ActRec* ar);
TypedValue* fg_hphp_splfileobject___construct(ActRec* ar);
TypedValue* fg_hphp_splfileobject_current(ActRec* ar);
TypedValue* fg_hphp_splfileobject_eof(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fflush(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fgetc(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fgetcsv(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fgets(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fgetss(ActRec* ar);
TypedValue* fg_hphp_splfileobject_flock(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fpassthru(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fscanf(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fseek(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fstat(ActRec* ar);
TypedValue* fg_hphp_splfileobject_ftell(ActRec* ar);
TypedValue* fg_hphp_splfileobject_ftruncate(ActRec* ar);
TypedValue* fg_hphp_splfileobject_fwrite(ActRec* ar);
TypedValue* fg_hphp_splfileobject_getcvscontrol(ActRec* ar);
TypedValue* fg_hphp_splfileobject_getflags(ActRec* ar);
TypedValue* fg_hphp_splfileobject_getmaxlinelen(ActRec* ar);
TypedValue* fg_hphp_splfileobject_key(ActRec* ar);
TypedValue* fg_hphp_splfileobject_next(ActRec* ar);
TypedValue* fg_hphp_splfileobject_rewind(ActRec* ar);
TypedValue* fg_hphp_splfileobject_valid(ActRec* ar);
TypedValue* fg_hphp_splfileobject_seek(ActRec* ar);
TypedValue* fg_hphp_splfileobject_setcsvcontrol(ActRec* ar);
TypedValue* fg_hphp_splfileobject_setflags(ActRec* ar);
TypedValue* fg_hphp_splfileobject_setmaxlinelen(ActRec* ar);
TypedValue* fg_stream_context_create(ActRec* ar);
TypedValue* fg_stream_context_get_default(ActRec* ar);
TypedValue* fg_stream_context_get_options(ActRec* ar);
TypedValue* fg_stream_context_set_option(ActRec* ar);
TypedValue* fg_stream_context_set_param(ActRec* ar);
TypedValue* fg_stream_copy_to_stream(ActRec* ar);
TypedValue* fg_stream_encoding(ActRec* ar);
TypedValue* fg_stream_bucket_append(ActRec* ar);
TypedValue* fg_stream_bucket_prepend(ActRec* ar);
TypedValue* fg_stream_bucket_make_writeable(ActRec* ar);
TypedValue* fg_stream_bucket_new(ActRec* ar);
TypedValue* fg_stream_filter_register(ActRec* ar);
TypedValue* fg_stream_filter_remove(ActRec* ar);
TypedValue* fg_stream_filter_append(ActRec* ar);
TypedValue* fg_stream_filter_prepend(ActRec* ar);
TypedValue* fg_stream_get_contents(ActRec* ar);
TypedValue* fg_stream_get_filters(ActRec* ar);
TypedValue* fg_stream_get_line(ActRec* ar);
TypedValue* fg_stream_get_meta_data(ActRec* ar);
TypedValue* fg_stream_get_transports(ActRec* ar);
TypedValue* fg_stream_get_wrappers(ActRec* ar);
TypedValue* fg_stream_register_wrapper(ActRec* ar);
TypedValue* fg_stream_wrapper_register(ActRec* ar);
TypedValue* fg_stream_wrapper_restore(ActRec* ar);
TypedValue* fg_stream_wrapper_unregister(ActRec* ar);
TypedValue* fg_stream_resolve_include_path(ActRec* ar);
TypedValue* fg_stream_select(ActRec* ar);
TypedValue* fg_stream_set_blocking(ActRec* ar);
TypedValue* fg_stream_set_timeout(ActRec* ar);
TypedValue* fg_stream_set_write_buffer(ActRec* ar);
TypedValue* fg_set_file_buffer(ActRec* ar);
TypedValue* fg_stream_socket_accept(ActRec* ar);
TypedValue* fg_stream_socket_server(ActRec* ar);
TypedValue* fg_stream_socket_client(ActRec* ar);
TypedValue* fg_stream_socket_enable_crypto(ActRec* ar);
TypedValue* fg_stream_socket_get_name(ActRec* ar);
TypedValue* fg_stream_socket_pair(ActRec* ar);
TypedValue* fg_stream_socket_recvfrom(ActRec* ar);
TypedValue* fg_stream_socket_sendto(ActRec* ar);
TypedValue* fg_stream_socket_shutdown(ActRec* ar);
TypedValue* fg_addcslashes(ActRec* ar);
TypedValue* fg_stripcslashes(ActRec* ar);
TypedValue* fg_addslashes(ActRec* ar);
TypedValue* fg_stripslashes(ActRec* ar);
TypedValue* fg_bin2hex(ActRec* ar);
TypedValue* fg_hex2bin(ActRec* ar);
TypedValue* fg_nl2br(ActRec* ar);
TypedValue* fg_quotemeta(ActRec* ar);
TypedValue* fg_str_shuffle(ActRec* ar);
TypedValue* fg_strrev(ActRec* ar);
TypedValue* fg_strtolower(ActRec* ar);
TypedValue* fg_strtoupper(ActRec* ar);
TypedValue* fg_ucfirst(ActRec* ar);
TypedValue* fg_lcfirst(ActRec* ar);
TypedValue* fg_ucwords(ActRec* ar);
TypedValue* fg_strip_tags(ActRec* ar);
TypedValue* fg_trim(ActRec* ar);
TypedValue* fg_ltrim(ActRec* ar);
TypedValue* fg_rtrim(ActRec* ar);
TypedValue* fg_chop(ActRec* ar);
TypedValue* fg_explode(ActRec* ar);
TypedValue* fg_implode(ActRec* ar);
TypedValue* fg_join(ActRec* ar);
TypedValue* fg_str_split(ActRec* ar);
TypedValue* fg_chunk_split(ActRec* ar);
TypedValue* fg_strtok(ActRec* ar);
TypedValue* fg_str_replace(ActRec* ar);
TypedValue* fg_str_ireplace(ActRec* ar);
TypedValue* fg_substr_replace(ActRec* ar);
TypedValue* fg_substr(ActRec* ar);
TypedValue* fg_str_pad(ActRec* ar);
TypedValue* fg_str_repeat(ActRec* ar);
TypedValue* fg_wordwrap(ActRec* ar);
TypedValue* fg_html_entity_decode(ActRec* ar);
TypedValue* fg_htmlentities(ActRec* ar);
TypedValue* fg_htmlspecialchars_decode(ActRec* ar);
TypedValue* fg_htmlspecialchars(ActRec* ar);
TypedValue* fg_fb_htmlspecialchars(ActRec* ar);
TypedValue* fg_quoted_printable_encode(ActRec* ar);
TypedValue* fg_quoted_printable_decode(ActRec* ar);
TypedValue* fg_convert_uudecode(ActRec* ar);
TypedValue* fg_convert_uuencode(ActRec* ar);
TypedValue* fg_str_rot13(ActRec* ar);
TypedValue* fg_crc32(ActRec* ar);
TypedValue* fg_crypt(ActRec* ar);
TypedValue* fg_md5(ActRec* ar);
TypedValue* fg_sha1(ActRec* ar);
TypedValue* fg_strtr(ActRec* ar);
TypedValue* fg_convert_cyr_string(ActRec* ar);
TypedValue* fg_get_html_translation_table(ActRec* ar);
TypedValue* fg_hebrev(ActRec* ar);
TypedValue* fg_hebrevc(ActRec* ar);
TypedValue* fg_setlocale(ActRec* ar);
TypedValue* fg_localeconv(ActRec* ar);
TypedValue* fg_nl_langinfo(ActRec* ar);
TypedValue* fg_printf(ActRec* ar);
TypedValue* fg_vprintf(ActRec* ar);
TypedValue* fg_sprintf(ActRec* ar);
TypedValue* fg_vsprintf(ActRec* ar);
TypedValue* fg_sscanf(ActRec* ar);
TypedValue* fg_chr(ActRec* ar);
TypedValue* fg_ord(ActRec* ar);
TypedValue* fg_money_format(ActRec* ar);
TypedValue* fg_number_format(ActRec* ar);
TypedValue* fg_strcmp(ActRec* ar);
TypedValue* fg_strncmp(ActRec* ar);
TypedValue* fg_strnatcmp(ActRec* ar);
TypedValue* fg_strcasecmp(ActRec* ar);
TypedValue* fg_strncasecmp(ActRec* ar);
TypedValue* fg_strnatcasecmp(ActRec* ar);
TypedValue* fg_strcoll(ActRec* ar);
TypedValue* fg_substr_compare(ActRec* ar);
TypedValue* fg_strchr(ActRec* ar);
TypedValue* fg_strrchr(ActRec* ar);
TypedValue* fg_strstr(ActRec* ar);
TypedValue* fg_stristr(ActRec* ar);
TypedValue* fg_strpbrk(ActRec* ar);
TypedValue* fg_strpos(ActRec* ar);
TypedValue* fg_stripos(ActRec* ar);
TypedValue* fg_strrpos(ActRec* ar);
TypedValue* fg_strripos(ActRec* ar);
TypedValue* fg_substr_count(ActRec* ar);
TypedValue* fg_strspn(ActRec* ar);
TypedValue* fg_strcspn(ActRec* ar);
TypedValue* fg_strlen(ActRec* ar);
TypedValue* fg_count_chars(ActRec* ar);
TypedValue* fg_str_word_count(ActRec* ar);
TypedValue* fg_levenshtein(ActRec* ar);
TypedValue* fg_similar_text(ActRec* ar);
TypedValue* fg_soundex(ActRec* ar);
TypedValue* fg_metaphone(ActRec* ar);
TypedValue* fg_parse_str(ActRec* ar);
TypedValue* fg_hphp_is_service_thread(ActRec* ar);
TypedValue* fg_hphp_service_thread_started(ActRec* ar);
TypedValue* fg_hphp_service_thread_stopped(ActRec* ar);
TypedValue* fg_hphp_thread_is_warmup_enabled(ActRec* ar);
TypedValue* fg_hphp_thread_set_warmup_enabled(ActRec* ar);
TypedValue* fg_hphp_get_thread_id(ActRec* ar);
TypedValue* fg_hphp_gettid(ActRec* ar);
TypedValue* fg_thrift_protocol_write_binary(ActRec* ar);
TypedValue* fg_thrift_protocol_read_binary(ActRec* ar);
TypedValue* fg_thrift_protocol_set_compact_version(ActRec* ar);
TypedValue* fg_thrift_protocol_write_compact(ActRec* ar);
TypedValue* fg_thrift_protocol_read_compact(ActRec* ar);
TypedValue* fg_base64_decode(ActRec* ar);
TypedValue* fg_base64_encode(ActRec* ar);
TypedValue* fg_get_headers(ActRec* ar);
TypedValue* fg_get_meta_tags(ActRec* ar);
TypedValue* fg_http_build_query(ActRec* ar);
TypedValue* fg_parse_url(ActRec* ar);
TypedValue* fg_rawurldecode(ActRec* ar);
TypedValue* fg_rawurlencode(ActRec* ar);
TypedValue* fg_urldecode(ActRec* ar);
TypedValue* fg_urlencode(ActRec* ar);
TypedValue* fg_is_bool(ActRec* ar);
TypedValue* fg_is_int(ActRec* ar);
TypedValue* fg_is_integer(ActRec* ar);
TypedValue* fg_is_long(ActRec* ar);
TypedValue* fg_is_double(ActRec* ar);
TypedValue* fg_is_float(ActRec* ar);
TypedValue* fg_is_numeric(ActRec* ar);
TypedValue* fg_is_real(ActRec* ar);
TypedValue* fg_is_string(ActRec* ar);
TypedValue* fg_is_scalar(ActRec* ar);
TypedValue* fg_is_array(ActRec* ar);
TypedValue* fg_is_object(ActRec* ar);
TypedValue* fg_is_resource(ActRec* ar);
TypedValue* fg_is_null(ActRec* ar);
TypedValue* fg_gettype(ActRec* ar);
TypedValue* fg_get_resource_type(ActRec* ar);
TypedValue* fg_intval(ActRec* ar);
TypedValue* fg_doubleval(ActRec* ar);
TypedValue* fg_floatval(ActRec* ar);
TypedValue* fg_strval(ActRec* ar);
TypedValue* fg_settype(ActRec* ar);
TypedValue* fg_print_r(ActRec* ar);
TypedValue* fg_var_export(ActRec* ar);
TypedValue* fg_var_dump(ActRec* ar);
TypedValue* fg_debug_zval_dump(ActRec* ar);
TypedValue* fg_serialize(ActRec* ar);
TypedValue* fg_unserialize(ActRec* ar);
TypedValue* fg_get_defined_vars(ActRec* ar);
TypedValue* fg_import_request_variables(ActRec* ar);
TypedValue* fg_extract(ActRec* ar);
TypedValue* fg_xml_parser_create(ActRec* ar);
TypedValue* fg_xml_parser_free(ActRec* ar);
TypedValue* fg_xml_parse(ActRec* ar);
TypedValue* fg_xml_parse_into_struct(ActRec* ar);
TypedValue* fg_xml_parser_create_ns(ActRec* ar);
TypedValue* fg_xml_parser_get_option(ActRec* ar);
TypedValue* fg_xml_parser_set_option(ActRec* ar);
TypedValue* fg_xml_set_character_data_handler(ActRec* ar);
TypedValue* fg_xml_set_default_handler(ActRec* ar);
TypedValue* fg_xml_set_element_handler(ActRec* ar);
TypedValue* fg_xml_set_processing_instruction_handler(ActRec* ar);
TypedValue* fg_xml_set_start_namespace_decl_handler(ActRec* ar);
TypedValue* fg_xml_set_end_namespace_decl_handler(ActRec* ar);
TypedValue* fg_xml_set_unparsed_entity_decl_handler(ActRec* ar);
TypedValue* fg_xml_set_external_entity_ref_handler(ActRec* ar);
TypedValue* fg_xml_set_notation_decl_handler(ActRec* ar);
TypedValue* fg_xml_set_object(ActRec* ar);
TypedValue* fg_xml_get_current_byte_index(ActRec* ar);
TypedValue* fg_xml_get_current_column_number(ActRec* ar);
TypedValue* fg_xml_get_current_line_number(ActRec* ar);
TypedValue* fg_xml_get_error_code(ActRec* ar);
TypedValue* fg_xml_error_string(ActRec* ar);
TypedValue* fg_utf8_decode(ActRec* ar);
TypedValue* fg_utf8_encode(ActRec* ar);
TypedValue* fg_xmlwriter_open_memory(ActRec* ar);
TypedValue* fg_xmlwriter_open_uri(ActRec* ar);
TypedValue* fg_xmlwriter_set_indent_string(ActRec* ar);
TypedValue* fg_xmlwriter_set_indent(ActRec* ar);
TypedValue* fg_xmlwriter_start_document(ActRec* ar);
TypedValue* fg_xmlwriter_start_element(ActRec* ar);
TypedValue* fg_xmlwriter_start_element_ns(ActRec* ar);
TypedValue* fg_xmlwriter_write_element_ns(ActRec* ar);
TypedValue* fg_xmlwriter_write_element(ActRec* ar);
TypedValue* fg_xmlwriter_end_element(ActRec* ar);
TypedValue* fg_xmlwriter_full_end_element(ActRec* ar);
TypedValue* fg_xmlwriter_start_attribute_ns(ActRec* ar);
TypedValue* fg_xmlwriter_start_attribute(ActRec* ar);
TypedValue* fg_xmlwriter_write_attribute_ns(ActRec* ar);
TypedValue* fg_xmlwriter_write_attribute(ActRec* ar);
TypedValue* fg_xmlwriter_end_attribute(ActRec* ar);
TypedValue* fg_xmlwriter_start_cdata(ActRec* ar);
TypedValue* fg_xmlwriter_write_cdata(ActRec* ar);
TypedValue* fg_xmlwriter_end_cdata(ActRec* ar);
TypedValue* fg_xmlwriter_start_comment(ActRec* ar);
TypedValue* fg_xmlwriter_write_comment(ActRec* ar);
TypedValue* fg_xmlwriter_end_comment(ActRec* ar);
TypedValue* fg_xmlwriter_end_document(ActRec* ar);
TypedValue* fg_xmlwriter_start_pi(ActRec* ar);
TypedValue* fg_xmlwriter_write_pi(ActRec* ar);
TypedValue* fg_xmlwriter_end_pi(ActRec* ar);
TypedValue* fg_xmlwriter_text(ActRec* ar);
TypedValue* fg_xmlwriter_write_raw(ActRec* ar);
TypedValue* fg_xmlwriter_start_dtd(ActRec* ar);
TypedValue* fg_xmlwriter_write_dtd(ActRec* ar);
TypedValue* fg_xmlwriter_start_dtd_element(ActRec* ar);
TypedValue* fg_xmlwriter_write_dtd_element(ActRec* ar);
TypedValue* fg_xmlwriter_end_dtd_element(ActRec* ar);
TypedValue* fg_xmlwriter_start_dtd_attlist(ActRec* ar);
TypedValue* fg_xmlwriter_write_dtd_attlist(ActRec* ar);
TypedValue* fg_xmlwriter_end_dtd_attlist(ActRec* ar);
TypedValue* fg_xmlwriter_start_dtd_entity(ActRec* ar);
TypedValue* fg_xmlwriter_write_dtd_entity(ActRec* ar);
TypedValue* fg_xmlwriter_end_dtd_entity(ActRec* ar);
TypedValue* fg_xmlwriter_end_dtd(ActRec* ar);
TypedValue* fg_xmlwriter_flush(ActRec* ar);
TypedValue* fg_xmlwriter_output_memory(ActRec* ar);
TypedValue* fg_readgzfile(ActRec* ar);
TypedValue* fg_gzfile(ActRec* ar);
TypedValue* fg_gzcompress(ActRec* ar);
TypedValue* fg_gzuncompress(ActRec* ar);
TypedValue* fg_gzdeflate(ActRec* ar);
TypedValue* fg_gzinflate(ActRec* ar);
TypedValue* fg_gzencode(ActRec* ar);
TypedValue* fg_gzdecode(ActRec* ar);
TypedValue* fg_zlib_get_coding_type(ActRec* ar);
TypedValue* fg_gzopen(ActRec* ar);
TypedValue* fg_gzclose(ActRec* ar);
TypedValue* fg_gzrewind(ActRec* ar);
TypedValue* fg_gzeof(ActRec* ar);
TypedValue* fg_gzgetc(ActRec* ar);
TypedValue* fg_gzgets(ActRec* ar);
TypedValue* fg_gzgetss(ActRec* ar);
TypedValue* fg_gzread(ActRec* ar);
TypedValue* fg_gzpassthru(ActRec* ar);
TypedValue* fg_gzseek(ActRec* ar);
TypedValue* fg_gztell(ActRec* ar);
TypedValue* fg_gzwrite(ActRec* ar);
TypedValue* fg_gzputs(ActRec* ar);
TypedValue* fg_qlzcompress(ActRec* ar);
TypedValue* fg_qlzuncompress(ActRec* ar);
TypedValue* fg_sncompress(ActRec* ar);
TypedValue* fg_snuncompress(ActRec* ar);
TypedValue* fg_nzcompress(ActRec* ar);
TypedValue* fg_nzuncompress(ActRec* ar);
TypedValue* fg_lz4compress(ActRec* ar);
TypedValue* fg_lz4hccompress(ActRec* ar);
TypedValue* fg_lz4uncompress(ActRec* ar);
TypedValue* tg_10WaitHandle___construct(ActRec* ar);
TypedValue* tg_10WaitHandle_getWaitHandle(ActRec* ar);
TypedValue* tg_10WaitHandle_import(ActRec* ar);
TypedValue* tg_10WaitHandle_join(ActRec* ar);
TypedValue* tg_10WaitHandle_isFinished(ActRec* ar);
TypedValue* tg_10WaitHandle_isSucceeded(ActRec* ar);
TypedValue* tg_10WaitHandle_isFailed(ActRec* ar);
TypedValue* tg_10WaitHandle_getID(ActRec* ar);
TypedValue* tg_10WaitHandle_getName(ActRec* ar);
TypedValue* tg_10WaitHandle_getExceptionIfFailed(ActRec* ar);
TypedValue* tg_16StaticWaitHandle___construct(ActRec* ar);
VM::Instance* new_StaticResultWaitHandle_Instance(VM::Class*);
TypedValue* tg_22StaticResultWaitHandle___construct(ActRec* ar);
TypedValue* tg_22StaticResultWaitHandle_create(ActRec* ar);
VM::Instance* new_StaticExceptionWaitHandle_Instance(VM::Class*);
TypedValue* tg_25StaticExceptionWaitHandle___construct(ActRec* ar);
TypedValue* tg_25StaticExceptionWaitHandle_create(ActRec* ar);
TypedValue* tg_18WaitableWaitHandle___construct(ActRec* ar);
TypedValue* tg_18WaitableWaitHandle_getContextIdx(ActRec* ar);
TypedValue* tg_18WaitableWaitHandle_getCreator(ActRec* ar);
TypedValue* tg_18WaitableWaitHandle_getParents(ActRec* ar);
TypedValue* tg_18WaitableWaitHandle_getStackTrace(ActRec* ar);
TypedValue* tg_19BlockableWaitHandle___construct(ActRec* ar);
VM::Instance* new_ContinuationWaitHandle_Instance(VM::Class*);
TypedValue* tg_22ContinuationWaitHandle___construct(ActRec* ar);
TypedValue* tg_22ContinuationWaitHandle_start(ActRec* ar);
TypedValue* tg_22ContinuationWaitHandle_getPrivData(ActRec* ar);
TypedValue* tg_22ContinuationWaitHandle_setPrivData(ActRec* ar);
VM::Instance* new_GenArrayWaitHandle_Instance(VM::Class*);
TypedValue* tg_18GenArrayWaitHandle___construct(ActRec* ar);
TypedValue* tg_18GenArrayWaitHandle_create(ActRec* ar);
VM::Instance* new_SetResultToRefWaitHandle_Instance(VM::Class*);
TypedValue* tg_24SetResultToRefWaitHandle___construct(ActRec* ar);
TypedValue* tg_24SetResultToRefWaitHandle_create(ActRec* ar);
VM::Instance* new_RescheduleWaitHandle_Instance(VM::Class*);
TypedValue* tg_20RescheduleWaitHandle___construct(ActRec* ar);
TypedValue* tg_20RescheduleWaitHandle_create(ActRec* ar);
VM::Instance* new_Closure_Instance(VM::Class*);
TypedValue* tg_7Closure___construct(ActRec* ar);
VM::Instance* new_DummyClosure_Instance(VM::Class*);
TypedValue* tg_12DummyClosure___construct(ActRec* ar);
VM::Instance* new_Vector_Instance(VM::Class*);
TypedValue* tg_6Vector___construct(ActRec* ar);
TypedValue* tg_6Vector_isEmpty(ActRec* ar);
TypedValue* tg_6Vector_count(ActRec* ar);
TypedValue* tg_6Vector_items(ActRec* ar);
TypedValue* tg_6Vector_keys(ActRec* ar);
TypedValue* tg_6Vector_view(ActRec* ar);
TypedValue* tg_6Vector_kvzip(ActRec* ar);
TypedValue* tg_6Vector_at(ActRec* ar);
TypedValue* tg_6Vector_get(ActRec* ar);
TypedValue* tg_6Vector_set(ActRec* ar);
TypedValue* tg_6Vector_setAll(ActRec* ar);
TypedValue* tg_6Vector_put(ActRec* ar);
TypedValue* tg_6Vector_clear(ActRec* ar);
TypedValue* tg_6Vector_contains(ActRec* ar);
TypedValue* tg_6Vector_containsKey(ActRec* ar);
TypedValue* tg_6Vector_removeKey(ActRec* ar);
TypedValue* tg_6Vector_append(ActRec* ar);
TypedValue* tg_6Vector_add(ActRec* ar);
TypedValue* tg_6Vector_addAll(ActRec* ar);
TypedValue* tg_6Vector_pop(ActRec* ar);
TypedValue* tg_6Vector_resize(ActRec* ar);
TypedValue* tg_6Vector_toArray(ActRec* ar);
TypedValue* tg_6Vector_getIterator(ActRec* ar);
TypedValue* tg_6Vector_map(ActRec* ar);
TypedValue* tg_6Vector_filter(ActRec* ar);
TypedValue* tg_6Vector_zip(ActRec* ar);
TypedValue* tg_6Vector_sort(ActRec* ar);
TypedValue* tg_6Vector_reverse(ActRec* ar);
TypedValue* tg_6Vector_splice(ActRec* ar);
TypedValue* tg_6Vector_linearSearch(ActRec* ar);
TypedValue* tg_6Vector_shuffle(ActRec* ar);
TypedValue* tg_6Vector___toString(ActRec* ar);
TypedValue* tg_6Vector___get(ActRec* ar);
TypedValue* tg_6Vector___set(ActRec* ar);
TypedValue* tg_6Vector___isset(ActRec* ar);
TypedValue* tg_6Vector___unset(ActRec* ar);
TypedValue* tg_6Vector_fromItems(ActRec* ar);
TypedValue* tg_6Vector_fromArray(ActRec* ar);
TypedValue* tg_6Vector_fromVector(ActRec* ar);
TypedValue* tg_6Vector_slice(ActRec* ar);
VM::Instance* new_VectorIterator_Instance(VM::Class*);
TypedValue* tg_14VectorIterator___construct(ActRec* ar);
TypedValue* tg_14VectorIterator_current(ActRec* ar);
TypedValue* tg_14VectorIterator_key(ActRec* ar);
TypedValue* tg_14VectorIterator_valid(ActRec* ar);
TypedValue* tg_14VectorIterator_next(ActRec* ar);
TypedValue* tg_14VectorIterator_rewind(ActRec* ar);
VM::Instance* new_Map_Instance(VM::Class*);
TypedValue* tg_3Map___construct(ActRec* ar);
TypedValue* tg_3Map_isEmpty(ActRec* ar);
TypedValue* tg_3Map_count(ActRec* ar);
TypedValue* tg_3Map_items(ActRec* ar);
TypedValue* tg_3Map_keys(ActRec* ar);
TypedValue* tg_3Map_view(ActRec* ar);
TypedValue* tg_3Map_kvzip(ActRec* ar);
TypedValue* tg_3Map_at(ActRec* ar);
TypedValue* tg_3Map_get(ActRec* ar);
TypedValue* tg_3Map_set(ActRec* ar);
TypedValue* tg_3Map_setAll(ActRec* ar);
TypedValue* tg_3Map_put(ActRec* ar);
TypedValue* tg_3Map_clear(ActRec* ar);
TypedValue* tg_3Map_contains(ActRec* ar);
TypedValue* tg_3Map_containsKey(ActRec* ar);
TypedValue* tg_3Map_remove(ActRec* ar);
TypedValue* tg_3Map_removeKey(ActRec* ar);
TypedValue* tg_3Map_discard(ActRec* ar);
TypedValue* tg_3Map_add(ActRec* ar);
TypedValue* tg_3Map_addAll(ActRec* ar);
TypedValue* tg_3Map_toArray(ActRec* ar);
TypedValue* tg_3Map_copyAsArray(ActRec* ar);
TypedValue* tg_3Map_toKeysArray(ActRec* ar);
TypedValue* tg_3Map_values(ActRec* ar);
TypedValue* tg_3Map_toValuesArray(ActRec* ar);
TypedValue* tg_3Map_updateFromArray(ActRec* ar);
TypedValue* tg_3Map_updateFromIterable(ActRec* ar);
TypedValue* tg_3Map_differenceByKey(ActRec* ar);
TypedValue* tg_3Map_getIterator(ActRec* ar);
TypedValue* tg_3Map_map(ActRec* ar);
TypedValue* tg_3Map_filter(ActRec* ar);
TypedValue* tg_3Map_zip(ActRec* ar);
TypedValue* tg_3Map___toString(ActRec* ar);
TypedValue* tg_3Map___get(ActRec* ar);
TypedValue* tg_3Map___set(ActRec* ar);
TypedValue* tg_3Map___isset(ActRec* ar);
TypedValue* tg_3Map___unset(ActRec* ar);
TypedValue* tg_3Map_fromItems(ActRec* ar);
TypedValue* tg_3Map_fromArray(ActRec* ar);
TypedValue* tg_3Map_fromIterable(ActRec* ar);
VM::Instance* new_MapIterator_Instance(VM::Class*);
TypedValue* tg_11MapIterator___construct(ActRec* ar);
TypedValue* tg_11MapIterator_current(ActRec* ar);
TypedValue* tg_11MapIterator_key(ActRec* ar);
TypedValue* tg_11MapIterator_valid(ActRec* ar);
TypedValue* tg_11MapIterator_next(ActRec* ar);
TypedValue* tg_11MapIterator_rewind(ActRec* ar);
VM::Instance* new_StableMap_Instance(VM::Class*);
TypedValue* tg_9StableMap___construct(ActRec* ar);
TypedValue* tg_9StableMap_isEmpty(ActRec* ar);
TypedValue* tg_9StableMap_count(ActRec* ar);
TypedValue* tg_9StableMap_items(ActRec* ar);
TypedValue* tg_9StableMap_keys(ActRec* ar);
TypedValue* tg_9StableMap_view(ActRec* ar);
TypedValue* tg_9StableMap_kvzip(ActRec* ar);
TypedValue* tg_9StableMap_at(ActRec* ar);
TypedValue* tg_9StableMap_get(ActRec* ar);
TypedValue* tg_9StableMap_set(ActRec* ar);
TypedValue* tg_9StableMap_setAll(ActRec* ar);
TypedValue* tg_9StableMap_put(ActRec* ar);
TypedValue* tg_9StableMap_clear(ActRec* ar);
TypedValue* tg_9StableMap_contains(ActRec* ar);
TypedValue* tg_9StableMap_containsKey(ActRec* ar);
TypedValue* tg_9StableMap_remove(ActRec* ar);
TypedValue* tg_9StableMap_removeKey(ActRec* ar);
TypedValue* tg_9StableMap_discard(ActRec* ar);
TypedValue* tg_9StableMap_add(ActRec* ar);
TypedValue* tg_9StableMap_addAll(ActRec* ar);
TypedValue* tg_9StableMap_toArray(ActRec* ar);
TypedValue* tg_9StableMap_copyAsArray(ActRec* ar);
TypedValue* tg_9StableMap_toKeysArray(ActRec* ar);
TypedValue* tg_9StableMap_values(ActRec* ar);
TypedValue* tg_9StableMap_toValuesArray(ActRec* ar);
TypedValue* tg_9StableMap_updateFromArray(ActRec* ar);
TypedValue* tg_9StableMap_updateFromIterable(ActRec* ar);
TypedValue* tg_9StableMap_differenceByKey(ActRec* ar);
TypedValue* tg_9StableMap_getIterator(ActRec* ar);
TypedValue* tg_9StableMap_map(ActRec* ar);
TypedValue* tg_9StableMap_filter(ActRec* ar);
TypedValue* tg_9StableMap_zip(ActRec* ar);
TypedValue* tg_9StableMap___get(ActRec* ar);
TypedValue* tg_9StableMap___set(ActRec* ar);
TypedValue* tg_9StableMap___isset(ActRec* ar);
TypedValue* tg_9StableMap___unset(ActRec* ar);
TypedValue* tg_9StableMap_fromItems(ActRec* ar);
TypedValue* tg_9StableMap___toString(ActRec* ar);
TypedValue* tg_9StableMap_fromArray(ActRec* ar);
TypedValue* tg_9StableMap_fromIterable(ActRec* ar);
VM::Instance* new_StableMapIterator_Instance(VM::Class*);
TypedValue* tg_17StableMapIterator___construct(ActRec* ar);
TypedValue* tg_17StableMapIterator_current(ActRec* ar);
TypedValue* tg_17StableMapIterator_key(ActRec* ar);
TypedValue* tg_17StableMapIterator_valid(ActRec* ar);
TypedValue* tg_17StableMapIterator_next(ActRec* ar);
TypedValue* tg_17StableMapIterator_rewind(ActRec* ar);
VM::Instance* new_Pair_Instance(VM::Class*);
TypedValue* tg_4Pair___construct(ActRec* ar);
TypedValue* tg_4Pair_isEmpty(ActRec* ar);
TypedValue* tg_4Pair_count(ActRec* ar);
TypedValue* tg_4Pair_items(ActRec* ar);
TypedValue* tg_4Pair_keys(ActRec* ar);
TypedValue* tg_4Pair_view(ActRec* ar);
TypedValue* tg_4Pair_kvzip(ActRec* ar);
TypedValue* tg_4Pair_toArray(ActRec* ar);
TypedValue* tg_4Pair_getIterator(ActRec* ar);
TypedValue* tg_4Pair_map(ActRec* ar);
TypedValue* tg_4Pair_filter(ActRec* ar);
TypedValue* tg_4Pair_zip(ActRec* ar);
TypedValue* tg_4Pair_at(ActRec* ar);
TypedValue* tg_4Pair_get(ActRec* ar);
TypedValue* tg_4Pair_containsKey(ActRec* ar);
VM::Instance* new_PairIterator_Instance(VM::Class*);
TypedValue* tg_12PairIterator___construct(ActRec* ar);
TypedValue* tg_12PairIterator_current(ActRec* ar);
TypedValue* tg_12PairIterator_key(ActRec* ar);
TypedValue* tg_12PairIterator_valid(ActRec* ar);
TypedValue* tg_12PairIterator_next(ActRec* ar);
TypedValue* tg_12PairIterator_rewind(ActRec* ar);
VM::Instance* new_Continuation_Instance(VM::Class*);
TypedValue* tg_12Continuation___construct(ActRec* ar);
TypedValue* tg_12Continuation_update(ActRec* ar);
TypedValue* tg_12Continuation_getWaitHandle(ActRec* ar);
TypedValue* tg_12Continuation_getLabel(ActRec* ar);
TypedValue* tg_12Continuation_num_args(ActRec* ar);
TypedValue* tg_12Continuation_get_args(ActRec* ar);
TypedValue* tg_12Continuation_get_arg(ActRec* ar);
TypedValue* tg_12Continuation_current(ActRec* ar);
TypedValue* tg_12Continuation_key(ActRec* ar);
TypedValue* tg_12Continuation_next(ActRec* ar);
TypedValue* tg_12Continuation_rewind(ActRec* ar);
TypedValue* tg_12Continuation_valid(ActRec* ar);
TypedValue* tg_12Continuation_send(ActRec* ar);
TypedValue* tg_12Continuation_raise(ActRec* ar);
TypedValue* tg_12Continuation_getOrigFuncName(ActRec* ar);
TypedValue* tg_12Continuation___clone(ActRec* ar);
VM::Instance* new_DummyContinuation_Instance(VM::Class*);
TypedValue* tg_17DummyContinuation___construct(ActRec* ar);
TypedValue* tg_17DummyContinuation_current(ActRec* ar);
TypedValue* tg_17DummyContinuation_key(ActRec* ar);
TypedValue* tg_17DummyContinuation_next(ActRec* ar);
TypedValue* tg_17DummyContinuation_rewind(ActRec* ar);
TypedValue* tg_17DummyContinuation_valid(ActRec* ar);
VM::Instance* new_DateTime_Instance(VM::Class*);
TypedValue* tg_8DateTime_add(ActRec* ar);
TypedValue* tg_8DateTime___construct(ActRec* ar);
TypedValue* tg_8DateTime_createFromFormat(ActRec* ar);
TypedValue* tg_8DateTime_diff(ActRec* ar);
TypedValue* tg_8DateTime_format(ActRec* ar);
TypedValue* tg_8DateTime_getLastErrors(ActRec* ar);
TypedValue* tg_8DateTime_getOffset(ActRec* ar);
TypedValue* tg_8DateTime_getTimestamp(ActRec* ar);
TypedValue* tg_8DateTime_getTimezone(ActRec* ar);
TypedValue* tg_8DateTime_modify(ActRec* ar);
TypedValue* tg_8DateTime_setDate(ActRec* ar);
TypedValue* tg_8DateTime_setISODate(ActRec* ar);
TypedValue* tg_8DateTime_setTime(ActRec* ar);
TypedValue* tg_8DateTime_setTimestamp(ActRec* ar);
TypedValue* tg_8DateTime_setTimezone(ActRec* ar);
TypedValue* tg_8DateTime_sub(ActRec* ar);
VM::Instance* new_DateTimeZone_Instance(VM::Class*);
TypedValue* tg_12DateTimeZone___construct(ActRec* ar);
TypedValue* tg_12DateTimeZone_getLocation(ActRec* ar);
TypedValue* tg_12DateTimeZone_getName(ActRec* ar);
TypedValue* tg_12DateTimeZone_getOffset(ActRec* ar);
TypedValue* tg_12DateTimeZone_getTransitions(ActRec* ar);
TypedValue* tg_12DateTimeZone_listAbbreviations(ActRec* ar);
TypedValue* tg_12DateTimeZone_listIdentifiers(ActRec* ar);
VM::Instance* new_DateInterval_Instance(VM::Class*);
TypedValue* tg_12DateInterval___construct(ActRec* ar);
TypedValue* tg_12DateInterval___get(ActRec* ar);
TypedValue* tg_12DateInterval___set(ActRec* ar);
TypedValue* tg_12DateInterval_createFromDateString(ActRec* ar);
TypedValue* tg_12DateInterval_format(ActRec* ar);
VM::Instance* new_DebuggerProxyCmdUser_Instance(VM::Class*);
TypedValue* tg_20DebuggerProxyCmdUser___construct(ActRec* ar);
TypedValue* tg_20DebuggerProxyCmdUser_isLocal(ActRec* ar);
TypedValue* tg_20DebuggerProxyCmdUser_send(ActRec* ar);
VM::Instance* new_DebuggerClientCmdUser_Instance(VM::Class*);
TypedValue* tg_21DebuggerClientCmdUser___construct(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_quit(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_print(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_help(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_info(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_output(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_error(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_code(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_ask(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_wrap(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_helpTitle(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_helpCmds(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_helpBody(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_helpSection(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_tutorial(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_getCode(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_getCommand(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_arg(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_argCount(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_argValue(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_lineRest(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_args(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_send(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_xend(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_getCurrentLocation(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_getStackTrace(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_getFrame(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_printFrame(ActRec* ar);
TypedValue* tg_21DebuggerClientCmdUser_addCompletion(ActRec* ar);
VM::Instance* new_DebuggerClient_Instance(VM::Class*);
TypedValue* tg_14DebuggerClient___construct(ActRec* ar);
TypedValue* tg_14DebuggerClient_getState(ActRec* ar);
TypedValue* tg_14DebuggerClient_init(ActRec* ar);
TypedValue* tg_14DebuggerClient_processCmd(ActRec* ar);
VM::Instance* new_DOMNode_Instance(VM::Class*);
TypedValue* tg_7DOMNode___construct(ActRec* ar);
TypedValue* tg_7DOMNode_appendChild(ActRec* ar);
TypedValue* tg_7DOMNode_cloneNode(ActRec* ar);
TypedValue* tg_7DOMNode_getLineNo(ActRec* ar);
TypedValue* tg_7DOMNode_hasAttributes(ActRec* ar);
TypedValue* tg_7DOMNode_hasChildNodes(ActRec* ar);
TypedValue* tg_7DOMNode_insertBefore(ActRec* ar);
TypedValue* tg_7DOMNode_isDefaultNamespace(ActRec* ar);
TypedValue* tg_7DOMNode_isSameNode(ActRec* ar);
TypedValue* tg_7DOMNode_isSupported(ActRec* ar);
TypedValue* tg_7DOMNode_lookupNamespaceUri(ActRec* ar);
TypedValue* tg_7DOMNode_lookupPrefix(ActRec* ar);
TypedValue* tg_7DOMNode_normalize(ActRec* ar);
TypedValue* tg_7DOMNode_removeChild(ActRec* ar);
TypedValue* tg_7DOMNode_replaceChild(ActRec* ar);
TypedValue* tg_7DOMNode_c14n(ActRec* ar);
TypedValue* tg_7DOMNode_c14nfile(ActRec* ar);
TypedValue* tg_7DOMNode_getNodePath(ActRec* ar);
TypedValue* tg_7DOMNode___get(ActRec* ar);
TypedValue* tg_7DOMNode___set(ActRec* ar);
TypedValue* tg_7DOMNode___isset(ActRec* ar);
VM::Instance* new_DOMAttr_Instance(VM::Class*);
TypedValue* tg_7DOMAttr___construct(ActRec* ar);
TypedValue* tg_7DOMAttr_isId(ActRec* ar);
TypedValue* tg_7DOMAttr___get(ActRec* ar);
TypedValue* tg_7DOMAttr___set(ActRec* ar);
TypedValue* tg_7DOMAttr___isset(ActRec* ar);
VM::Instance* new_DOMCharacterData_Instance(VM::Class*);
TypedValue* tg_16DOMCharacterData___construct(ActRec* ar);
TypedValue* tg_16DOMCharacterData_appendData(ActRec* ar);
TypedValue* tg_16DOMCharacterData_deleteData(ActRec* ar);
TypedValue* tg_16DOMCharacterData_insertData(ActRec* ar);
TypedValue* tg_16DOMCharacterData_replaceData(ActRec* ar);
TypedValue* tg_16DOMCharacterData_substringData(ActRec* ar);
TypedValue* tg_16DOMCharacterData___get(ActRec* ar);
TypedValue* tg_16DOMCharacterData___set(ActRec* ar);
TypedValue* tg_16DOMCharacterData___isset(ActRec* ar);
VM::Instance* new_DOMComment_Instance(VM::Class*);
TypedValue* tg_10DOMComment___construct(ActRec* ar);
VM::Instance* new_DOMText_Instance(VM::Class*);
TypedValue* tg_7DOMText___construct(ActRec* ar);
TypedValue* tg_7DOMText_isWhitespaceInElementContent(ActRec* ar);
TypedValue* tg_7DOMText_splitText(ActRec* ar);
TypedValue* tg_7DOMText___get(ActRec* ar);
TypedValue* tg_7DOMText___set(ActRec* ar);
TypedValue* tg_7DOMText___isset(ActRec* ar);
VM::Instance* new_DOMCDATASection_Instance(VM::Class*);
TypedValue* tg_15DOMCDATASection___construct(ActRec* ar);
VM::Instance* new_DOMDocument_Instance(VM::Class*);
TypedValue* tg_11DOMDocument___construct(ActRec* ar);
TypedValue* tg_11DOMDocument_createAttribute(ActRec* ar);
TypedValue* tg_11DOMDocument_createAttributens(ActRec* ar);
TypedValue* tg_11DOMDocument_createCDATASection(ActRec* ar);
TypedValue* tg_11DOMDocument_createComment(ActRec* ar);
TypedValue* tg_11DOMDocument_createDocumentFragment(ActRec* ar);
TypedValue* tg_11DOMDocument_createElement(ActRec* ar);
TypedValue* tg_11DOMDocument_createElementNS(ActRec* ar);
TypedValue* tg_11DOMDocument_createEntityReference(ActRec* ar);
TypedValue* tg_11DOMDocument_createProcessingInstruction(ActRec* ar);
TypedValue* tg_11DOMDocument_createTextNode(ActRec* ar);
TypedValue* tg_11DOMDocument_getElementById(ActRec* ar);
TypedValue* tg_11DOMDocument_getElementsByTagName(ActRec* ar);
TypedValue* tg_11DOMDocument_getElementsByTagNameNS(ActRec* ar);
TypedValue* tg_11DOMDocument_importNode(ActRec* ar);
TypedValue* tg_11DOMDocument_load(ActRec* ar);
TypedValue* tg_11DOMDocument_loadHTML(ActRec* ar);
TypedValue* tg_11DOMDocument_loadHTMLFile(ActRec* ar);
TypedValue* tg_11DOMDocument_loadXML(ActRec* ar);
TypedValue* tg_11DOMDocument_normalizeDocument(ActRec* ar);
TypedValue* tg_11DOMDocument_registerNodeClass(ActRec* ar);
TypedValue* tg_11DOMDocument_relaxNGValidate(ActRec* ar);
TypedValue* tg_11DOMDocument_relaxNGValidateSource(ActRec* ar);
TypedValue* tg_11DOMDocument_save(ActRec* ar);
TypedValue* tg_11DOMDocument_saveHTML(ActRec* ar);
TypedValue* tg_11DOMDocument_saveHTMLFile(ActRec* ar);
TypedValue* tg_11DOMDocument_saveXML(ActRec* ar);
TypedValue* tg_11DOMDocument_schemaValidate(ActRec* ar);
TypedValue* tg_11DOMDocument_schemaValidateSource(ActRec* ar);
TypedValue* tg_11DOMDocument_validate(ActRec* ar);
TypedValue* tg_11DOMDocument_xinclude(ActRec* ar);
TypedValue* tg_11DOMDocument___get(ActRec* ar);
TypedValue* tg_11DOMDocument___set(ActRec* ar);
TypedValue* tg_11DOMDocument___isset(ActRec* ar);
VM::Instance* new_DOMDocumentFragment_Instance(VM::Class*);
TypedValue* tg_19DOMDocumentFragment___construct(ActRec* ar);
TypedValue* tg_19DOMDocumentFragment_appendXML(ActRec* ar);
VM::Instance* new_DOMDocumentType_Instance(VM::Class*);
TypedValue* tg_15DOMDocumentType___construct(ActRec* ar);
TypedValue* tg_15DOMDocumentType___get(ActRec* ar);
TypedValue* tg_15DOMDocumentType___set(ActRec* ar);
TypedValue* tg_15DOMDocumentType___isset(ActRec* ar);
VM::Instance* new_DOMElement_Instance(VM::Class*);
TypedValue* tg_10DOMElement___construct(ActRec* ar);
TypedValue* tg_10DOMElement_getAttribute(ActRec* ar);
TypedValue* tg_10DOMElement_getAttributeNode(ActRec* ar);
TypedValue* tg_10DOMElement_getAttributeNodeNS(ActRec* ar);
TypedValue* tg_10DOMElement_getAttributeNS(ActRec* ar);
TypedValue* tg_10DOMElement_getElementsByTagName(ActRec* ar);
TypedValue* tg_10DOMElement_getElementsByTagNameNS(ActRec* ar);
TypedValue* tg_10DOMElement_hasAttribute(ActRec* ar);
TypedValue* tg_10DOMElement_hasAttributeNS(ActRec* ar);
TypedValue* tg_10DOMElement_removeAttribute(ActRec* ar);
TypedValue* tg_10DOMElement_removeAttributeNode(ActRec* ar);
TypedValue* tg_10DOMElement_removeAttributeNS(ActRec* ar);
TypedValue* tg_10DOMElement_setAttribute(ActRec* ar);
TypedValue* tg_10DOMElement_setAttributeNode(ActRec* ar);
TypedValue* tg_10DOMElement_setAttributeNodeNS(ActRec* ar);
TypedValue* tg_10DOMElement_setAttributeNS(ActRec* ar);
TypedValue* tg_10DOMElement_setIDAttribute(ActRec* ar);
TypedValue* tg_10DOMElement_setIDAttributeNode(ActRec* ar);
TypedValue* tg_10DOMElement_setIDAttributeNS(ActRec* ar);
TypedValue* tg_10DOMElement___get(ActRec* ar);
TypedValue* tg_10DOMElement___set(ActRec* ar);
TypedValue* tg_10DOMElement___isset(ActRec* ar);
VM::Instance* new_DOMEntity_Instance(VM::Class*);
TypedValue* tg_9DOMEntity___construct(ActRec* ar);
TypedValue* tg_9DOMEntity___get(ActRec* ar);
TypedValue* tg_9DOMEntity___set(ActRec* ar);
TypedValue* tg_9DOMEntity___isset(ActRec* ar);
VM::Instance* new_DOMEntityReference_Instance(VM::Class*);
TypedValue* tg_18DOMEntityReference___construct(ActRec* ar);
VM::Instance* new_DOMNotation_Instance(VM::Class*);
TypedValue* tg_11DOMNotation___construct(ActRec* ar);
TypedValue* tg_11DOMNotation___get(ActRec* ar);
TypedValue* tg_11DOMNotation___set(ActRec* ar);
TypedValue* tg_11DOMNotation___isset(ActRec* ar);
VM::Instance* new_DOMProcessingInstruction_Instance(VM::Class*);
TypedValue* tg_24DOMProcessingInstruction___construct(ActRec* ar);
TypedValue* tg_24DOMProcessingInstruction___get(ActRec* ar);
TypedValue* tg_24DOMProcessingInstruction___set(ActRec* ar);
TypedValue* tg_24DOMProcessingInstruction___isset(ActRec* ar);
VM::Instance* new_DOMNodeIterator_Instance(VM::Class*);
TypedValue* tg_15DOMNodeIterator___construct(ActRec* ar);
TypedValue* tg_15DOMNodeIterator_current(ActRec* ar);
TypedValue* tg_15DOMNodeIterator_key(ActRec* ar);
TypedValue* tg_15DOMNodeIterator_next(ActRec* ar);
TypedValue* tg_15DOMNodeIterator_rewind(ActRec* ar);
TypedValue* tg_15DOMNodeIterator_valid(ActRec* ar);
VM::Instance* new_DOMNamedNodeMap_Instance(VM::Class*);
TypedValue* tg_15DOMNamedNodeMap___construct(ActRec* ar);
TypedValue* tg_15DOMNamedNodeMap_getNamedItem(ActRec* ar);
TypedValue* tg_15DOMNamedNodeMap_getNamedItemNS(ActRec* ar);
TypedValue* tg_15DOMNamedNodeMap_item(ActRec* ar);
TypedValue* tg_15DOMNamedNodeMap___get(ActRec* ar);
TypedValue* tg_15DOMNamedNodeMap___set(ActRec* ar);
TypedValue* tg_15DOMNamedNodeMap___isset(ActRec* ar);
TypedValue* tg_15DOMNamedNodeMap_getIterator(ActRec* ar);
VM::Instance* new_DOMNodeList_Instance(VM::Class*);
TypedValue* tg_11DOMNodeList___construct(ActRec* ar);
TypedValue* tg_11DOMNodeList_item(ActRec* ar);
TypedValue* tg_11DOMNodeList___get(ActRec* ar);
TypedValue* tg_11DOMNodeList___set(ActRec* ar);
TypedValue* tg_11DOMNodeList___isset(ActRec* ar);
TypedValue* tg_11DOMNodeList_getIterator(ActRec* ar);
VM::Instance* new_DOMImplementation_Instance(VM::Class*);
TypedValue* tg_17DOMImplementation___construct(ActRec* ar);
TypedValue* tg_17DOMImplementation_createDocument(ActRec* ar);
TypedValue* tg_17DOMImplementation_createDocumentType(ActRec* ar);
TypedValue* tg_17DOMImplementation_hasFeature(ActRec* ar);
VM::Instance* new_DOMXPath_Instance(VM::Class*);
TypedValue* tg_8DOMXPath___construct(ActRec* ar);
TypedValue* tg_8DOMXPath_evaluate(ActRec* ar);
TypedValue* tg_8DOMXPath_query(ActRec* ar);
TypedValue* tg_8DOMXPath_registerNamespace(ActRec* ar);
TypedValue* tg_8DOMXPath_registerPHPFunctions(ActRec* ar);
TypedValue* tg_8DOMXPath___get(ActRec* ar);
TypedValue* tg_8DOMXPath___set(ActRec* ar);
TypedValue* tg_8DOMXPath___isset(ActRec* ar);
VM::Instance* new_UConverter_Instance(VM::Class*);
TypedValue* tg_10UConverter___construct(ActRec* ar);
TypedValue* tg_10UConverter___destruct(ActRec* ar);
TypedValue* tg_10UConverter_getSourceEncoding(ActRec* ar);
TypedValue* tg_10UConverter_setSourceEncoding(ActRec* ar);
TypedValue* tg_10UConverter_getDestinationEncoding(ActRec* ar);
TypedValue* tg_10UConverter_setDestinationEncoding(ActRec* ar);
TypedValue* tg_10UConverter_getSourceType(ActRec* ar);
TypedValue* tg_10UConverter_getDestinationType(ActRec* ar);
TypedValue* tg_10UConverter_getSubstChars(ActRec* ar);
TypedValue* tg_10UConverter_setSubstChars(ActRec* ar);
TypedValue* tg_10UConverter_fromUCallback(ActRec* ar);
TypedValue* tg_10UConverter_toUCallback(ActRec* ar);
TypedValue* tg_10UConverter_convert(ActRec* ar);
TypedValue* tg_10UConverter_transcode(ActRec* ar);
TypedValue* tg_10UConverter_getErrorCode(ActRec* ar);
TypedValue* tg_10UConverter_getErrorMessage(ActRec* ar);
TypedValue* tg_10UConverter_reasonText(ActRec* ar);
TypedValue* tg_10UConverter_getAvailable(ActRec* ar);
TypedValue* tg_10UConverter_getAliases(ActRec* ar);
TypedValue* tg_10UConverter_getStandards(ActRec* ar);
TypedValue* tg_10UConverter_getStandardName(ActRec* ar);
TypedValue* tg_10UConverter_getMIMEName(ActRec* ar);
VM::Instance* new_EncodingDetector_Instance(VM::Class*);
TypedValue* tg_16EncodingDetector___construct(ActRec* ar);
TypedValue* tg_16EncodingDetector_setText(ActRec* ar);
TypedValue* tg_16EncodingDetector_setDeclaredEncoding(ActRec* ar);
TypedValue* tg_16EncodingDetector_detect(ActRec* ar);
TypedValue* tg_16EncodingDetector_detectAll(ActRec* ar);
VM::Instance* new_EncodingMatch_Instance(VM::Class*);
TypedValue* tg_13EncodingMatch___construct(ActRec* ar);
TypedValue* tg_13EncodingMatch_isValid(ActRec* ar);
TypedValue* tg_13EncodingMatch_getEncoding(ActRec* ar);
TypedValue* tg_13EncodingMatch_getConfidence(ActRec* ar);
TypedValue* tg_13EncodingMatch_getLanguage(ActRec* ar);
TypedValue* tg_13EncodingMatch_getUTF8(ActRec* ar);
VM::Instance* new_SpoofChecker_Instance(VM::Class*);
TypedValue* tg_12SpoofChecker___construct(ActRec* ar);
TypedValue* tg_12SpoofChecker_isSuspicious(ActRec* ar);
TypedValue* tg_12SpoofChecker_areConfusable(ActRec* ar);
TypedValue* tg_12SpoofChecker_setAllowedLocales(ActRec* ar);
TypedValue* tg_12SpoofChecker_setChecks(ActRec* ar);
VM::Instance* new_ImageSprite_Instance(VM::Class*);
TypedValue* tg_11ImageSprite___construct(ActRec* ar);
TypedValue* tg_11ImageSprite_addFile(ActRec* ar);
TypedValue* tg_11ImageSprite_addString(ActRec* ar);
TypedValue* tg_11ImageSprite_addUrl(ActRec* ar);
TypedValue* tg_11ImageSprite_clear(ActRec* ar);
TypedValue* tg_11ImageSprite_loadDims(ActRec* ar);
TypedValue* tg_11ImageSprite_loadImages(ActRec* ar);
TypedValue* tg_11ImageSprite_output(ActRec* ar);
TypedValue* tg_11ImageSprite_css(ActRec* ar);
TypedValue* tg_11ImageSprite_getErrors(ActRec* ar);
TypedValue* tg_11ImageSprite_mapping(ActRec* ar);
TypedValue* tg_11ImageSprite___destruct(ActRec* ar);
VM::Instance* new_Collator_Instance(VM::Class*);
TypedValue* tg_8Collator___construct(ActRec* ar);
TypedValue* tg_8Collator_asort(ActRec* ar);
TypedValue* tg_8Collator_compare(ActRec* ar);
TypedValue* tg_8Collator_create(ActRec* ar);
TypedValue* tg_8Collator_getattribute(ActRec* ar);
TypedValue* tg_8Collator_geterrorcode(ActRec* ar);
TypedValue* tg_8Collator_geterrormessage(ActRec* ar);
TypedValue* tg_8Collator_getlocale(ActRec* ar);
TypedValue* tg_8Collator_getstrength(ActRec* ar);
TypedValue* tg_8Collator_setattribute(ActRec* ar);
TypedValue* tg_8Collator_setstrength(ActRec* ar);
TypedValue* tg_8Collator_sortwithsortkeys(ActRec* ar);
TypedValue* tg_8Collator_sort(ActRec* ar);
VM::Instance* new_Locale_Instance(VM::Class*);
TypedValue* tg_6Locale___construct(ActRec* ar);
VM::Instance* new_Normalizer_Instance(VM::Class*);
TypedValue* tg_10Normalizer___construct(ActRec* ar);
TypedValue* tg_10Normalizer_isnormalized(ActRec* ar);
TypedValue* tg_10Normalizer_normalize(ActRec* ar);
VM::Instance* new_MutableArrayIterator_Instance(VM::Class*);
TypedValue* tg_20MutableArrayIterator___construct(ActRec* ar);
TypedValue* tg_20MutableArrayIterator_currentRef(ActRec* ar);
TypedValue* tg_20MutableArrayIterator_current(ActRec* ar);
TypedValue* tg_20MutableArrayIterator_key(ActRec* ar);
TypedValue* tg_20MutableArrayIterator_next(ActRec* ar);
TypedValue* tg_20MutableArrayIterator_valid(ActRec* ar);
VM::Instance* new_Memcache_Instance(VM::Class*);
TypedValue* tg_8Memcache___construct(ActRec* ar);
TypedValue* tg_8Memcache_connect(ActRec* ar);
TypedValue* tg_8Memcache_pconnect(ActRec* ar);
TypedValue* tg_8Memcache_add(ActRec* ar);
TypedValue* tg_8Memcache_set(ActRec* ar);
TypedValue* tg_8Memcache_replace(ActRec* ar);
TypedValue* tg_8Memcache_get(ActRec* ar);
TypedValue* tg_8Memcache_delete(ActRec* ar);
TypedValue* tg_8Memcache_increment(ActRec* ar);
TypedValue* tg_8Memcache_decrement(ActRec* ar);
TypedValue* tg_8Memcache_getversion(ActRec* ar);
TypedValue* tg_8Memcache_flush(ActRec* ar);
TypedValue* tg_8Memcache_setoptimeout(ActRec* ar);
TypedValue* tg_8Memcache_close(ActRec* ar);
TypedValue* tg_8Memcache_getserverstatus(ActRec* ar);
TypedValue* tg_8Memcache_setcompressthreshold(ActRec* ar);
TypedValue* tg_8Memcache_getstats(ActRec* ar);
TypedValue* tg_8Memcache_getextendedstats(ActRec* ar);
TypedValue* tg_8Memcache_setserverparams(ActRec* ar);
TypedValue* tg_8Memcache_addserver(ActRec* ar);
TypedValue* tg_8Memcache___destruct(ActRec* ar);
VM::Instance* new_Memcached_Instance(VM::Class*);
TypedValue* tg_9Memcached___construct(ActRec* ar);
TypedValue* tg_9Memcached_add(ActRec* ar);
TypedValue* tg_9Memcached_addByKey(ActRec* ar);
TypedValue* tg_9Memcached_addServer(ActRec* ar);
TypedValue* tg_9Memcached_addServers(ActRec* ar);
TypedValue* tg_9Memcached_append(ActRec* ar);
TypedValue* tg_9Memcached_appendByKey(ActRec* ar);
TypedValue* tg_9Memcached_cas(ActRec* ar);
TypedValue* tg_9Memcached_casByKey(ActRec* ar);
TypedValue* tg_9Memcached_decrement(ActRec* ar);
TypedValue* tg_9Memcached_delete(ActRec* ar);
TypedValue* tg_9Memcached_deleteByKey(ActRec* ar);
TypedValue* tg_9Memcached_fetch(ActRec* ar);
TypedValue* tg_9Memcached_fetchAll(ActRec* ar);
TypedValue* tg_9Memcached_flush(ActRec* ar);
TypedValue* tg_9Memcached_get(ActRec* ar);
TypedValue* tg_9Memcached_getByKey(ActRec* ar);
TypedValue* tg_9Memcached_getDelayed(ActRec* ar);
TypedValue* tg_9Memcached_getDelayedByKey(ActRec* ar);
TypedValue* tg_9Memcached_getMulti(ActRec* ar);
TypedValue* tg_9Memcached_getMultiByKey(ActRec* ar);
TypedValue* tg_9Memcached_getOption(ActRec* ar);
TypedValue* tg_9Memcached_getResultCode(ActRec* ar);
TypedValue* tg_9Memcached_getResultMessage(ActRec* ar);
TypedValue* tg_9Memcached_getServerByKey(ActRec* ar);
TypedValue* tg_9Memcached_getServerList(ActRec* ar);
TypedValue* tg_9Memcached_getStats(ActRec* ar);
TypedValue* tg_9Memcached_getVersion(ActRec* ar);
TypedValue* tg_9Memcached_increment(ActRec* ar);
TypedValue* tg_9Memcached_prepend(ActRec* ar);
TypedValue* tg_9Memcached_prependByKey(ActRec* ar);
TypedValue* tg_9Memcached_replace(ActRec* ar);
TypedValue* tg_9Memcached_replaceByKey(ActRec* ar);
TypedValue* tg_9Memcached_set(ActRec* ar);
TypedValue* tg_9Memcached_setByKey(ActRec* ar);
TypedValue* tg_9Memcached_setMulti(ActRec* ar);
TypedValue* tg_9Memcached_setMultiByKey(ActRec* ar);
TypedValue* tg_9Memcached_setOption(ActRec* ar);
VM::Instance* new_PDO_Instance(VM::Class*);
TypedValue* tg_3PDO___construct(ActRec* ar);
TypedValue* tg_3PDO_prepare(ActRec* ar);
TypedValue* tg_3PDO_begintransaction(ActRec* ar);
TypedValue* tg_3PDO_commit(ActRec* ar);
TypedValue* tg_3PDO_rollback(ActRec* ar);
TypedValue* tg_3PDO_setattribute(ActRec* ar);
TypedValue* tg_3PDO_getattribute(ActRec* ar);
TypedValue* tg_3PDO_exec(ActRec* ar);
TypedValue* tg_3PDO_lastinsertid(ActRec* ar);
TypedValue* tg_3PDO_errorcode(ActRec* ar);
TypedValue* tg_3PDO_errorinfo(ActRec* ar);
TypedValue* tg_3PDO_query(ActRec* ar);
TypedValue* tg_3PDO_quote(ActRec* ar);
TypedValue* tg_3PDO___wakeup(ActRec* ar);
TypedValue* tg_3PDO___sleep(ActRec* ar);
TypedValue* tg_3PDO_getavailabledrivers(ActRec* ar);
VM::Instance* new_PDOStatement_Instance(VM::Class*);
TypedValue* tg_12PDOStatement___construct(ActRec* ar);
TypedValue* tg_12PDOStatement_execute(ActRec* ar);
TypedValue* tg_12PDOStatement_fetch(ActRec* ar);
TypedValue* tg_12PDOStatement_fetchobject(ActRec* ar);
TypedValue* tg_12PDOStatement_fetchcolumn(ActRec* ar);
TypedValue* tg_12PDOStatement_fetchall(ActRec* ar);
TypedValue* tg_12PDOStatement_bindvalue(ActRec* ar);
TypedValue* tg_12PDOStatement_bindparam(ActRec* ar);
TypedValue* tg_12PDOStatement_bindcolumn(ActRec* ar);
TypedValue* tg_12PDOStatement_rowcount(ActRec* ar);
TypedValue* tg_12PDOStatement_errorcode(ActRec* ar);
TypedValue* tg_12PDOStatement_errorinfo(ActRec* ar);
TypedValue* tg_12PDOStatement_setattribute(ActRec* ar);
TypedValue* tg_12PDOStatement_getattribute(ActRec* ar);
TypedValue* tg_12PDOStatement_columncount(ActRec* ar);
TypedValue* tg_12PDOStatement_getcolumnmeta(ActRec* ar);
TypedValue* tg_12PDOStatement_setfetchmode(ActRec* ar);
TypedValue* tg_12PDOStatement_nextrowset(ActRec* ar);
TypedValue* tg_12PDOStatement_closecursor(ActRec* ar);
TypedValue* tg_12PDOStatement_debugdumpparams(ActRec* ar);
TypedValue* tg_12PDOStatement_current(ActRec* ar);
TypedValue* tg_12PDOStatement_key(ActRec* ar);
TypedValue* tg_12PDOStatement_next(ActRec* ar);
TypedValue* tg_12PDOStatement_rewind(ActRec* ar);
TypedValue* tg_12PDOStatement_valid(ActRec* ar);
TypedValue* tg_12PDOStatement___wakeup(ActRec* ar);
TypedValue* tg_12PDOStatement___sleep(ActRec* ar);
VM::Instance* new_SimpleXMLElement_Instance(VM::Class*);
TypedValue* tg_16SimpleXMLElement___construct(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_offsetExists(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_offsetGet(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_offsetSet(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_offsetUnset(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_getIterator(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_count(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_xpath(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_registerXPathNamespace(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_asXML(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_getNamespaces(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_getDocNamespaces(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_children(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_getName(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_attributes(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_addChild(ActRec* ar);
TypedValue* tg_16SimpleXMLElement_addAttribute(ActRec* ar);
TypedValue* tg_16SimpleXMLElement___toString(ActRec* ar);
TypedValue* tg_16SimpleXMLElement___get(ActRec* ar);
TypedValue* tg_16SimpleXMLElement___set(ActRec* ar);
TypedValue* tg_16SimpleXMLElement___isset(ActRec* ar);
TypedValue* tg_16SimpleXMLElement___unset(ActRec* ar);
VM::Instance* new_LibXMLError_Instance(VM::Class*);
TypedValue* tg_11LibXMLError___construct(ActRec* ar);
VM::Instance* new_SimpleXMLElementIterator_Instance(VM::Class*);
TypedValue* tg_24SimpleXMLElementIterator___construct(ActRec* ar);
TypedValue* tg_24SimpleXMLElementIterator_current(ActRec* ar);
TypedValue* tg_24SimpleXMLElementIterator_key(ActRec* ar);
TypedValue* tg_24SimpleXMLElementIterator_next(ActRec* ar);
TypedValue* tg_24SimpleXMLElementIterator_rewind(ActRec* ar);
TypedValue* tg_24SimpleXMLElementIterator_valid(ActRec* ar);
VM::Instance* new_SoapServer_Instance(VM::Class*);
TypedValue* tg_10SoapServer___construct(ActRec* ar);
TypedValue* tg_10SoapServer_setclass(ActRec* ar);
TypedValue* tg_10SoapServer_setobject(ActRec* ar);
TypedValue* tg_10SoapServer_addfunction(ActRec* ar);
TypedValue* tg_10SoapServer_getfunctions(ActRec* ar);
TypedValue* tg_10SoapServer_handle(ActRec* ar);
TypedValue* tg_10SoapServer_setpersistence(ActRec* ar);
TypedValue* tg_10SoapServer_fault(ActRec* ar);
TypedValue* tg_10SoapServer_addsoapheader(ActRec* ar);
VM::Instance* new_SoapClient_Instance(VM::Class*);
TypedValue* tg_10SoapClient___construct(ActRec* ar);
TypedValue* tg_10SoapClient___call(ActRec* ar);
TypedValue* tg_10SoapClient___soapcall(ActRec* ar);
TypedValue* tg_10SoapClient___getlastrequest(ActRec* ar);
TypedValue* tg_10SoapClient___getlastresponse(ActRec* ar);
TypedValue* tg_10SoapClient___getlastrequestheaders(ActRec* ar);
TypedValue* tg_10SoapClient___getlastresponseheaders(ActRec* ar);
TypedValue* tg_10SoapClient___getfunctions(ActRec* ar);
TypedValue* tg_10SoapClient___gettypes(ActRec* ar);
TypedValue* tg_10SoapClient___dorequest(ActRec* ar);
TypedValue* tg_10SoapClient___setcookie(ActRec* ar);
TypedValue* tg_10SoapClient___setlocation(ActRec* ar);
TypedValue* tg_10SoapClient___setsoapheaders(ActRec* ar);
VM::Instance* new_SoapVar_Instance(VM::Class*);
TypedValue* tg_7SoapVar___construct(ActRec* ar);
VM::Instance* new_SoapParam_Instance(VM::Class*);
TypedValue* tg_9SoapParam___construct(ActRec* ar);
VM::Instance* new_SoapHeader_Instance(VM::Class*);
TypedValue* tg_10SoapHeader___construct(ActRec* ar);
VM::Instance* new_SQLite3_Instance(VM::Class*);
TypedValue* tg_7SQLite3___construct(ActRec* ar);
TypedValue* tg_7SQLite3_open(ActRec* ar);
TypedValue* tg_7SQLite3_busytimeout(ActRec* ar);
TypedValue* tg_7SQLite3_close(ActRec* ar);
TypedValue* tg_7SQLite3_exec(ActRec* ar);
TypedValue* tg_7SQLite3_version(ActRec* ar);
TypedValue* tg_7SQLite3_lastinsertrowid(ActRec* ar);
TypedValue* tg_7SQLite3_lasterrorcode(ActRec* ar);
TypedValue* tg_7SQLite3_lasterrormsg(ActRec* ar);
TypedValue* tg_7SQLite3_loadextension(ActRec* ar);
TypedValue* tg_7SQLite3_changes(ActRec* ar);
TypedValue* tg_7SQLite3_escapestring(ActRec* ar);
TypedValue* tg_7SQLite3_prepare(ActRec* ar);
TypedValue* tg_7SQLite3_query(ActRec* ar);
TypedValue* tg_7SQLite3_querysingle(ActRec* ar);
TypedValue* tg_7SQLite3_createfunction(ActRec* ar);
TypedValue* tg_7SQLite3_createaggregate(ActRec* ar);
TypedValue* tg_7SQLite3_openblob(ActRec* ar);
VM::Instance* new_SQLite3Stmt_Instance(VM::Class*);
TypedValue* tg_11SQLite3Stmt___construct(ActRec* ar);
TypedValue* tg_11SQLite3Stmt_paramcount(ActRec* ar);
TypedValue* tg_11SQLite3Stmt_close(ActRec* ar);
TypedValue* tg_11SQLite3Stmt_reset(ActRec* ar);
TypedValue* tg_11SQLite3Stmt_clear(ActRec* ar);
TypedValue* tg_11SQLite3Stmt_bindparam(ActRec* ar);
TypedValue* tg_11SQLite3Stmt_bindvalue(ActRec* ar);
TypedValue* tg_11SQLite3Stmt_execute(ActRec* ar);
VM::Instance* new_SQLite3Result_Instance(VM::Class*);
TypedValue* tg_13SQLite3Result___construct(ActRec* ar);
TypedValue* tg_13SQLite3Result_numcolumns(ActRec* ar);
TypedValue* tg_13SQLite3Result_columnname(ActRec* ar);
TypedValue* tg_13SQLite3Result_columntype(ActRec* ar);
TypedValue* tg_13SQLite3Result_fetcharray(ActRec* ar);
TypedValue* tg_13SQLite3Result_reset(ActRec* ar);
TypedValue* tg_13SQLite3Result_finalize(ActRec* ar);
VM::Instance* new_XMLReader_Instance(VM::Class*);
TypedValue* tg_9XMLReader___construct(ActRec* ar);
TypedValue* tg_9XMLReader_open(ActRec* ar);
TypedValue* tg_9XMLReader_XML(ActRec* ar);
TypedValue* tg_9XMLReader_close(ActRec* ar);
TypedValue* tg_9XMLReader_read(ActRec* ar);
TypedValue* tg_9XMLReader_next(ActRec* ar);
TypedValue* tg_9XMLReader_readString(ActRec* ar);
TypedValue* tg_9XMLReader_readInnerXML(ActRec* ar);
TypedValue* tg_9XMLReader_readOuterXML(ActRec* ar);
TypedValue* tg_9XMLReader_moveToNextAttribute(ActRec* ar);
TypedValue* tg_9XMLReader_getAttribute(ActRec* ar);
TypedValue* tg_9XMLReader_getAttributeNo(ActRec* ar);
TypedValue* tg_9XMLReader_getAttributeNs(ActRec* ar);
TypedValue* tg_9XMLReader_moveToAttribute(ActRec* ar);
TypedValue* tg_9XMLReader_moveToAttributeNo(ActRec* ar);
TypedValue* tg_9XMLReader_moveToAttributeNs(ActRec* ar);
TypedValue* tg_9XMLReader_moveToElement(ActRec* ar);
TypedValue* tg_9XMLReader_moveToFirstAttribute(ActRec* ar);
TypedValue* tg_9XMLReader_isValid(ActRec* ar);
TypedValue* tg_9XMLReader_expand(ActRec* ar);
TypedValue* tg_9XMLReader___get(ActRec* ar);
TypedValue* tg_9XMLReader_getParserProperty(ActRec* ar);
TypedValue* tg_9XMLReader_lookupNamespace(ActRec* ar);
TypedValue* tg_9XMLReader_setSchema(ActRec* ar);
TypedValue* tg_9XMLReader_setParserProperty(ActRec* ar);
TypedValue* tg_9XMLReader_setRelaxNGSchema(ActRec* ar);
TypedValue* tg_9XMLReader_setRelaxNGSchemaSource(ActRec* ar);
VM::Instance* new_XMLWriter_Instance(VM::Class*);
TypedValue* tg_9XMLWriter___construct(ActRec* ar);
TypedValue* tg_9XMLWriter_openMemory(ActRec* ar);
TypedValue* tg_9XMLWriter_openURI(ActRec* ar);
TypedValue* tg_9XMLWriter_setIndentString(ActRec* ar);
TypedValue* tg_9XMLWriter_setIndent(ActRec* ar);
TypedValue* tg_9XMLWriter_startDocument(ActRec* ar);
TypedValue* tg_9XMLWriter_startElement(ActRec* ar);
TypedValue* tg_9XMLWriter_startElementNS(ActRec* ar);
TypedValue* tg_9XMLWriter_writeElementNS(ActRec* ar);
TypedValue* tg_9XMLWriter_writeElement(ActRec* ar);
TypedValue* tg_9XMLWriter_endElement(ActRec* ar);
TypedValue* tg_9XMLWriter_fullEndElement(ActRec* ar);
TypedValue* tg_9XMLWriter_startAttributens(ActRec* ar);
TypedValue* tg_9XMLWriter_startAttribute(ActRec* ar);
TypedValue* tg_9XMLWriter_writeAttributeNS(ActRec* ar);
TypedValue* tg_9XMLWriter_writeAttribute(ActRec* ar);
TypedValue* tg_9XMLWriter_endAttribute(ActRec* ar);
TypedValue* tg_9XMLWriter_startCData(ActRec* ar);
TypedValue* tg_9XMLWriter_writeCData(ActRec* ar);
TypedValue* tg_9XMLWriter_endCData(ActRec* ar);
TypedValue* tg_9XMLWriter_startComment(ActRec* ar);
TypedValue* tg_9XMLWriter_writeComment(ActRec* ar);
TypedValue* tg_9XMLWriter_endComment(ActRec* ar);
TypedValue* tg_9XMLWriter_endDocument(ActRec* ar);
TypedValue* tg_9XMLWriter_startPI(ActRec* ar);
TypedValue* tg_9XMLWriter_writePI(ActRec* ar);
TypedValue* tg_9XMLWriter_endPI(ActRec* ar);
TypedValue* tg_9XMLWriter_text(ActRec* ar);
TypedValue* tg_9XMLWriter_writeRaw(ActRec* ar);
TypedValue* tg_9XMLWriter_startDTD(ActRec* ar);
TypedValue* tg_9XMLWriter_writeDTD(ActRec* ar);
TypedValue* tg_9XMLWriter_startDTDElement(ActRec* ar);
TypedValue* tg_9XMLWriter_writeDTDElement(ActRec* ar);
TypedValue* tg_9XMLWriter_endDTDElement(ActRec* ar);
TypedValue* tg_9XMLWriter_startDTDAttlist(ActRec* ar);
TypedValue* tg_9XMLWriter_writeDTDAttlist(ActRec* ar);
TypedValue* tg_9XMLWriter_endDTDAttlist(ActRec* ar);
TypedValue* tg_9XMLWriter_startDTDEntity(ActRec* ar);
TypedValue* tg_9XMLWriter_writeDTDEntity(ActRec* ar);
TypedValue* tg_9XMLWriter_endDTDEntity(ActRec* ar);
TypedValue* tg_9XMLWriter_endDTD(ActRec* ar);
TypedValue* tg_9XMLWriter_flush(ActRec* ar);
TypedValue* tg_9XMLWriter_outputMemory(ActRec* ar);

const long long hhbc_ext_funcs_count = 2205;
const HhbcExtFuncInfo hhbc_ext_funcs[] = {
  { "apache_note", fg_apache_note, (void *)&fh_apache_note },
  { "apache_request_headers", fg_apache_request_headers, (void *)&fh_apache_request_headers },
  { "apache_response_headers", fg_apache_response_headers, (void *)&fh_apache_response_headers },
  { "apache_setenv", fg_apache_setenv, (void *)&fh_apache_setenv },
  { "getallheaders", fg_getallheaders, (void *)&fh_getallheaders },
  { "virtual", fg_virtual, (void *)&fh_virtual },
  { "apache_get_config", fg_apache_get_config, (void *)&fh_apache_get_config },
  { "apache_get_scoreboard", fg_apache_get_scoreboard, (void *)&fh_apache_get_scoreboard },
  { "apache_get_rewrite_rules", fg_apache_get_rewrite_rules, (void *)&fh_apache_get_rewrite_rules },
  { "apc_add", fg_apc_add, (void *)&fh_apc_add },
  { "apc_store", fg_apc_store, (void *)&fh_apc_store },
  { "apc_fetch", fg_apc_fetch, (void *)&fh_apc_fetch },
  { "apc_delete", fg_apc_delete, (void *)&fh_apc_delete },
  { "apc_compile_file", fg_apc_compile_file, (void *)&fh_apc_compile_file },
  { "apc_cache_info", fg_apc_cache_info, (void *)&fh_apc_cache_info },
  { "apc_clear_cache", fg_apc_clear_cache, (void *)&fh_apc_clear_cache },
  { "apc_define_constants", fg_apc_define_constants, (void *)&fh_apc_define_constants },
  { "apc_load_constants", fg_apc_load_constants, (void *)&fh_apc_load_constants },
  { "apc_sma_info", fg_apc_sma_info, (void *)&fh_apc_sma_info },
  { "apc_filehits", fg_apc_filehits, (void *)&fh_apc_filehits },
  { "apc_delete_file", fg_apc_delete_file, (void *)&fh_apc_delete_file },
  { "apc_inc", fg_apc_inc, (void *)&fh_apc_inc },
  { "apc_dec", fg_apc_dec, (void *)&fh_apc_dec },
  { "apc_cas", fg_apc_cas, (void *)&fh_apc_cas },
  { "apc_exists", fg_apc_exists, (void *)&fh_apc_exists },
  { "apc_bin_dump", fg_apc_bin_dump, (void *)&fh_apc_bin_dump },
  { "apc_bin_load", fg_apc_bin_load, (void *)&fh_apc_bin_load },
  { "apc_bin_dumpfile", fg_apc_bin_dumpfile, (void *)&fh_apc_bin_dumpfile },
  { "apc_bin_loadfile", fg_apc_bin_loadfile, (void *)&fh_apc_bin_loadfile },
  { "override_function", fg_override_function, (void *)&fh_override_function },
  { "rename_function", fg_rename_function, (void *)&fh_rename_function },
  { "apd_set_browser_trace", fg_apd_set_browser_trace, (void *)&fh_apd_set_browser_trace },
  { "apd_set_pprof_trace", fg_apd_set_pprof_trace, (void *)&fh_apd_set_pprof_trace },
  { "apd_set_session_trace_socket", fg_apd_set_session_trace_socket, (void *)&fh_apd_set_session_trace_socket },
  { "apd_stop_trace", fg_apd_stop_trace, (void *)&fh_apd_stop_trace },
  { "apd_breakpoint", fg_apd_breakpoint, (void *)&fh_apd_breakpoint },
  { "apd_continue", fg_apd_continue, (void *)&fh_apd_continue },
  { "apd_echo", fg_apd_echo, (void *)&fh_apd_echo },
  { "array_change_key_case", fg_array_change_key_case, (void *)&fh_array_change_key_case },
  { "array_chunk", fg_array_chunk, (void *)&fh_array_chunk },
  { "array_column", fg_array_column, (void *)&fh_array_column },
  { "array_combine", fg_array_combine, (void *)&fh_array_combine },
  { "array_count_values", fg_array_count_values, (void *)&fh_array_count_values },
  { "array_fill_keys", fg_array_fill_keys, (void *)&fh_array_fill_keys },
  { "array_fill", fg_array_fill, (void *)&fh_array_fill },
  { "array_filter", fg_array_filter, (void *)&fh_array_filter },
  { "array_flip", fg_array_flip, (void *)&fh_array_flip },
  { "array_key_exists", fg_array_key_exists, (void *)&fh_array_key_exists },
  { "key_exists", fg_key_exists, (void *)&fh_key_exists },
  { "array_keys", fg_array_keys, (void *)&fh_array_keys },
  { "array_map", fg_array_map, (void *)&fh_array_map },
  { "array_merge_recursive", fg_array_merge_recursive, (void *)&fh_array_merge_recursive },
  { "array_merge", fg_array_merge, (void *)&fh_array_merge },
  { "array_replace_recursive", fg_array_replace_recursive, (void *)&fh_array_replace_recursive },
  { "array_replace", fg_array_replace, (void *)&fh_array_replace },
  { "array_multisort", fg_array_multisort, (void *)&fh_array_multisort },
  { "array_pad", fg_array_pad, (void *)&fh_array_pad },
  { "array_pop", fg_array_pop, (void *)&fh_array_pop },
  { "array_product", fg_array_product, (void *)&fh_array_product },
  { "array_push", fg_array_push, (void *)&fh_array_push },
  { "array_rand", fg_array_rand, (void *)&fh_array_rand },
  { "array_reduce", fg_array_reduce, (void *)&fh_array_reduce },
  { "array_reverse", fg_array_reverse, (void *)&fh_array_reverse },
  { "array_search", fg_array_search, (void *)&fh_array_search },
  { "array_shift", fg_array_shift, (void *)&fh_array_shift },
  { "array_slice", fg_array_slice, (void *)&fh_array_slice },
  { "array_splice", fg_array_splice, (void *)&fh_array_splice },
  { "array_sum", fg_array_sum, (void *)&fh_array_sum },
  { "array_unique", fg_array_unique, (void *)&fh_array_unique },
  { "array_unshift", fg_array_unshift, (void *)&fh_array_unshift },
  { "array_values", fg_array_values, (void *)&fh_array_values },
  { "array_walk_recursive", fg_array_walk_recursive, (void *)&fh_array_walk_recursive },
  { "array_walk", fg_array_walk, (void *)&fh_array_walk },
  { "compact", fg_compact, (void *)&fh_compact },
  { "shuffle", fg_shuffle, (void *)&fh_shuffle },
  { "count", fg_count, (void *)&fh_count },
  { "sizeof", fg_sizeof, (void *)&fh_sizeof },
  { "each", fg_each, (void *)&fh_each },
  { "current", fg_current, (void *)&fh_current },
  { "hphp_current_ref", fg_hphp_current_ref, (void *)&fh_hphp_current_ref },
  { "next", fg_next, (void *)&fh_next },
  { "pos", fg_pos, (void *)&fh_pos },
  { "prev", fg_prev, (void *)&fh_prev },
  { "reset", fg_reset, (void *)&fh_reset },
  { "end", fg_end, (void *)&fh_end },
  { "key", fg_key, (void *)&fh_key },
  { "hphp_get_iterator", fg_hphp_get_iterator, (void *)&fh_hphp_get_iterator },
  { "hphp_get_mutable_iterator", fg_hphp_get_mutable_iterator, (void *)&fh_hphp_get_mutable_iterator },
  { "in_array", fg_in_array, (void *)&fh_in_array },
  { "range", fg_range, (void *)&fh_range },
  { "array_diff", fg_array_diff, (void *)&fh_array_diff },
  { "array_udiff", fg_array_udiff, (void *)&fh_array_udiff },
  { "array_diff_assoc", fg_array_diff_assoc, (void *)&fh_array_diff_assoc },
  { "array_diff_uassoc", fg_array_diff_uassoc, (void *)&fh_array_diff_uassoc },
  { "array_udiff_assoc", fg_array_udiff_assoc, (void *)&fh_array_udiff_assoc },
  { "array_udiff_uassoc", fg_array_udiff_uassoc, (void *)&fh_array_udiff_uassoc },
  { "array_diff_key", fg_array_diff_key, (void *)&fh_array_diff_key },
  { "array_diff_ukey", fg_array_diff_ukey, (void *)&fh_array_diff_ukey },
  { "array_intersect", fg_array_intersect, (void *)&fh_array_intersect },
  { "array_uintersect", fg_array_uintersect, (void *)&fh_array_uintersect },
  { "array_intersect_assoc", fg_array_intersect_assoc, (void *)&fh_array_intersect_assoc },
  { "array_intersect_uassoc", fg_array_intersect_uassoc, (void *)&fh_array_intersect_uassoc },
  { "array_uintersect_assoc", fg_array_uintersect_assoc, (void *)&fh_array_uintersect_assoc },
  { "array_uintersect_uassoc", fg_array_uintersect_uassoc, (void *)&fh_array_uintersect_uassoc },
  { "array_intersect_key", fg_array_intersect_key, (void *)&fh_array_intersect_key },
  { "array_intersect_ukey", fg_array_intersect_ukey, (void *)&fh_array_intersect_ukey },
  { "sort", fg_sort, (void *)&fh_sort },
  { "rsort", fg_rsort, (void *)&fh_rsort },
  { "asort", fg_asort, (void *)&fh_asort },
  { "arsort", fg_arsort, (void *)&fh_arsort },
  { "ksort", fg_ksort, (void *)&fh_ksort },
  { "krsort", fg_krsort, (void *)&fh_krsort },
  { "usort", fg_usort, (void *)&fh_usort },
  { "uasort", fg_uasort, (void *)&fh_uasort },
  { "uksort", fg_uksort, (void *)&fh_uksort },
  { "natsort", fg_natsort, (void *)&fh_natsort },
  { "natcasesort", fg_natcasesort, (void *)&fh_natcasesort },
  { "i18n_loc_get_default", fg_i18n_loc_get_default, (void *)&fh_i18n_loc_get_default },
  { "i18n_loc_set_default", fg_i18n_loc_set_default, (void *)&fh_i18n_loc_set_default },
  { "i18n_loc_set_attribute", fg_i18n_loc_set_attribute, (void *)&fh_i18n_loc_set_attribute },
  { "i18n_loc_set_strength", fg_i18n_loc_set_strength, (void *)&fh_i18n_loc_set_strength },
  { "i18n_loc_get_error_code", fg_i18n_loc_get_error_code, (void *)&fh_i18n_loc_get_error_code },
  { "asio_enter_context", fg_asio_enter_context, (void *)&fh_asio_enter_context },
  { "asio_exit_context", fg_asio_exit_context, (void *)&fh_asio_exit_context },
  { "asio_get_current_context_idx", fg_asio_get_current_context_idx, (void *)&fh_asio_get_current_context_idx },
  { "asio_get_running_in_context", fg_asio_get_running_in_context, (void *)&fh_asio_get_running_in_context },
  { "asio_get_running", fg_asio_get_running, (void *)&fh_asio_get_running },
  { "asio_get_current", fg_asio_get_current, (void *)&fh_asio_get_current },
  { "asio_set_on_failed_callback", fg_asio_set_on_failed_callback, (void *)&fh_asio_set_on_failed_callback },
  { "asio_set_on_started_callback", fg_asio_set_on_started_callback, (void *)&fh_asio_set_on_started_callback },
  { "bcscale", fg_bcscale, (void *)&fh_bcscale },
  { "bcadd", fg_bcadd, (void *)&fh_bcadd },
  { "bcsub", fg_bcsub, (void *)&fh_bcsub },
  { "bccomp", fg_bccomp, (void *)&fh_bccomp },
  { "bcmul", fg_bcmul, (void *)&fh_bcmul },
  { "bcdiv", fg_bcdiv, (void *)&fh_bcdiv },
  { "bcmod", fg_bcmod, (void *)&fh_bcmod },
  { "bcpow", fg_bcpow, (void *)&fh_bcpow },
  { "bcpowmod", fg_bcpowmod, (void *)&fh_bcpowmod },
  { "bcsqrt", fg_bcsqrt, (void *)&fh_bcsqrt },
  { "bzclose", fg_bzclose, (void *)&fh_bzclose },
  { "bzopen", fg_bzopen, (void *)&fh_bzopen },
  { "bzread", fg_bzread, (void *)&fh_bzread },
  { "bzwrite", fg_bzwrite, (void *)&fh_bzwrite },
  { "bzflush", fg_bzflush, (void *)&fh_bzflush },
  { "bzerrstr", fg_bzerrstr, (void *)&fh_bzerrstr },
  { "bzerror", fg_bzerror, (void *)&fh_bzerror },
  { "bzerrno", fg_bzerrno, (void *)&fh_bzerrno },
  { "bzcompress", fg_bzcompress, (void *)&fh_bzcompress },
  { "bzdecompress", fg_bzdecompress, (void *)&fh_bzdecompress },
  { "get_declared_classes", fg_get_declared_classes, (void *)&fh_get_declared_classes },
  { "get_declared_interfaces", fg_get_declared_interfaces, (void *)&fh_get_declared_interfaces },
  { "get_declared_traits", fg_get_declared_traits, (void *)&fh_get_declared_traits },
  { "class_exists", fg_class_exists, (void *)&fh_class_exists },
  { "interface_exists", fg_interface_exists, (void *)&fh_interface_exists },
  { "trait_exists", fg_trait_exists, (void *)&fh_trait_exists },
  { "get_class_methods", fg_get_class_methods, (void *)&fh_get_class_methods },
  { "get_class_vars", fg_get_class_vars, (void *)&fh_get_class_vars },
  { "get_class_constants", fg_get_class_constants, (void *)&fh_get_class_constants },
  { "get_class", fg_get_class, (void *)&fh_get_class },
  { "get_parent_class", fg_get_parent_class, (void *)&fh_get_parent_class },
  { "is_a", fg_is_a, (void *)&fh_is_a },
  { "is_subclass_of", fg_is_subclass_of, (void *)&fh_is_subclass_of },
  { "method_exists", fg_method_exists, (void *)&fh_method_exists },
  { "property_exists", fg_property_exists, (void *)&fh_property_exists },
  { "get_object_vars", fg_get_object_vars, (void *)&fh_get_object_vars },
  { "call_user_method_array", fg_call_user_method_array, (void *)&fh_call_user_method_array },
  { "call_user_method", fg_call_user_method, (void *)&fh_call_user_method },
  { "hphp_create_continuation", fg_hphp_create_continuation, (void *)&fh_hphp_create_continuation },
  { "ctype_alnum", fg_ctype_alnum, (void *)&fh_ctype_alnum },
  { "ctype_alpha", fg_ctype_alpha, (void *)&fh_ctype_alpha },
  { "ctype_cntrl", fg_ctype_cntrl, (void *)&fh_ctype_cntrl },
  { "ctype_digit", fg_ctype_digit, (void *)&fh_ctype_digit },
  { "ctype_graph", fg_ctype_graph, (void *)&fh_ctype_graph },
  { "ctype_lower", fg_ctype_lower, (void *)&fh_ctype_lower },
  { "ctype_print", fg_ctype_print, (void *)&fh_ctype_print },
  { "ctype_punct", fg_ctype_punct, (void *)&fh_ctype_punct },
  { "ctype_space", fg_ctype_space, (void *)&fh_ctype_space },
  { "ctype_upper", fg_ctype_upper, (void *)&fh_ctype_upper },
  { "ctype_xdigit", fg_ctype_xdigit, (void *)&fh_ctype_xdigit },
  { "curl_init", fg_curl_init, (void *)&fh_curl_init },
  { "curl_copy_handle", fg_curl_copy_handle, (void *)&fh_curl_copy_handle },
  { "curl_version", fg_curl_version, (void *)&fh_curl_version },
  { "curl_setopt", fg_curl_setopt, (void *)&fh_curl_setopt },
  { "curl_setopt_array", fg_curl_setopt_array, (void *)&fh_curl_setopt_array },
  { "fb_curl_getopt", fg_fb_curl_getopt, (void *)&fh_fb_curl_getopt },
  { "curl_exec", fg_curl_exec, (void *)&fh_curl_exec },
  { "curl_getinfo", fg_curl_getinfo, (void *)&fh_curl_getinfo },
  { "curl_errno", fg_curl_errno, (void *)&fh_curl_errno },
  { "curl_error", fg_curl_error, (void *)&fh_curl_error },
  { "curl_close", fg_curl_close, (void *)&fh_curl_close },
  { "curl_multi_init", fg_curl_multi_init, (void *)&fh_curl_multi_init },
  { "curl_multi_add_handle", fg_curl_multi_add_handle, (void *)&fh_curl_multi_add_handle },
  { "curl_multi_remove_handle", fg_curl_multi_remove_handle, (void *)&fh_curl_multi_remove_handle },
  { "curl_multi_exec", fg_curl_multi_exec, (void *)&fh_curl_multi_exec },
  { "curl_multi_select", fg_curl_multi_select, (void *)&fh_curl_multi_select },
  { "fb_curl_multi_fdset", fg_fb_curl_multi_fdset, (void *)&fh_fb_curl_multi_fdset },
  { "curl_multi_getcontent", fg_curl_multi_getcontent, (void *)&fh_curl_multi_getcontent },
  { "curl_multi_info_read", fg_curl_multi_info_read, (void *)&fh_curl_multi_info_read },
  { "curl_multi_close", fg_curl_multi_close, (void *)&fh_curl_multi_close },
  { "evhttp_set_cache", fg_evhttp_set_cache, (void *)&fh_evhttp_set_cache },
  { "evhttp_get", fg_evhttp_get, (void *)&fh_evhttp_get },
  { "evhttp_post", fg_evhttp_post, (void *)&fh_evhttp_post },
  { "evhttp_async_get", fg_evhttp_async_get, (void *)&fh_evhttp_async_get },
  { "evhttp_async_post", fg_evhttp_async_post, (void *)&fh_evhttp_async_post },
  { "evhttp_recv", fg_evhttp_recv, (void *)&fh_evhttp_recv },
  { "checkdate", fg_checkdate, (void *)&fh_checkdate },
  { "date_add", fg_date_add, (void *)&fh_date_add },
  { "date_create_from_format", fg_date_create_from_format, (void *)&fh_date_create_from_format },
  { "date_create", fg_date_create, (void *)&fh_date_create },
  { "date_date_set", fg_date_date_set, (void *)&fh_date_date_set },
  { "date_default_timezone_get", fg_date_default_timezone_get, (void *)&fh_date_default_timezone_get },
  { "date_default_timezone_set", fg_date_default_timezone_set, (void *)&fh_date_default_timezone_set },
  { "date_diff", fg_date_diff, (void *)&fh_date_diff },
  { "date_format", fg_date_format, (void *)&fh_date_format },
  { "date_get_last_errors", fg_date_get_last_errors, (void *)&fh_date_get_last_errors },
  { "date_interval_create_from_date_string", fg_date_interval_create_from_date_string, (void *)&fh_date_interval_create_from_date_string },
  { "date_interval_format", fg_date_interval_format, (void *)&fh_date_interval_format },
  { "date_isodate_set", fg_date_isodate_set, (void *)&fh_date_isodate_set },
  { "date_modify", fg_date_modify, (void *)&fh_date_modify },
  { "date_offset_get", fg_date_offset_get, (void *)&fh_date_offset_get },
  { "date_parse", fg_date_parse, (void *)&fh_date_parse },
  { "date_sub", fg_date_sub, (void *)&fh_date_sub },
  { "date_sun_info", fg_date_sun_info, (void *)&fh_date_sun_info },
  { "date_sunrise", fg_date_sunrise, (void *)&fh_date_sunrise },
  { "date_sunset", fg_date_sunset, (void *)&fh_date_sunset },
  { "date_time_set", fg_date_time_set, (void *)&fh_date_time_set },
  { "date_timestamp_get", fg_date_timestamp_get, (void *)&fh_date_timestamp_get },
  { "date_timestamp_set", fg_date_timestamp_set, (void *)&fh_date_timestamp_set },
  { "date_timezone_get", fg_date_timezone_get, (void *)&fh_date_timezone_get },
  { "date_timezone_set", fg_date_timezone_set, (void *)&fh_date_timezone_set },
  { "date", fg_date, (void *)&fh_date },
  { "getdate", fg_getdate, (void *)&fh_getdate },
  { "gettimeofday", fg_gettimeofday, (void *)&fh_gettimeofday },
  { "gmdate", fg_gmdate, (void *)&fh_gmdate },
  { "gmmktime", fg_gmmktime, (void *)&fh_gmmktime },
  { "gmstrftime", fg_gmstrftime, (void *)&fh_gmstrftime },
  { "idate", fg_idate, (void *)&fh_idate },
  { "localtime", fg_localtime, (void *)&fh_localtime },
  { "microtime", fg_microtime, (void *)&fh_microtime },
  { "mktime", fg_mktime, (void *)&fh_mktime },
  { "strftime", fg_strftime, (void *)&fh_strftime },
  { "strptime", fg_strptime, (void *)&fh_strptime },
  { "strtotime", fg_strtotime, (void *)&fh_strtotime },
  { "time", fg_time, (void *)&fh_time },
  { "timezone_abbreviations_list", fg_timezone_abbreviations_list, (void *)&fh_timezone_abbreviations_list },
  { "timezone_identifiers_list", fg_timezone_identifiers_list, (void *)&fh_timezone_identifiers_list },
  { "timezone_location_get", fg_timezone_location_get, (void *)&fh_timezone_location_get },
  { "timezone_name_from_abbr", fg_timezone_name_from_abbr, (void *)&fh_timezone_name_from_abbr },
  { "timezone_name_get", fg_timezone_name_get, (void *)&fh_timezone_name_get },
  { "timezone_offset_get", fg_timezone_offset_get, (void *)&fh_timezone_offset_get },
  { "timezone_open", fg_timezone_open, (void *)&fh_timezone_open },
  { "timezone_transitions_get", fg_timezone_transitions_get, (void *)&fh_timezone_transitions_get },
  { "timezone_version_get", fg_timezone_version_get, (void *)&fh_timezone_version_get },
  { "hphpd_install_user_command", fg_hphpd_install_user_command, (void *)&fh_hphpd_install_user_command },
  { "hphpd_get_user_commands", fg_hphpd_get_user_commands, (void *)&fh_hphpd_get_user_commands },
  { "hphpd_break", fg_hphpd_break, (void *)&fh_hphpd_break },
  { "hphpd_get_client", fg_hphpd_get_client, (void *)&fh_hphpd_get_client },
  { "hphpd_client_ctrl", fg_hphpd_client_ctrl, (void *)&fh_hphpd_client_ctrl },
  { "dom_document_create_element", fg_dom_document_create_element, (void *)&fh_dom_document_create_element },
  { "dom_document_create_document_fragment", fg_dom_document_create_document_fragment, (void *)&fh_dom_document_create_document_fragment },
  { "dom_document_create_text_node", fg_dom_document_create_text_node, (void *)&fh_dom_document_create_text_node },
  { "dom_document_create_comment", fg_dom_document_create_comment, (void *)&fh_dom_document_create_comment },
  { "dom_document_create_cdatasection", fg_dom_document_create_cdatasection, (void *)&fh_dom_document_create_cdatasection },
  { "dom_document_create_processing_instruction", fg_dom_document_create_processing_instruction, (void *)&fh_dom_document_create_processing_instruction },
  { "dom_document_create_attribute", fg_dom_document_create_attribute, (void *)&fh_dom_document_create_attribute },
  { "dom_document_create_entity_reference", fg_dom_document_create_entity_reference, (void *)&fh_dom_document_create_entity_reference },
  { "dom_document_get_elements_by_tag_name", fg_dom_document_get_elements_by_tag_name, (void *)&fh_dom_document_get_elements_by_tag_name },
  { "dom_document_import_node", fg_dom_document_import_node, (void *)&fh_dom_document_import_node },
  { "dom_document_create_element_ns", fg_dom_document_create_element_ns, (void *)&fh_dom_document_create_element_ns },
  { "dom_document_create_attribute_ns", fg_dom_document_create_attribute_ns, (void *)&fh_dom_document_create_attribute_ns },
  { "dom_document_get_elements_by_tag_name_ns", fg_dom_document_get_elements_by_tag_name_ns, (void *)&fh_dom_document_get_elements_by_tag_name_ns },
  { "dom_document_get_element_by_id", fg_dom_document_get_element_by_id, (void *)&fh_dom_document_get_element_by_id },
  { "dom_document_normalize_document", fg_dom_document_normalize_document, (void *)&fh_dom_document_normalize_document },
  { "dom_document_save", fg_dom_document_save, (void *)&fh_dom_document_save },
  { "dom_document_savexml", fg_dom_document_savexml, (void *)&fh_dom_document_savexml },
  { "dom_document_validate", fg_dom_document_validate, (void *)&fh_dom_document_validate },
  { "dom_document_xinclude", fg_dom_document_xinclude, (void *)&fh_dom_document_xinclude },
  { "dom_document_save_html", fg_dom_document_save_html, (void *)&fh_dom_document_save_html },
  { "dom_document_save_html_file", fg_dom_document_save_html_file, (void *)&fh_dom_document_save_html_file },
  { "dom_document_schema_validate_file", fg_dom_document_schema_validate_file, (void *)&fh_dom_document_schema_validate_file },
  { "dom_document_schema_validate_xml", fg_dom_document_schema_validate_xml, (void *)&fh_dom_document_schema_validate_xml },
  { "dom_document_relaxng_validate_file", fg_dom_document_relaxng_validate_file, (void *)&fh_dom_document_relaxng_validate_file },
  { "dom_document_relaxng_validate_xml", fg_dom_document_relaxng_validate_xml, (void *)&fh_dom_document_relaxng_validate_xml },
  { "dom_node_insert_before", fg_dom_node_insert_before, (void *)&fh_dom_node_insert_before },
  { "dom_node_replace_child", fg_dom_node_replace_child, (void *)&fh_dom_node_replace_child },
  { "dom_node_remove_child", fg_dom_node_remove_child, (void *)&fh_dom_node_remove_child },
  { "dom_node_append_child", fg_dom_node_append_child, (void *)&fh_dom_node_append_child },
  { "dom_node_has_child_nodes", fg_dom_node_has_child_nodes, (void *)&fh_dom_node_has_child_nodes },
  { "dom_node_clone_node", fg_dom_node_clone_node, (void *)&fh_dom_node_clone_node },
  { "dom_node_normalize", fg_dom_node_normalize, (void *)&fh_dom_node_normalize },
  { "dom_node_is_supported", fg_dom_node_is_supported, (void *)&fh_dom_node_is_supported },
  { "dom_node_has_attributes", fg_dom_node_has_attributes, (void *)&fh_dom_node_has_attributes },
  { "dom_node_is_same_node", fg_dom_node_is_same_node, (void *)&fh_dom_node_is_same_node },
  { "dom_node_lookup_prefix", fg_dom_node_lookup_prefix, (void *)&fh_dom_node_lookup_prefix },
  { "dom_node_is_default_namespace", fg_dom_node_is_default_namespace, (void *)&fh_dom_node_is_default_namespace },
  { "dom_node_lookup_namespace_uri", fg_dom_node_lookup_namespace_uri, (void *)&fh_dom_node_lookup_namespace_uri },
  { "dom_nodelist_item", fg_dom_nodelist_item, (void *)&fh_dom_nodelist_item },
  { "dom_namednodemap_get_named_item", fg_dom_namednodemap_get_named_item, (void *)&fh_dom_namednodemap_get_named_item },
  { "dom_namednodemap_item", fg_dom_namednodemap_item, (void *)&fh_dom_namednodemap_item },
  { "dom_namednodemap_get_named_item_ns", fg_dom_namednodemap_get_named_item_ns, (void *)&fh_dom_namednodemap_get_named_item_ns },
  { "dom_characterdata_substring_data", fg_dom_characterdata_substring_data, (void *)&fh_dom_characterdata_substring_data },
  { "dom_characterdata_append_data", fg_dom_characterdata_append_data, (void *)&fh_dom_characterdata_append_data },
  { "dom_characterdata_insert_data", fg_dom_characterdata_insert_data, (void *)&fh_dom_characterdata_insert_data },
  { "dom_characterdata_delete_data", fg_dom_characterdata_delete_data, (void *)&fh_dom_characterdata_delete_data },
  { "dom_characterdata_replace_data", fg_dom_characterdata_replace_data, (void *)&fh_dom_characterdata_replace_data },
  { "dom_attr_is_id", fg_dom_attr_is_id, (void *)&fh_dom_attr_is_id },
  { "dom_element_get_attribute", fg_dom_element_get_attribute, (void *)&fh_dom_element_get_attribute },
  { "dom_element_set_attribute", fg_dom_element_set_attribute, (void *)&fh_dom_element_set_attribute },
  { "dom_element_remove_attribute", fg_dom_element_remove_attribute, (void *)&fh_dom_element_remove_attribute },
  { "dom_element_get_attribute_node", fg_dom_element_get_attribute_node, (void *)&fh_dom_element_get_attribute_node },
  { "dom_element_set_attribute_node", fg_dom_element_set_attribute_node, (void *)&fh_dom_element_set_attribute_node },
  { "dom_element_remove_attribute_node", fg_dom_element_remove_attribute_node, (void *)&fh_dom_element_remove_attribute_node },
  { "dom_element_get_elements_by_tag_name", fg_dom_element_get_elements_by_tag_name, (void *)&fh_dom_element_get_elements_by_tag_name },
  { "dom_element_get_attribute_ns", fg_dom_element_get_attribute_ns, (void *)&fh_dom_element_get_attribute_ns },
  { "dom_element_set_attribute_ns", fg_dom_element_set_attribute_ns, (void *)&fh_dom_element_set_attribute_ns },
  { "dom_element_remove_attribute_ns", fg_dom_element_remove_attribute_ns, (void *)&fh_dom_element_remove_attribute_ns },
  { "dom_element_get_attribute_node_ns", fg_dom_element_get_attribute_node_ns, (void *)&fh_dom_element_get_attribute_node_ns },
  { "dom_element_set_attribute_node_ns", fg_dom_element_set_attribute_node_ns, (void *)&fh_dom_element_set_attribute_node_ns },
  { "dom_element_get_elements_by_tag_name_ns", fg_dom_element_get_elements_by_tag_name_ns, (void *)&fh_dom_element_get_elements_by_tag_name_ns },
  { "dom_element_has_attribute", fg_dom_element_has_attribute, (void *)&fh_dom_element_has_attribute },
  { "dom_element_has_attribute_ns", fg_dom_element_has_attribute_ns, (void *)&fh_dom_element_has_attribute_ns },
  { "dom_element_set_id_attribute", fg_dom_element_set_id_attribute, (void *)&fh_dom_element_set_id_attribute },
  { "dom_element_set_id_attribute_ns", fg_dom_element_set_id_attribute_ns, (void *)&fh_dom_element_set_id_attribute_ns },
  { "dom_element_set_id_attribute_node", fg_dom_element_set_id_attribute_node, (void *)&fh_dom_element_set_id_attribute_node },
  { "dom_text_split_text", fg_dom_text_split_text, (void *)&fh_dom_text_split_text },
  { "dom_text_is_whitespace_in_element_content", fg_dom_text_is_whitespace_in_element_content, (void *)&fh_dom_text_is_whitespace_in_element_content },
  { "dom_xpath_register_ns", fg_dom_xpath_register_ns, (void *)&fh_dom_xpath_register_ns },
  { "dom_xpath_query", fg_dom_xpath_query, (void *)&fh_dom_xpath_query },
  { "dom_xpath_evaluate", fg_dom_xpath_evaluate, (void *)&fh_dom_xpath_evaluate },
  { "dom_xpath_register_php_functions", fg_dom_xpath_register_php_functions, (void *)&fh_dom_xpath_register_php_functions },
  { "debug_backtrace", fg_debug_backtrace, (void *)&fh_debug_backtrace },
  { "debug_print_backtrace", fg_debug_print_backtrace, (void *)&fh_debug_print_backtrace },
  { "error_get_last", fg_error_get_last, (void *)&fh_error_get_last },
  { "error_log", fg_error_log, (void *)&fh_error_log },
  { "error_reporting", fg_error_reporting, (void *)&fh_error_reporting },
  { "restore_error_handler", fg_restore_error_handler, (void *)&fh_restore_error_handler },
  { "restore_exception_handler", fg_restore_exception_handler, (void *)&fh_restore_exception_handler },
  { "set_error_handler", fg_set_error_handler, (void *)&fh_set_error_handler },
  { "set_exception_handler", fg_set_exception_handler, (void *)&fh_set_exception_handler },
  { "hphp_set_error_page", fg_hphp_set_error_page, (void *)&fh_hphp_set_error_page },
  { "hphp_throw_fatal_error", fg_hphp_throw_fatal_error, (void *)&fh_hphp_throw_fatal_error },
  { "hphp_clear_unflushed", fg_hphp_clear_unflushed, (void *)&fh_hphp_clear_unflushed },
  { "hphp_debug_caller_info", fg_hphp_debug_caller_info, (void *)&fh_hphp_debug_caller_info },
  { "trigger_error", fg_trigger_error, (void *)&fh_trigger_error },
  { "user_error", fg_user_error, (void *)&fh_user_error },
  { "fb_thrift_serialize", fg_fb_thrift_serialize, (void *)&fh_fb_thrift_serialize },
  { "fb_thrift_unserialize", fg_fb_thrift_unserialize, (void *)&fh_fb_thrift_unserialize },
  { "fb_serialize", fg_fb_serialize, (void *)&fh_fb_serialize },
  { "fb_unserialize", fg_fb_unserialize, (void *)&fh_fb_unserialize },
  { "fb_compact_serialize", fg_fb_compact_serialize, (void *)&fh_fb_compact_serialize },
  { "fb_compact_unserialize", fg_fb_compact_unserialize, (void *)&fh_fb_compact_unserialize },
  { "fb_could_include", fg_fb_could_include, (void *)&fh_fb_could_include },
  { "fb_intercept", fg_fb_intercept, (void *)&fh_fb_intercept },
  { "fb_stubout_intercept_handler", fg_fb_stubout_intercept_handler, (void *)&fh_fb_stubout_intercept_handler },
  { "fb_rpc_intercept_handler", fg_fb_rpc_intercept_handler, (void *)&fh_fb_rpc_intercept_handler },
  { "fb_renamed_functions", fg_fb_renamed_functions, (void *)&fh_fb_renamed_functions },
  { "fb_rename_function", fg_fb_rename_function, (void *)&fh_fb_rename_function },
  { "fb_autoload_map", fg_fb_autoload_map, (void *)&fh_fb_autoload_map },
  { "fb_utf8ize", fg_fb_utf8ize, (void *)&fh_fb_utf8ize },
  { "fb_utf8_strlen_deprecated", fg_fb_utf8_strlen_deprecated, (void *)&fh_fb_utf8_strlen_deprecated },
  { "fb_utf8_strlen", fg_fb_utf8_strlen, (void *)&fh_fb_utf8_strlen },
  { "fb_utf8_substr", fg_fb_utf8_substr, (void *)&fh_fb_utf8_substr },
  { "fb_call_user_func_safe", fg_fb_call_user_func_safe, (void *)&fh_fb_call_user_func_safe },
  { "fb_call_user_func_safe_return", fg_fb_call_user_func_safe_return, (void *)&fh_fb_call_user_func_safe_return },
  { "fb_call_user_func_array_safe", fg_fb_call_user_func_array_safe, (void *)&fh_fb_call_user_func_array_safe },
  { "fb_get_code_coverage", fg_fb_get_code_coverage, (void *)&fh_fb_get_code_coverage },
  { "fb_enable_code_coverage", fg_fb_enable_code_coverage, (void *)&fh_fb_enable_code_coverage },
  { "fb_disable_code_coverage", fg_fb_disable_code_coverage, (void *)&fh_fb_disable_code_coverage },
  { "xhprof_enable", fg_xhprof_enable, (void *)&fh_xhprof_enable },
  { "xhprof_disable", fg_xhprof_disable, (void *)&fh_xhprof_disable },
  { "xhprof_network_enable", fg_xhprof_network_enable, (void *)&fh_xhprof_network_enable },
  { "xhprof_network_disable", fg_xhprof_network_disable, (void *)&fh_xhprof_network_disable },
  { "xhprof_frame_begin", fg_xhprof_frame_begin, (void *)&fh_xhprof_frame_begin },
  { "xhprof_frame_end", fg_xhprof_frame_end, (void *)&fh_xhprof_frame_end },
  { "xhprof_run_trace", fg_xhprof_run_trace, (void *)&fh_xhprof_run_trace },
  { "xhprof_sample_enable", fg_xhprof_sample_enable, (void *)&fh_xhprof_sample_enable },
  { "xhprof_sample_disable", fg_xhprof_sample_disable, (void *)&fh_xhprof_sample_disable },
  { "fb_load_local_databases", fg_fb_load_local_databases, (void *)&fh_fb_load_local_databases },
  { "fb_parallel_query", fg_fb_parallel_query, (void *)&fh_fb_parallel_query },
  { "fb_crossall_query", fg_fb_crossall_query, (void *)&fh_fb_crossall_query },
  { "fb_const_fetch", fg_fb_const_fetch, (void *)&fh_fb_const_fetch },
  { "fb_output_compression", fg_fb_output_compression, (void *)&fh_fb_output_compression },
  { "fb_set_exit_callback", fg_fb_set_exit_callback, (void *)&fh_fb_set_exit_callback },
  { "fb_get_flush_stat", fg_fb_get_flush_stat, (void *)&fh_fb_get_flush_stat },
  { "fb_get_last_flush_size", fg_fb_get_last_flush_size, (void *)&fh_fb_get_last_flush_size },
  { "fb_lazy_stat", fg_fb_lazy_stat, (void *)&fh_fb_lazy_stat },
  { "fb_lazy_lstat", fg_fb_lazy_lstat, (void *)&fh_fb_lazy_lstat },
  { "fb_lazy_realpath", fg_fb_lazy_realpath, (void *)&fh_fb_lazy_realpath },
  { "fb_setprofile", fg_fb_setprofile, (void *)&fh_fb_setprofile },
  { "fb_gc_collect_cycles", fg_fb_gc_collect_cycles, (void *)&fh_fb_gc_collect_cycles },
  { "fb_gc_detect_cycles", fg_fb_gc_detect_cycles, (void *)&fh_fb_gc_detect_cycles },
  { "fopen", fg_fopen, (void *)&fh_fopen },
  { "popen", fg_popen, (void *)&fh_popen },
  { "fclose", fg_fclose, (void *)&fh_fclose },
  { "pclose", fg_pclose, (void *)&fh_pclose },
  { "fseek", fg_fseek, (void *)&fh_fseek },
  { "rewind", fg_rewind, (void *)&fh_rewind },
  { "ftell", fg_ftell, (void *)&fh_ftell },
  { "feof", fg_feof, (void *)&fh_feof },
  { "fstat", fg_fstat, (void *)&fh_fstat },
  { "fread", fg_fread, (void *)&fh_fread },
  { "fgetc", fg_fgetc, (void *)&fh_fgetc },
  { "fgets", fg_fgets, (void *)&fh_fgets },
  { "fgetss", fg_fgetss, (void *)&fh_fgetss },
  { "fscanf", fg_fscanf, (void *)&fh_fscanf },
  { "fpassthru", fg_fpassthru, (void *)&fh_fpassthru },
  { "fwrite", fg_fwrite, (void *)&fh_fwrite },
  { "fputs", fg_fputs, (void *)&fh_fputs },
  { "fprintf", fg_fprintf, (void *)&fh_fprintf },
  { "vfprintf", fg_vfprintf, (void *)&fh_vfprintf },
  { "fflush", fg_fflush, (void *)&fh_fflush },
  { "ftruncate", fg_ftruncate, (void *)&fh_ftruncate },
  { "flock", fg_flock, (void *)&fh_flock },
  { "fputcsv", fg_fputcsv, (void *)&fh_fputcsv },
  { "fgetcsv", fg_fgetcsv, (void *)&fh_fgetcsv },
  { "file_get_contents", fg_file_get_contents, (void *)&fh_file_get_contents },
  { "file_put_contents", fg_file_put_contents, (void *)&fh_file_put_contents },
  { "file", fg_file, (void *)&fh_file },
  { "readfile", fg_readfile, (void *)&fh_readfile },
  { "move_uploaded_file", fg_move_uploaded_file, (void *)&fh_move_uploaded_file },
  { "parse_ini_file", fg_parse_ini_file, (void *)&fh_parse_ini_file },
  { "parse_ini_string", fg_parse_ini_string, (void *)&fh_parse_ini_string },
  { "parse_hdf_file", fg_parse_hdf_file, (void *)&fh_parse_hdf_file },
  { "parse_hdf_string", fg_parse_hdf_string, (void *)&fh_parse_hdf_string },
  { "write_hdf_file", fg_write_hdf_file, (void *)&fh_write_hdf_file },
  { "write_hdf_string", fg_write_hdf_string, (void *)&fh_write_hdf_string },
  { "md5_file", fg_md5_file, (void *)&fh_md5_file },
  { "sha1_file", fg_sha1_file, (void *)&fh_sha1_file },
  { "chmod", fg_chmod, (void *)&fh_chmod },
  { "chown", fg_chown, (void *)&fh_chown },
  { "lchown", fg_lchown, (void *)&fh_lchown },
  { "chgrp", fg_chgrp, (void *)&fh_chgrp },
  { "lchgrp", fg_lchgrp, (void *)&fh_lchgrp },
  { "touch", fg_touch, (void *)&fh_touch },
  { "copy", fg_copy, (void *)&fh_copy },
  { "rename", fg_rename, (void *)&fh_rename },
  { "umask", fg_umask, (void *)&fh_umask },
  { "unlink", fg_unlink, (void *)&fh_unlink },
  { "link", fg_link, (void *)&fh_link },
  { "symlink", fg_symlink, (void *)&fh_symlink },
  { "basename", fg_basename, (void *)&fh_basename },
  { "fnmatch", fg_fnmatch, (void *)&fh_fnmatch },
  { "glob", fg_glob, (void *)&fh_glob },
  { "tempnam", fg_tempnam, (void *)&fh_tempnam },
  { "tmpfile", fg_tmpfile, (void *)&fh_tmpfile },
  { "fileperms", fg_fileperms, (void *)&fh_fileperms },
  { "fileinode", fg_fileinode, (void *)&fh_fileinode },
  { "filesize", fg_filesize, (void *)&fh_filesize },
  { "fileowner", fg_fileowner, (void *)&fh_fileowner },
  { "filegroup", fg_filegroup, (void *)&fh_filegroup },
  { "fileatime", fg_fileatime, (void *)&fh_fileatime },
  { "filemtime", fg_filemtime, (void *)&fh_filemtime },
  { "filectime", fg_filectime, (void *)&fh_filectime },
  { "filetype", fg_filetype, (void *)&fh_filetype },
  { "linkinfo", fg_linkinfo, (void *)&fh_linkinfo },
  { "is_writable", fg_is_writable, (void *)&fh_is_writable },
  { "is_writeable", fg_is_writeable, (void *)&fh_is_writeable },
  { "is_readable", fg_is_readable, (void *)&fh_is_readable },
  { "is_executable", fg_is_executable, (void *)&fh_is_executable },
  { "is_file", fg_is_file, (void *)&fh_is_file },
  { "is_dir", fg_is_dir, (void *)&fh_is_dir },
  { "is_link", fg_is_link, (void *)&fh_is_link },
  { "is_uploaded_file", fg_is_uploaded_file, (void *)&fh_is_uploaded_file },
  { "file_exists", fg_file_exists, (void *)&fh_file_exists },
  { "stat", fg_stat, (void *)&fh_stat },
  { "lstat", fg_lstat, (void *)&fh_lstat },
  { "clearstatcache", fg_clearstatcache, (void *)&fh_clearstatcache },
  { "readlink", fg_readlink, (void *)&fh_readlink },
  { "realpath", fg_realpath, (void *)&fh_realpath },
  { "pathinfo", fg_pathinfo, (void *)&fh_pathinfo },
  { "disk_free_space", fg_disk_free_space, (void *)&fh_disk_free_space },
  { "diskfreespace", fg_diskfreespace, (void *)&fh_diskfreespace },
  { "disk_total_space", fg_disk_total_space, (void *)&fh_disk_total_space },
  { "mkdir", fg_mkdir, (void *)&fh_mkdir },
  { "rmdir", fg_rmdir, (void *)&fh_rmdir },
  { "dirname", fg_dirname, (void *)&fh_dirname },
  { "getcwd", fg_getcwd, (void *)&fh_getcwd },
  { "chdir", fg_chdir, (void *)&fh_chdir },
  { "chroot", fg_chroot, (void *)&fh_chroot },
  { "dir", fg_dir, (void *)&fh_dir },
  { "opendir", fg_opendir, (void *)&fh_opendir },
  { "readdir", fg_readdir, (void *)&fh_readdir },
  { "rewinddir", fg_rewinddir, (void *)&fh_rewinddir },
  { "scandir", fg_scandir, (void *)&fh_scandir },
  { "closedir", fg_closedir, (void *)&fh_closedir },
  { "get_defined_functions", fg_get_defined_functions, (void *)&fh_get_defined_functions },
  { "function_exists", fg_function_exists, (void *)&fh_function_exists },
  { "is_callable", fg_is_callable, (void *)&fh_is_callable },
  { "call_user_func_array", fg_call_user_func_array, (void *)&fh_call_user_func_array },
  { "call_user_func", fg_call_user_func, (void *)&fh_call_user_func },
  { "call_user_func_array_async", fg_call_user_func_array_async, (void *)&fh_call_user_func_array_async },
  { "call_user_func_async", fg_call_user_func_async, (void *)&fh_call_user_func_async },
  { "check_user_func_async", fg_check_user_func_async, (void *)&fh_check_user_func_async },
  { "end_user_func_async", fg_end_user_func_async, (void *)&fh_end_user_func_async },
  { "call_user_func_serialized", fg_call_user_func_serialized, (void *)&fh_call_user_func_serialized },
  { "call_user_func_array_rpc", fg_call_user_func_array_rpc, (void *)&fh_call_user_func_array_rpc },
  { "call_user_func_rpc", fg_call_user_func_rpc, (void *)&fh_call_user_func_rpc },
  { "forward_static_call_array", fg_forward_static_call_array, (void *)&fh_forward_static_call_array },
  { "forward_static_call", fg_forward_static_call, (void *)&fh_forward_static_call },
  { "get_called_class", fg_get_called_class, (void *)&fh_get_called_class },
  { "create_function", fg_create_function, (void *)&fh_create_function },
  { "func_get_arg", fg_func_get_arg, (void *)&fh_func_get_arg },
  { "func_get_args", fg_func_get_args, (void *)&fh_func_get_args },
  { "func_num_args", fg_func_num_args, (void *)&fh_func_num_args },
  { "register_postsend_function", fg_register_postsend_function, (void *)&fh_register_postsend_function },
  { "register_shutdown_function", fg_register_shutdown_function, (void *)&fh_register_shutdown_function },
  { "register_cleanup_function", fg_register_cleanup_function, (void *)&fh_register_cleanup_function },
  { "register_tick_function", fg_register_tick_function, (void *)&fh_register_tick_function },
  { "unregister_tick_function", fg_unregister_tick_function, (void *)&fh_unregister_tick_function },
  { "hash", fg_hash, (void *)&fh_hash },
  { "hash_algos", fg_hash_algos, (void *)&fh_hash_algos },
  { "hash_init", fg_hash_init, (void *)&fh_hash_init },
  { "hash_file", fg_hash_file, (void *)&fh_hash_file },
  { "hash_final", fg_hash_final, (void *)&fh_hash_final },
  { "hash_hmac_file", fg_hash_hmac_file, (void *)&fh_hash_hmac_file },
  { "hash_hmac", fg_hash_hmac, (void *)&fh_hash_hmac },
  { "hash_update_file", fg_hash_update_file, (void *)&fh_hash_update_file },
  { "hash_update_stream", fg_hash_update_stream, (void *)&fh_hash_update_stream },
  { "hash_update", fg_hash_update, (void *)&fh_hash_update },
  { "furchash_hphp_ext", fg_furchash_hphp_ext, (void *)&fh_furchash_hphp_ext },
  { "furchash_hphp_ext_supported", fg_furchash_hphp_ext_supported, (void *)&fh_furchash_hphp_ext_supported },
  { "hphp_murmurhash", fg_hphp_murmurhash, (void *)&fh_hphp_murmurhash },
  { "iconv_mime_encode", fg_iconv_mime_encode, (void *)&fh_iconv_mime_encode },
  { "iconv_mime_decode", fg_iconv_mime_decode, (void *)&fh_iconv_mime_decode },
  { "iconv_mime_decode_headers", fg_iconv_mime_decode_headers, (void *)&fh_iconv_mime_decode_headers },
  { "iconv_get_encoding", fg_iconv_get_encoding, (void *)&fh_iconv_get_encoding },
  { "iconv_set_encoding", fg_iconv_set_encoding, (void *)&fh_iconv_set_encoding },
  { "iconv", fg_iconv, (void *)&fh_iconv },
  { "iconv_strlen", fg_iconv_strlen, (void *)&fh_iconv_strlen },
  { "iconv_strpos", fg_iconv_strpos, (void *)&fh_iconv_strpos },
  { "iconv_strrpos", fg_iconv_strrpos, (void *)&fh_iconv_strrpos },
  { "iconv_substr", fg_iconv_substr, (void *)&fh_iconv_substr },
  { "ob_iconv_handler", fg_ob_iconv_handler, (void *)&fh_ob_iconv_handler },
  { "icu_match", fg_icu_match, (void *)&fh_icu_match },
  { "icu_transliterate", fg_icu_transliterate, (void *)&fh_icu_transliterate },
  { "icu_tokenize", fg_icu_tokenize, (void *)&fh_icu_tokenize },
  { "gd_info", fg_gd_info, (void *)&fh_gd_info },
  { "getimagesize", fg_getimagesize, (void *)&fh_getimagesize },
  { "image_type_to_extension", fg_image_type_to_extension, (void *)&fh_image_type_to_extension },
  { "image_type_to_mime_type", fg_image_type_to_mime_type, (void *)&fh_image_type_to_mime_type },
  { "image2wbmp", fg_image2wbmp, (void *)&fh_image2wbmp },
  { "imagealphablending", fg_imagealphablending, (void *)&fh_imagealphablending },
  { "imageantialias", fg_imageantialias, (void *)&fh_imageantialias },
  { "imagearc", fg_imagearc, (void *)&fh_imagearc },
  { "imagechar", fg_imagechar, (void *)&fh_imagechar },
  { "imagecharup", fg_imagecharup, (void *)&fh_imagecharup },
  { "imagecolorallocate", fg_imagecolorallocate, (void *)&fh_imagecolorallocate },
  { "imagecolorallocatealpha", fg_imagecolorallocatealpha, (void *)&fh_imagecolorallocatealpha },
  { "imagecolorat", fg_imagecolorat, (void *)&fh_imagecolorat },
  { "imagecolorclosest", fg_imagecolorclosest, (void *)&fh_imagecolorclosest },
  { "imagecolorclosestalpha", fg_imagecolorclosestalpha, (void *)&fh_imagecolorclosestalpha },
  { "imagecolorclosesthwb", fg_imagecolorclosesthwb, (void *)&fh_imagecolorclosesthwb },
  { "imagecolordeallocate", fg_imagecolordeallocate, (void *)&fh_imagecolordeallocate },
  { "imagecolorexact", fg_imagecolorexact, (void *)&fh_imagecolorexact },
  { "imagecolorexactalpha", fg_imagecolorexactalpha, (void *)&fh_imagecolorexactalpha },
  { "imagecolormatch", fg_imagecolormatch, (void *)&fh_imagecolormatch },
  { "imagecolorresolve", fg_imagecolorresolve, (void *)&fh_imagecolorresolve },
  { "imagecolorresolvealpha", fg_imagecolorresolvealpha, (void *)&fh_imagecolorresolvealpha },
  { "imagecolorset", fg_imagecolorset, (void *)&fh_imagecolorset },
  { "imagecolorsforindex", fg_imagecolorsforindex, (void *)&fh_imagecolorsforindex },
  { "imagecolorstotal", fg_imagecolorstotal, (void *)&fh_imagecolorstotal },
  { "imagecolortransparent", fg_imagecolortransparent, (void *)&fh_imagecolortransparent },
  { "imageconvolution", fg_imageconvolution, (void *)&fh_imageconvolution },
  { "imagecopy", fg_imagecopy, (void *)&fh_imagecopy },
  { "imagecopymerge", fg_imagecopymerge, (void *)&fh_imagecopymerge },
  { "imagecopymergegray", fg_imagecopymergegray, (void *)&fh_imagecopymergegray },
  { "imagecopyresampled", fg_imagecopyresampled, (void *)&fh_imagecopyresampled },
  { "imagecopyresized", fg_imagecopyresized, (void *)&fh_imagecopyresized },
  { "imagecreate", fg_imagecreate, (void *)&fh_imagecreate },
  { "imagecreatefromgd2part", fg_imagecreatefromgd2part, (void *)&fh_imagecreatefromgd2part },
  { "imagecreatefromgd", fg_imagecreatefromgd, (void *)&fh_imagecreatefromgd },
  { "imagecreatefromgd2", fg_imagecreatefromgd2, (void *)&fh_imagecreatefromgd2 },
  { "imagecreatefromgif", fg_imagecreatefromgif, (void *)&fh_imagecreatefromgif },
  { "imagecreatefromjpeg", fg_imagecreatefromjpeg, (void *)&fh_imagecreatefromjpeg },
  { "imagecreatefrompng", fg_imagecreatefrompng, (void *)&fh_imagecreatefrompng },
  { "imagecreatefromstring", fg_imagecreatefromstring, (void *)&fh_imagecreatefromstring },
  { "imagecreatefromwbmp", fg_imagecreatefromwbmp, (void *)&fh_imagecreatefromwbmp },
  { "imagecreatefromxbm", fg_imagecreatefromxbm, (void *)&fh_imagecreatefromxbm },
  { "imagecreatefromxpm", fg_imagecreatefromxpm, (void *)&fh_imagecreatefromxpm },
  { "imagecreatetruecolor", fg_imagecreatetruecolor, (void *)&fh_imagecreatetruecolor },
  { "imagedashedline", fg_imagedashedline, (void *)&fh_imagedashedline },
  { "imagedestroy", fg_imagedestroy, (void *)&fh_imagedestroy },
  { "imageellipse", fg_imageellipse, (void *)&fh_imageellipse },
  { "imagefill", fg_imagefill, (void *)&fh_imagefill },
  { "imagefilledarc", fg_imagefilledarc, (void *)&fh_imagefilledarc },
  { "imagefilledellipse", fg_imagefilledellipse, (void *)&fh_imagefilledellipse },
  { "imagefilledpolygon", fg_imagefilledpolygon, (void *)&fh_imagefilledpolygon },
  { "imagefilledrectangle", fg_imagefilledrectangle, (void *)&fh_imagefilledrectangle },
  { "imagefilltoborder", fg_imagefilltoborder, (void *)&fh_imagefilltoborder },
  { "imagefilter", fg_imagefilter, (void *)&fh_imagefilter },
  { "imagefontheight", fg_imagefontheight, (void *)&fh_imagefontheight },
  { "imagefontwidth", fg_imagefontwidth, (void *)&fh_imagefontwidth },
  { "imageftbbox", fg_imageftbbox, (void *)&fh_imageftbbox },
  { "imagefttext", fg_imagefttext, (void *)&fh_imagefttext },
  { "imagegammacorrect", fg_imagegammacorrect, (void *)&fh_imagegammacorrect },
  { "imagegd2", fg_imagegd2, (void *)&fh_imagegd2 },
  { "imagegd", fg_imagegd, (void *)&fh_imagegd },
  { "imagegif", fg_imagegif, (void *)&fh_imagegif },
  { "imagegrabscreen", fg_imagegrabscreen, (void *)&fh_imagegrabscreen },
  { "imagegrabwindow", fg_imagegrabwindow, (void *)&fh_imagegrabwindow },
  { "imageinterlace", fg_imageinterlace, (void *)&fh_imageinterlace },
  { "imageistruecolor", fg_imageistruecolor, (void *)&fh_imageistruecolor },
  { "imagejpeg", fg_imagejpeg, (void *)&fh_imagejpeg },
  { "imagelayereffect", fg_imagelayereffect, (void *)&fh_imagelayereffect },
  { "imageline", fg_imageline, (void *)&fh_imageline },
  { "imageloadfont", fg_imageloadfont, (void *)&fh_imageloadfont },
  { "imagepalettecopy", fg_imagepalettecopy, (void *)&fh_imagepalettecopy },
  { "imagepng", fg_imagepng, (void *)&fh_imagepng },
  { "imagepolygon", fg_imagepolygon, (void *)&fh_imagepolygon },
  { "imagepsbbox", fg_imagepsbbox, (void *)&fh_imagepsbbox },
  { "imagepsencodefont", fg_imagepsencodefont, (void *)&fh_imagepsencodefont },
  { "imagepsextendfont", fg_imagepsextendfont, (void *)&fh_imagepsextendfont },
  { "imagepsfreefont", fg_imagepsfreefont, (void *)&fh_imagepsfreefont },
  { "imagepsloadfont", fg_imagepsloadfont, (void *)&fh_imagepsloadfont },
  { "imagepsslantfont", fg_imagepsslantfont, (void *)&fh_imagepsslantfont },
  { "imagepstext", fg_imagepstext, (void *)&fh_imagepstext },
  { "imagerectangle", fg_imagerectangle, (void *)&fh_imagerectangle },
  { "imagerotate", fg_imagerotate, (void *)&fh_imagerotate },
  { "imagesavealpha", fg_imagesavealpha, (void *)&fh_imagesavealpha },
  { "imagesetbrush", fg_imagesetbrush, (void *)&fh_imagesetbrush },
  { "imagesetpixel", fg_imagesetpixel, (void *)&fh_imagesetpixel },
  { "imagesetstyle", fg_imagesetstyle, (void *)&fh_imagesetstyle },
  { "imagesetthickness", fg_imagesetthickness, (void *)&fh_imagesetthickness },
  { "imagesettile", fg_imagesettile, (void *)&fh_imagesettile },
  { "imagestring", fg_imagestring, (void *)&fh_imagestring },
  { "imagestringup", fg_imagestringup, (void *)&fh_imagestringup },
  { "imagesx", fg_imagesx, (void *)&fh_imagesx },
  { "imagesy", fg_imagesy, (void *)&fh_imagesy },
  { "imagetruecolortopalette", fg_imagetruecolortopalette, (void *)&fh_imagetruecolortopalette },
  { "imagettfbbox", fg_imagettfbbox, (void *)&fh_imagettfbbox },
  { "imagettftext", fg_imagettftext, (void *)&fh_imagettftext },
  { "imagetypes", fg_imagetypes, (void *)&fh_imagetypes },
  { "imagewbmp", fg_imagewbmp, (void *)&fh_imagewbmp },
  { "imagexbm", fg_imagexbm, (void *)&fh_imagexbm },
  { "iptcembed", fg_iptcembed, (void *)&fh_iptcembed },
  { "iptcparse", fg_iptcparse, (void *)&fh_iptcparse },
  { "jpeg2wbmp", fg_jpeg2wbmp, (void *)&fh_jpeg2wbmp },
  { "png2wbmp", fg_png2wbmp, (void *)&fh_png2wbmp },
  { "exif_imagetype", fg_exif_imagetype, (void *)&fh_exif_imagetype },
  { "exif_read_data", fg_exif_read_data, (void *)&fh_exif_read_data },
  { "read_exif_data", fg_read_exif_data, (void *)&fh_read_exif_data },
  { "exif_tagname", fg_exif_tagname, (void *)&fh_exif_tagname },
  { "exif_thumbnail", fg_exif_thumbnail, (void *)&fh_exif_thumbnail },
  { "imap_8bit", fg_imap_8bit, (void *)&fh_imap_8bit },
  { "imap_alerts", fg_imap_alerts, (void *)&fh_imap_alerts },
  { "imap_append", fg_imap_append, (void *)&fh_imap_append },
  { "imap_base64", fg_imap_base64, (void *)&fh_imap_base64 },
  { "imap_binary", fg_imap_binary, (void *)&fh_imap_binary },
  { "imap_body", fg_imap_body, (void *)&fh_imap_body },
  { "imap_bodystruct", fg_imap_bodystruct, (void *)&fh_imap_bodystruct },
  { "imap_check", fg_imap_check, (void *)&fh_imap_check },
  { "imap_clearflag_full", fg_imap_clearflag_full, (void *)&fh_imap_clearflag_full },
  { "imap_close", fg_imap_close, (void *)&fh_imap_close },
  { "imap_createmailbox", fg_imap_createmailbox, (void *)&fh_imap_createmailbox },
  { "imap_delete", fg_imap_delete, (void *)&fh_imap_delete },
  { "imap_deletemailbox", fg_imap_deletemailbox, (void *)&fh_imap_deletemailbox },
  { "imap_errors", fg_imap_errors, (void *)&fh_imap_errors },
  { "imap_expunge", fg_imap_expunge, (void *)&fh_imap_expunge },
  { "imap_fetch_overview", fg_imap_fetch_overview, (void *)&fh_imap_fetch_overview },
  { "imap_fetchbody", fg_imap_fetchbody, (void *)&fh_imap_fetchbody },
  { "imap_fetchheader", fg_imap_fetchheader, (void *)&fh_imap_fetchheader },
  { "imap_fetchstructure", fg_imap_fetchstructure, (void *)&fh_imap_fetchstructure },
  { "imap_gc", fg_imap_gc, (void *)&fh_imap_gc },
  { "imap_get_quota", fg_imap_get_quota, (void *)&fh_imap_get_quota },
  { "imap_get_quotaroot", fg_imap_get_quotaroot, (void *)&fh_imap_get_quotaroot },
  { "imap_getacl", fg_imap_getacl, (void *)&fh_imap_getacl },
  { "imap_getmailboxes", fg_imap_getmailboxes, (void *)&fh_imap_getmailboxes },
  { "imap_getsubscribed", fg_imap_getsubscribed, (void *)&fh_imap_getsubscribed },
  { "imap_header", fg_imap_header, (void *)&fh_imap_header },
  { "imap_headerinfo", fg_imap_headerinfo, (void *)&fh_imap_headerinfo },
  { "imap_headers", fg_imap_headers, (void *)&fh_imap_headers },
  { "imap_last_error", fg_imap_last_error, (void *)&fh_imap_last_error },
  { "imap_list", fg_imap_list, (void *)&fh_imap_list },
  { "imap_listmailbox", fg_imap_listmailbox, (void *)&fh_imap_listmailbox },
  { "imap_listscan", fg_imap_listscan, (void *)&fh_imap_listscan },
  { "imap_listsubscribed", fg_imap_listsubscribed, (void *)&fh_imap_listsubscribed },
  { "imap_lsub", fg_imap_lsub, (void *)&fh_imap_lsub },
  { "imap_mail_compose", fg_imap_mail_compose, (void *)&fh_imap_mail_compose },
  { "imap_mail_copy", fg_imap_mail_copy, (void *)&fh_imap_mail_copy },
  { "imap_mail_move", fg_imap_mail_move, (void *)&fh_imap_mail_move },
  { "imap_mail", fg_imap_mail, (void *)&fh_imap_mail },
  { "imap_mailboxmsginfo", fg_imap_mailboxmsginfo, (void *)&fh_imap_mailboxmsginfo },
  { "imap_mime_header_decode", fg_imap_mime_header_decode, (void *)&fh_imap_mime_header_decode },
  { "imap_msgno", fg_imap_msgno, (void *)&fh_imap_msgno },
  { "imap_num_msg", fg_imap_num_msg, (void *)&fh_imap_num_msg },
  { "imap_num_recent", fg_imap_num_recent, (void *)&fh_imap_num_recent },
  { "imap_open", fg_imap_open, (void *)&fh_imap_open },
  { "imap_ping", fg_imap_ping, (void *)&fh_imap_ping },
  { "imap_qprint", fg_imap_qprint, (void *)&fh_imap_qprint },
  { "imap_renamemailbox", fg_imap_renamemailbox, (void *)&fh_imap_renamemailbox },
  { "imap_reopen", fg_imap_reopen, (void *)&fh_imap_reopen },
  { "imap_rfc822_parse_adrlist", fg_imap_rfc822_parse_adrlist, (void *)&fh_imap_rfc822_parse_adrlist },
  { "imap_rfc822_parse_headers", fg_imap_rfc822_parse_headers, (void *)&fh_imap_rfc822_parse_headers },
  { "imap_rfc822_write_address", fg_imap_rfc822_write_address, (void *)&fh_imap_rfc822_write_address },
  { "imap_savebody", fg_imap_savebody, (void *)&fh_imap_savebody },
  { "imap_scanmailbox", fg_imap_scanmailbox, (void *)&fh_imap_scanmailbox },
  { "imap_search", fg_imap_search, (void *)&fh_imap_search },
  { "imap_set_quota", fg_imap_set_quota, (void *)&fh_imap_set_quota },
  { "imap_setacl", fg_imap_setacl, (void *)&fh_imap_setacl },
  { "imap_setflag_full", fg_imap_setflag_full, (void *)&fh_imap_setflag_full },
  { "imap_sort", fg_imap_sort, (void *)&fh_imap_sort },
  { "imap_status", fg_imap_status, (void *)&fh_imap_status },
  { "imap_subscribe", fg_imap_subscribe, (void *)&fh_imap_subscribe },
  { "imap_thread", fg_imap_thread, (void *)&fh_imap_thread },
  { "imap_timeout", fg_imap_timeout, (void *)&fh_imap_timeout },
  { "imap_uid", fg_imap_uid, (void *)&fh_imap_uid },
  { "imap_undelete", fg_imap_undelete, (void *)&fh_imap_undelete },
  { "imap_unsubscribe", fg_imap_unsubscribe, (void *)&fh_imap_unsubscribe },
  { "imap_utf7_decode", fg_imap_utf7_decode, (void *)&fh_imap_utf7_decode },
  { "imap_utf7_encode", fg_imap_utf7_encode, (void *)&fh_imap_utf7_encode },
  { "imap_utf8", fg_imap_utf8, (void *)&fh_imap_utf8 },
  { "intl_get_error_code", fg_intl_get_error_code, (void *)&fh_intl_get_error_code },
  { "intl_get_error_message", fg_intl_get_error_message, (void *)&fh_intl_get_error_message },
  { "intl_error_name", fg_intl_error_name, (void *)&fh_intl_error_name },
  { "intl_is_failure", fg_intl_is_failure, (void *)&fh_intl_is_failure },
  { "collator_asort", fg_collator_asort, (void *)&fh_collator_asort },
  { "collator_compare", fg_collator_compare, (void *)&fh_collator_compare },
  { "collator_create", fg_collator_create, (void *)&fh_collator_create },
  { "collator_get_attribute", fg_collator_get_attribute, (void *)&fh_collator_get_attribute },
  { "collator_get_error_code", fg_collator_get_error_code, (void *)&fh_collator_get_error_code },
  { "collator_get_error_message", fg_collator_get_error_message, (void *)&fh_collator_get_error_message },
  { "collator_get_locale", fg_collator_get_locale, (void *)&fh_collator_get_locale },
  { "collator_get_strength", fg_collator_get_strength, (void *)&fh_collator_get_strength },
  { "collator_set_attribute", fg_collator_set_attribute, (void *)&fh_collator_set_attribute },
  { "collator_set_strength", fg_collator_set_strength, (void *)&fh_collator_set_strength },
  { "collator_sort_with_sort_keys", fg_collator_sort_with_sort_keys, (void *)&fh_collator_sort_with_sort_keys },
  { "collator_sort", fg_collator_sort, (void *)&fh_collator_sort },
  { "idn_to_ascii", fg_idn_to_ascii, (void *)&fh_idn_to_ascii },
  { "idn_to_unicode", fg_idn_to_unicode, (void *)&fh_idn_to_unicode },
  { "idn_to_utf8", fg_idn_to_utf8, (void *)&fh_idn_to_utf8 },
  { "ftok", fg_ftok, (void *)&fh_ftok },
  { "msg_get_queue", fg_msg_get_queue, (void *)&fh_msg_get_queue },
  { "msg_queue_exists", fg_msg_queue_exists, (void *)&fh_msg_queue_exists },
  { "msg_send", fg_msg_send, (void *)&fh_msg_send },
  { "msg_receive", fg_msg_receive, (void *)&fh_msg_receive },
  { "msg_remove_queue", fg_msg_remove_queue, (void *)&fh_msg_remove_queue },
  { "msg_set_queue", fg_msg_set_queue, (void *)&fh_msg_set_queue },
  { "msg_stat_queue", fg_msg_stat_queue, (void *)&fh_msg_stat_queue },
  { "sem_acquire", fg_sem_acquire, (void *)&fh_sem_acquire },
  { "sem_get", fg_sem_get, (void *)&fh_sem_get },
  { "sem_release", fg_sem_release, (void *)&fh_sem_release },
  { "sem_remove", fg_sem_remove, (void *)&fh_sem_remove },
  { "shm_attach", fg_shm_attach, (void *)&fh_shm_attach },
  { "shm_detach", fg_shm_detach, (void *)&fh_shm_detach },
  { "shm_remove", fg_shm_remove, (void *)&fh_shm_remove },
  { "shm_get_var", fg_shm_get_var, (void *)&fh_shm_get_var },
  { "shm_has_var", fg_shm_has_var, (void *)&fh_shm_has_var },
  { "shm_put_var", fg_shm_put_var, (void *)&fh_shm_put_var },
  { "shm_remove_var", fg_shm_remove_var, (void *)&fh_shm_remove_var },
  { "hphp_recursiveiteratoriterator___construct", fg_hphp_recursiveiteratoriterator___construct, (void *)&fh_hphp_recursiveiteratoriterator___construct },
  { "hphp_recursiveiteratoriterator_getinneriterator", fg_hphp_recursiveiteratoriterator_getinneriterator, (void *)&fh_hphp_recursiveiteratoriterator_getinneriterator },
  { "hphp_recursiveiteratoriterator_current", fg_hphp_recursiveiteratoriterator_current, (void *)&fh_hphp_recursiveiteratoriterator_current },
  { "hphp_recursiveiteratoriterator_key", fg_hphp_recursiveiteratoriterator_key, (void *)&fh_hphp_recursiveiteratoriterator_key },
  { "hphp_recursiveiteratoriterator_next", fg_hphp_recursiveiteratoriterator_next, (void *)&fh_hphp_recursiveiteratoriterator_next },
  { "hphp_recursiveiteratoriterator_rewind", fg_hphp_recursiveiteratoriterator_rewind, (void *)&fh_hphp_recursiveiteratoriterator_rewind },
  { "hphp_recursiveiteratoriterator_valid", fg_hphp_recursiveiteratoriterator_valid, (void *)&fh_hphp_recursiveiteratoriterator_valid },
  { "hphp_directoryiterator___construct", fg_hphp_directoryiterator___construct, (void *)&fh_hphp_directoryiterator___construct },
  { "hphp_directoryiterator_key", fg_hphp_directoryiterator_key, (void *)&fh_hphp_directoryiterator_key },
  { "hphp_directoryiterator_next", fg_hphp_directoryiterator_next, (void *)&fh_hphp_directoryiterator_next },
  { "hphp_directoryiterator_rewind", fg_hphp_directoryiterator_rewind, (void *)&fh_hphp_directoryiterator_rewind },
  { "hphp_directoryiterator_seek", fg_hphp_directoryiterator_seek, (void *)&fh_hphp_directoryiterator_seek },
  { "hphp_directoryiterator_current", fg_hphp_directoryiterator_current, (void *)&fh_hphp_directoryiterator_current },
  { "hphp_directoryiterator___tostring", fg_hphp_directoryiterator___tostring, (void *)&fh_hphp_directoryiterator___tostring },
  { "hphp_directoryiterator_valid", fg_hphp_directoryiterator_valid, (void *)&fh_hphp_directoryiterator_valid },
  { "hphp_directoryiterator_isdot", fg_hphp_directoryiterator_isdot, (void *)&fh_hphp_directoryiterator_isdot },
  { "hphp_recursivedirectoryiterator___construct", fg_hphp_recursivedirectoryiterator___construct, (void *)&fh_hphp_recursivedirectoryiterator___construct },
  { "hphp_recursivedirectoryiterator_key", fg_hphp_recursivedirectoryiterator_key, (void *)&fh_hphp_recursivedirectoryiterator_key },
  { "hphp_recursivedirectoryiterator_next", fg_hphp_recursivedirectoryiterator_next, (void *)&fh_hphp_recursivedirectoryiterator_next },
  { "hphp_recursivedirectoryiterator_rewind", fg_hphp_recursivedirectoryiterator_rewind, (void *)&fh_hphp_recursivedirectoryiterator_rewind },
  { "hphp_recursivedirectoryiterator_seek", fg_hphp_recursivedirectoryiterator_seek, (void *)&fh_hphp_recursivedirectoryiterator_seek },
  { "hphp_recursivedirectoryiterator_current", fg_hphp_recursivedirectoryiterator_current, (void *)&fh_hphp_recursivedirectoryiterator_current },
  { "hphp_recursivedirectoryiterator___tostring", fg_hphp_recursivedirectoryiterator___tostring, (void *)&fh_hphp_recursivedirectoryiterator___tostring },
  { "hphp_recursivedirectoryiterator_valid", fg_hphp_recursivedirectoryiterator_valid, (void *)&fh_hphp_recursivedirectoryiterator_valid },
  { "hphp_recursivedirectoryiterator_haschildren", fg_hphp_recursivedirectoryiterator_haschildren, (void *)&fh_hphp_recursivedirectoryiterator_haschildren },
  { "hphp_recursivedirectoryiterator_getchildren", fg_hphp_recursivedirectoryiterator_getchildren, (void *)&fh_hphp_recursivedirectoryiterator_getchildren },
  { "hphp_recursivedirectoryiterator_getsubpath", fg_hphp_recursivedirectoryiterator_getsubpath, (void *)&fh_hphp_recursivedirectoryiterator_getsubpath },
  { "hphp_recursivedirectoryiterator_getsubpathname", fg_hphp_recursivedirectoryiterator_getsubpathname, (void *)&fh_hphp_recursivedirectoryiterator_getsubpathname },
  { "json_encode", fg_json_encode, (void *)&fh_json_encode },
  { "json_decode", fg_json_decode, (void *)&fh_json_decode },
  { "ldap_connect", fg_ldap_connect, (void *)&fh_ldap_connect },
  { "ldap_explode_dn", fg_ldap_explode_dn, (void *)&fh_ldap_explode_dn },
  { "ldap_dn2ufn", fg_ldap_dn2ufn, (void *)&fh_ldap_dn2ufn },
  { "ldap_err2str", fg_ldap_err2str, (void *)&fh_ldap_err2str },
  { "ldap_add", fg_ldap_add, (void *)&fh_ldap_add },
  { "ldap_mod_add", fg_ldap_mod_add, (void *)&fh_ldap_mod_add },
  { "ldap_mod_del", fg_ldap_mod_del, (void *)&fh_ldap_mod_del },
  { "ldap_mod_replace", fg_ldap_mod_replace, (void *)&fh_ldap_mod_replace },
  { "ldap_modify", fg_ldap_modify, (void *)&fh_ldap_modify },
  { "ldap_bind", fg_ldap_bind, (void *)&fh_ldap_bind },
  { "ldap_set_rebind_proc", fg_ldap_set_rebind_proc, (void *)&fh_ldap_set_rebind_proc },
  { "ldap_sort", fg_ldap_sort, (void *)&fh_ldap_sort },
  { "ldap_start_tls", fg_ldap_start_tls, (void *)&fh_ldap_start_tls },
  { "ldap_unbind", fg_ldap_unbind, (void *)&fh_ldap_unbind },
  { "ldap_get_option", fg_ldap_get_option, (void *)&fh_ldap_get_option },
  { "ldap_set_option", fg_ldap_set_option, (void *)&fh_ldap_set_option },
  { "ldap_close", fg_ldap_close, (void *)&fh_ldap_close },
  { "ldap_list", fg_ldap_list, (void *)&fh_ldap_list },
  { "ldap_read", fg_ldap_read, (void *)&fh_ldap_read },
  { "ldap_search", fg_ldap_search, (void *)&fh_ldap_search },
  { "ldap_rename", fg_ldap_rename, (void *)&fh_ldap_rename },
  { "ldap_delete", fg_ldap_delete, (void *)&fh_ldap_delete },
  { "ldap_compare", fg_ldap_compare, (void *)&fh_ldap_compare },
  { "ldap_errno", fg_ldap_errno, (void *)&fh_ldap_errno },
  { "ldap_error", fg_ldap_error, (void *)&fh_ldap_error },
  { "ldap_get_dn", fg_ldap_get_dn, (void *)&fh_ldap_get_dn },
  { "ldap_count_entries", fg_ldap_count_entries, (void *)&fh_ldap_count_entries },
  { "ldap_get_entries", fg_ldap_get_entries, (void *)&fh_ldap_get_entries },
  { "ldap_first_entry", fg_ldap_first_entry, (void *)&fh_ldap_first_entry },
  { "ldap_next_entry", fg_ldap_next_entry, (void *)&fh_ldap_next_entry },
  { "ldap_get_attributes", fg_ldap_get_attributes, (void *)&fh_ldap_get_attributes },
  { "ldap_first_attribute", fg_ldap_first_attribute, (void *)&fh_ldap_first_attribute },
  { "ldap_next_attribute", fg_ldap_next_attribute, (void *)&fh_ldap_next_attribute },
  { "ldap_first_reference", fg_ldap_first_reference, (void *)&fh_ldap_first_reference },
  { "ldap_next_reference", fg_ldap_next_reference, (void *)&fh_ldap_next_reference },
  { "ldap_parse_reference", fg_ldap_parse_reference, (void *)&fh_ldap_parse_reference },
  { "ldap_parse_result", fg_ldap_parse_result, (void *)&fh_ldap_parse_result },
  { "ldap_free_result", fg_ldap_free_result, (void *)&fh_ldap_free_result },
  { "ldap_get_values_len", fg_ldap_get_values_len, (void *)&fh_ldap_get_values_len },
  { "ldap_get_values", fg_ldap_get_values, (void *)&fh_ldap_get_values },
  { "magickgetcopyright", fg_magickgetcopyright, (void *)&fh_magickgetcopyright },
  { "magickgethomeurl", fg_magickgethomeurl, (void *)&fh_magickgethomeurl },
  { "magickgetpackagename", fg_magickgetpackagename, (void *)&fh_magickgetpackagename },
  { "magickgetquantumdepth", fg_magickgetquantumdepth, (void *)&fh_magickgetquantumdepth },
  { "magickgetreleasedate", fg_magickgetreleasedate, (void *)&fh_magickgetreleasedate },
  { "magickgetresourcelimit", fg_magickgetresourcelimit, (void *)&fh_magickgetresourcelimit },
  { "magickgetversion", fg_magickgetversion, (void *)&fh_magickgetversion },
  { "magickgetversionnumber", fg_magickgetversionnumber, (void *)&fh_magickgetversionnumber },
  { "magickgetversionstring", fg_magickgetversionstring, (void *)&fh_magickgetversionstring },
  { "magickqueryconfigureoption", fg_magickqueryconfigureoption, (void *)&fh_magickqueryconfigureoption },
  { "magickqueryconfigureoptions", fg_magickqueryconfigureoptions, (void *)&fh_magickqueryconfigureoptions },
  { "magickqueryfonts", fg_magickqueryfonts, (void *)&fh_magickqueryfonts },
  { "magickqueryformats", fg_magickqueryformats, (void *)&fh_magickqueryformats },
  { "magicksetresourcelimit", fg_magicksetresourcelimit, (void *)&fh_magicksetresourcelimit },
  { "newdrawingwand", fg_newdrawingwand, (void *)&fh_newdrawingwand },
  { "newmagickwand", fg_newmagickwand, (void *)&fh_newmagickwand },
  { "newpixeliterator", fg_newpixeliterator, (void *)&fh_newpixeliterator },
  { "newpixelregioniterator", fg_newpixelregioniterator, (void *)&fh_newpixelregioniterator },
  { "newpixelwand", fg_newpixelwand, (void *)&fh_newpixelwand },
  { "newpixelwandarray", fg_newpixelwandarray, (void *)&fh_newpixelwandarray },
  { "newpixelwands", fg_newpixelwands, (void *)&fh_newpixelwands },
  { "destroydrawingwand", fg_destroydrawingwand, (void *)&fh_destroydrawingwand },
  { "destroymagickwand", fg_destroymagickwand, (void *)&fh_destroymagickwand },
  { "destroypixeliterator", fg_destroypixeliterator, (void *)&fh_destroypixeliterator },
  { "destroypixelwand", fg_destroypixelwand, (void *)&fh_destroypixelwand },
  { "destroypixelwandarray", fg_destroypixelwandarray, (void *)&fh_destroypixelwandarray },
  { "destroypixelwands", fg_destroypixelwands, (void *)&fh_destroypixelwands },
  { "isdrawingwand", fg_isdrawingwand, (void *)&fh_isdrawingwand },
  { "ismagickwand", fg_ismagickwand, (void *)&fh_ismagickwand },
  { "ispixeliterator", fg_ispixeliterator, (void *)&fh_ispixeliterator },
  { "ispixelwand", fg_ispixelwand, (void *)&fh_ispixelwand },
  { "cleardrawingwand", fg_cleardrawingwand, (void *)&fh_cleardrawingwand },
  { "clearmagickwand", fg_clearmagickwand, (void *)&fh_clearmagickwand },
  { "clearpixeliterator", fg_clearpixeliterator, (void *)&fh_clearpixeliterator },
  { "clearpixelwand", fg_clearpixelwand, (void *)&fh_clearpixelwand },
  { "clonedrawingwand", fg_clonedrawingwand, (void *)&fh_clonedrawingwand },
  { "clonemagickwand", fg_clonemagickwand, (void *)&fh_clonemagickwand },
  { "wandgetexception", fg_wandgetexception, (void *)&fh_wandgetexception },
  { "wandgetexceptionstring", fg_wandgetexceptionstring, (void *)&fh_wandgetexceptionstring },
  { "wandgetexceptiontype", fg_wandgetexceptiontype, (void *)&fh_wandgetexceptiontype },
  { "wandhasexception", fg_wandhasexception, (void *)&fh_wandhasexception },
  { "drawaffine", fg_drawaffine, (void *)&fh_drawaffine },
  { "drawannotation", fg_drawannotation, (void *)&fh_drawannotation },
  { "drawarc", fg_drawarc, (void *)&fh_drawarc },
  { "drawbezier", fg_drawbezier, (void *)&fh_drawbezier },
  { "drawcircle", fg_drawcircle, (void *)&fh_drawcircle },
  { "drawcolor", fg_drawcolor, (void *)&fh_drawcolor },
  { "drawcomment", fg_drawcomment, (void *)&fh_drawcomment },
  { "drawcomposite", fg_drawcomposite, (void *)&fh_drawcomposite },
  { "drawellipse", fg_drawellipse, (void *)&fh_drawellipse },
  { "drawgetclippath", fg_drawgetclippath, (void *)&fh_drawgetclippath },
  { "drawgetcliprule", fg_drawgetcliprule, (void *)&fh_drawgetcliprule },
  { "drawgetclipunits", fg_drawgetclipunits, (void *)&fh_drawgetclipunits },
  { "drawgetexception", fg_drawgetexception, (void *)&fh_drawgetexception },
  { "drawgetexceptionstring", fg_drawgetexceptionstring, (void *)&fh_drawgetexceptionstring },
  { "drawgetexceptiontype", fg_drawgetexceptiontype, (void *)&fh_drawgetexceptiontype },
  { "drawgetfillalpha", fg_drawgetfillalpha, (void *)&fh_drawgetfillalpha },
  { "drawgetfillcolor", fg_drawgetfillcolor, (void *)&fh_drawgetfillcolor },
  { "drawgetfillopacity", fg_drawgetfillopacity, (void *)&fh_drawgetfillopacity },
  { "drawgetfillrule", fg_drawgetfillrule, (void *)&fh_drawgetfillrule },
  { "drawgetfont", fg_drawgetfont, (void *)&fh_drawgetfont },
  { "drawgetfontfamily", fg_drawgetfontfamily, (void *)&fh_drawgetfontfamily },
  { "drawgetfontsize", fg_drawgetfontsize, (void *)&fh_drawgetfontsize },
  { "drawgetfontstretch", fg_drawgetfontstretch, (void *)&fh_drawgetfontstretch },
  { "drawgetfontstyle", fg_drawgetfontstyle, (void *)&fh_drawgetfontstyle },
  { "drawgetfontweight", fg_drawgetfontweight, (void *)&fh_drawgetfontweight },
  { "drawgetgravity", fg_drawgetgravity, (void *)&fh_drawgetgravity },
  { "drawgetstrokealpha", fg_drawgetstrokealpha, (void *)&fh_drawgetstrokealpha },
  { "drawgetstrokeantialias", fg_drawgetstrokeantialias, (void *)&fh_drawgetstrokeantialias },
  { "drawgetstrokecolor", fg_drawgetstrokecolor, (void *)&fh_drawgetstrokecolor },
  { "drawgetstrokedasharray", fg_drawgetstrokedasharray, (void *)&fh_drawgetstrokedasharray },
  { "drawgetstrokedashoffset", fg_drawgetstrokedashoffset, (void *)&fh_drawgetstrokedashoffset },
  { "drawgetstrokelinecap", fg_drawgetstrokelinecap, (void *)&fh_drawgetstrokelinecap },
  { "drawgetstrokelinejoin", fg_drawgetstrokelinejoin, (void *)&fh_drawgetstrokelinejoin },
  { "drawgetstrokemiterlimit", fg_drawgetstrokemiterlimit, (void *)&fh_drawgetstrokemiterlimit },
  { "drawgetstrokeopacity", fg_drawgetstrokeopacity, (void *)&fh_drawgetstrokeopacity },
  { "drawgetstrokewidth", fg_drawgetstrokewidth, (void *)&fh_drawgetstrokewidth },
  { "drawgettextalignment", fg_drawgettextalignment, (void *)&fh_drawgettextalignment },
  { "drawgettextantialias", fg_drawgettextantialias, (void *)&fh_drawgettextantialias },
  { "drawgettextdecoration", fg_drawgettextdecoration, (void *)&fh_drawgettextdecoration },
  { "drawgettextencoding", fg_drawgettextencoding, (void *)&fh_drawgettextencoding },
  { "drawgettextundercolor", fg_drawgettextundercolor, (void *)&fh_drawgettextundercolor },
  { "drawgetvectorgraphics", fg_drawgetvectorgraphics, (void *)&fh_drawgetvectorgraphics },
  { "drawline", fg_drawline, (void *)&fh_drawline },
  { "drawmatte", fg_drawmatte, (void *)&fh_drawmatte },
  { "drawpathclose", fg_drawpathclose, (void *)&fh_drawpathclose },
  { "drawpathcurvetoabsolute", fg_drawpathcurvetoabsolute, (void *)&fh_drawpathcurvetoabsolute },
  { "drawpathcurvetoquadraticbezierabsolute", fg_drawpathcurvetoquadraticbezierabsolute, (void *)&fh_drawpathcurvetoquadraticbezierabsolute },
  { "drawpathcurvetoquadraticbezierrelative", fg_drawpathcurvetoquadraticbezierrelative, (void *)&fh_drawpathcurvetoquadraticbezierrelative },
  { "drawpathcurvetoquadraticbeziersmoothabsolute", fg_drawpathcurvetoquadraticbeziersmoothabsolute, (void *)&fh_drawpathcurvetoquadraticbeziersmoothabsolute },
  { "drawpathcurvetoquadraticbeziersmoothrelative", fg_drawpathcurvetoquadraticbeziersmoothrelative, (void *)&fh_drawpathcurvetoquadraticbeziersmoothrelative },
  { "drawpathcurvetorelative", fg_drawpathcurvetorelative, (void *)&fh_drawpathcurvetorelative },
  { "drawpathcurvetosmoothabsolute", fg_drawpathcurvetosmoothabsolute, (void *)&fh_drawpathcurvetosmoothabsolute },
  { "drawpathcurvetosmoothrelative", fg_drawpathcurvetosmoothrelative, (void *)&fh_drawpathcurvetosmoothrelative },
  { "drawpathellipticarcabsolute", fg_drawpathellipticarcabsolute, (void *)&fh_drawpathellipticarcabsolute },
  { "drawpathellipticarcrelative", fg_drawpathellipticarcrelative, (void *)&fh_drawpathellipticarcrelative },
  { "drawpathfinish", fg_drawpathfinish, (void *)&fh_drawpathfinish },
  { "drawpathlinetoabsolute", fg_drawpathlinetoabsolute, (void *)&fh_drawpathlinetoabsolute },
  { "drawpathlinetohorizontalabsolute", fg_drawpathlinetohorizontalabsolute, (void *)&fh_drawpathlinetohorizontalabsolute },
  { "drawpathlinetohorizontalrelative", fg_drawpathlinetohorizontalrelative, (void *)&fh_drawpathlinetohorizontalrelative },
  { "drawpathlinetorelative", fg_drawpathlinetorelative, (void *)&fh_drawpathlinetorelative },
  { "drawpathlinetoverticalabsolute", fg_drawpathlinetoverticalabsolute, (void *)&fh_drawpathlinetoverticalabsolute },
  { "drawpathlinetoverticalrelative", fg_drawpathlinetoverticalrelative, (void *)&fh_drawpathlinetoverticalrelative },
  { "drawpathmovetoabsolute", fg_drawpathmovetoabsolute, (void *)&fh_drawpathmovetoabsolute },
  { "drawpathmovetorelative", fg_drawpathmovetorelative, (void *)&fh_drawpathmovetorelative },
  { "drawpathstart", fg_drawpathstart, (void *)&fh_drawpathstart },
  { "drawpoint", fg_drawpoint, (void *)&fh_drawpoint },
  { "drawpolygon", fg_drawpolygon, (void *)&fh_drawpolygon },
  { "drawpolyline", fg_drawpolyline, (void *)&fh_drawpolyline },
  { "drawrectangle", fg_drawrectangle, (void *)&fh_drawrectangle },
  { "drawrender", fg_drawrender, (void *)&fh_drawrender },
  { "drawrotate", fg_drawrotate, (void *)&fh_drawrotate },
  { "drawroundrectangle", fg_drawroundrectangle, (void *)&fh_drawroundrectangle },
  { "drawscale", fg_drawscale, (void *)&fh_drawscale },
  { "drawsetclippath", fg_drawsetclippath, (void *)&fh_drawsetclippath },
  { "drawsetcliprule", fg_drawsetcliprule, (void *)&fh_drawsetcliprule },
  { "drawsetclipunits", fg_drawsetclipunits, (void *)&fh_drawsetclipunits },
  { "drawsetfillalpha", fg_drawsetfillalpha, (void *)&fh_drawsetfillalpha },
  { "drawsetfillcolor", fg_drawsetfillcolor, (void *)&fh_drawsetfillcolor },
  { "drawsetfillopacity", fg_drawsetfillopacity, (void *)&fh_drawsetfillopacity },
  { "drawsetfillpatternurl", fg_drawsetfillpatternurl, (void *)&fh_drawsetfillpatternurl },
  { "drawsetfillrule", fg_drawsetfillrule, (void *)&fh_drawsetfillrule },
  { "drawsetfont", fg_drawsetfont, (void *)&fh_drawsetfont },
  { "drawsetfontfamily", fg_drawsetfontfamily, (void *)&fh_drawsetfontfamily },
  { "drawsetfontsize", fg_drawsetfontsize, (void *)&fh_drawsetfontsize },
  { "drawsetfontstretch", fg_drawsetfontstretch, (void *)&fh_drawsetfontstretch },
  { "drawsetfontstyle", fg_drawsetfontstyle, (void *)&fh_drawsetfontstyle },
  { "drawsetfontweight", fg_drawsetfontweight, (void *)&fh_drawsetfontweight },
  { "drawsetgravity", fg_drawsetgravity, (void *)&fh_drawsetgravity },
  { "drawsetstrokealpha", fg_drawsetstrokealpha, (void *)&fh_drawsetstrokealpha },
  { "drawsetstrokeantialias", fg_drawsetstrokeantialias, (void *)&fh_drawsetstrokeantialias },
  { "drawsetstrokecolor", fg_drawsetstrokecolor, (void *)&fh_drawsetstrokecolor },
  { "drawsetstrokedasharray", fg_drawsetstrokedasharray, (void *)&fh_drawsetstrokedasharray },
  { "drawsetstrokedashoffset", fg_drawsetstrokedashoffset, (void *)&fh_drawsetstrokedashoffset },
  { "drawsetstrokelinecap", fg_drawsetstrokelinecap, (void *)&fh_drawsetstrokelinecap },
  { "drawsetstrokelinejoin", fg_drawsetstrokelinejoin, (void *)&fh_drawsetstrokelinejoin },
  { "drawsetstrokemiterlimit", fg_drawsetstrokemiterlimit, (void *)&fh_drawsetstrokemiterlimit },
  { "drawsetstrokeopacity", fg_drawsetstrokeopacity, (void *)&fh_drawsetstrokeopacity },
  { "drawsetstrokepatternurl", fg_drawsetstrokepatternurl, (void *)&fh_drawsetstrokepatternurl },
  { "drawsetstrokewidth", fg_drawsetstrokewidth, (void *)&fh_drawsetstrokewidth },
  { "drawsettextalignment", fg_drawsettextalignment, (void *)&fh_drawsettextalignment },
  { "drawsettextantialias", fg_drawsettextantialias, (void *)&fh_drawsettextantialias },
  { "drawsettextdecoration", fg_drawsettextdecoration, (void *)&fh_drawsettextdecoration },
  { "drawsettextencoding", fg_drawsettextencoding, (void *)&fh_drawsettextencoding },
  { "drawsettextundercolor", fg_drawsettextundercolor, (void *)&fh_drawsettextundercolor },
  { "drawsetvectorgraphics", fg_drawsetvectorgraphics, (void *)&fh_drawsetvectorgraphics },
  { "drawsetviewbox", fg_drawsetviewbox, (void *)&fh_drawsetviewbox },
  { "drawskewx", fg_drawskewx, (void *)&fh_drawskewx },
  { "drawskewy", fg_drawskewy, (void *)&fh_drawskewy },
  { "drawtranslate", fg_drawtranslate, (void *)&fh_drawtranslate },
  { "pushdrawingwand", fg_pushdrawingwand, (void *)&fh_pushdrawingwand },
  { "drawpushclippath", fg_drawpushclippath, (void *)&fh_drawpushclippath },
  { "drawpushdefs", fg_drawpushdefs, (void *)&fh_drawpushdefs },
  { "drawpushpattern", fg_drawpushpattern, (void *)&fh_drawpushpattern },
  { "popdrawingwand", fg_popdrawingwand, (void *)&fh_popdrawingwand },
  { "drawpopclippath", fg_drawpopclippath, (void *)&fh_drawpopclippath },
  { "drawpopdefs", fg_drawpopdefs, (void *)&fh_drawpopdefs },
  { "drawpoppattern", fg_drawpoppattern, (void *)&fh_drawpoppattern },
  { "magickadaptivethresholdimage", fg_magickadaptivethresholdimage, (void *)&fh_magickadaptivethresholdimage },
  { "magickaddimage", fg_magickaddimage, (void *)&fh_magickaddimage },
  { "magickaddnoiseimage", fg_magickaddnoiseimage, (void *)&fh_magickaddnoiseimage },
  { "magickaffinetransformimage", fg_magickaffinetransformimage, (void *)&fh_magickaffinetransformimage },
  { "magickannotateimage", fg_magickannotateimage, (void *)&fh_magickannotateimage },
  { "magickappendimages", fg_magickappendimages, (void *)&fh_magickappendimages },
  { "magickaverageimages", fg_magickaverageimages, (void *)&fh_magickaverageimages },
  { "magickblackthresholdimage", fg_magickblackthresholdimage, (void *)&fh_magickblackthresholdimage },
  { "magickblurimage", fg_magickblurimage, (void *)&fh_magickblurimage },
  { "magickborderimage", fg_magickborderimage, (void *)&fh_magickborderimage },
  { "magickcharcoalimage", fg_magickcharcoalimage, (void *)&fh_magickcharcoalimage },
  { "magickchopimage", fg_magickchopimage, (void *)&fh_magickchopimage },
  { "magickclipimage", fg_magickclipimage, (void *)&fh_magickclipimage },
  { "magickclippathimage", fg_magickclippathimage, (void *)&fh_magickclippathimage },
  { "magickcoalesceimages", fg_magickcoalesceimages, (void *)&fh_magickcoalesceimages },
  { "magickcolorfloodfillimage", fg_magickcolorfloodfillimage, (void *)&fh_magickcolorfloodfillimage },
  { "magickcolorizeimage", fg_magickcolorizeimage, (void *)&fh_magickcolorizeimage },
  { "magickcombineimages", fg_magickcombineimages, (void *)&fh_magickcombineimages },
  { "magickcommentimage", fg_magickcommentimage, (void *)&fh_magickcommentimage },
  { "magickcompareimages", fg_magickcompareimages, (void *)&fh_magickcompareimages },
  { "magickcompositeimage", fg_magickcompositeimage, (void *)&fh_magickcompositeimage },
  { "magickconstituteimage", fg_magickconstituteimage, (void *)&fh_magickconstituteimage },
  { "magickcontrastimage", fg_magickcontrastimage, (void *)&fh_magickcontrastimage },
  { "magickconvolveimage", fg_magickconvolveimage, (void *)&fh_magickconvolveimage },
  { "magickcropimage", fg_magickcropimage, (void *)&fh_magickcropimage },
  { "magickcyclecolormapimage", fg_magickcyclecolormapimage, (void *)&fh_magickcyclecolormapimage },
  { "magickdeconstructimages", fg_magickdeconstructimages, (void *)&fh_magickdeconstructimages },
  { "magickdescribeimage", fg_magickdescribeimage, (void *)&fh_magickdescribeimage },
  { "magickdespeckleimage", fg_magickdespeckleimage, (void *)&fh_magickdespeckleimage },
  { "magickdrawimage", fg_magickdrawimage, (void *)&fh_magickdrawimage },
  { "magickechoimageblob", fg_magickechoimageblob, (void *)&fh_magickechoimageblob },
  { "magickechoimagesblob", fg_magickechoimagesblob, (void *)&fh_magickechoimagesblob },
  { "magickedgeimage", fg_magickedgeimage, (void *)&fh_magickedgeimage },
  { "magickembossimage", fg_magickembossimage, (void *)&fh_magickembossimage },
  { "magickenhanceimage", fg_magickenhanceimage, (void *)&fh_magickenhanceimage },
  { "magickequalizeimage", fg_magickequalizeimage, (void *)&fh_magickequalizeimage },
  { "magickevaluateimage", fg_magickevaluateimage, (void *)&fh_magickevaluateimage },
  { "magickflattenimages", fg_magickflattenimages, (void *)&fh_magickflattenimages },
  { "magickflipimage", fg_magickflipimage, (void *)&fh_magickflipimage },
  { "magickflopimage", fg_magickflopimage, (void *)&fh_magickflopimage },
  { "magickframeimage", fg_magickframeimage, (void *)&fh_magickframeimage },
  { "magickfximage", fg_magickfximage, (void *)&fh_magickfximage },
  { "magickgammaimage", fg_magickgammaimage, (void *)&fh_magickgammaimage },
  { "magickgaussianblurimage", fg_magickgaussianblurimage, (void *)&fh_magickgaussianblurimage },
  { "magickgetcharheight", fg_magickgetcharheight, (void *)&fh_magickgetcharheight },
  { "magickgetcharwidth", fg_magickgetcharwidth, (void *)&fh_magickgetcharwidth },
  { "magickgetexception", fg_magickgetexception, (void *)&fh_magickgetexception },
  { "magickgetexceptionstring", fg_magickgetexceptionstring, (void *)&fh_magickgetexceptionstring },
  { "magickgetexceptiontype", fg_magickgetexceptiontype, (void *)&fh_magickgetexceptiontype },
  { "magickgetfilename", fg_magickgetfilename, (void *)&fh_magickgetfilename },
  { "magickgetformat", fg_magickgetformat, (void *)&fh_magickgetformat },
  { "magickgetimage", fg_magickgetimage, (void *)&fh_magickgetimage },
  { "magickgetimagebackgroundcolor", fg_magickgetimagebackgroundcolor, (void *)&fh_magickgetimagebackgroundcolor },
  { "magickgetimageblob", fg_magickgetimageblob, (void *)&fh_magickgetimageblob },
  { "magickgetimageblueprimary", fg_magickgetimageblueprimary, (void *)&fh_magickgetimageblueprimary },
  { "magickgetimagebordercolor", fg_magickgetimagebordercolor, (void *)&fh_magickgetimagebordercolor },
  { "magickgetimagechannelmean", fg_magickgetimagechannelmean, (void *)&fh_magickgetimagechannelmean },
  { "magickgetimagecolormapcolor", fg_magickgetimagecolormapcolor, (void *)&fh_magickgetimagecolormapcolor },
  { "magickgetimagecolors", fg_magickgetimagecolors, (void *)&fh_magickgetimagecolors },
  { "magickgetimagecolorspace", fg_magickgetimagecolorspace, (void *)&fh_magickgetimagecolorspace },
  { "magickgetimagecompose", fg_magickgetimagecompose, (void *)&fh_magickgetimagecompose },
  { "magickgetimagecompression", fg_magickgetimagecompression, (void *)&fh_magickgetimagecompression },
  { "magickgetimagecompressionquality", fg_magickgetimagecompressionquality, (void *)&fh_magickgetimagecompressionquality },
  { "magickgetimagedelay", fg_magickgetimagedelay, (void *)&fh_magickgetimagedelay },
  { "magickgetimagedepth", fg_magickgetimagedepth, (void *)&fh_magickgetimagedepth },
  { "magickgetimagedispose", fg_magickgetimagedispose, (void *)&fh_magickgetimagedispose },
  { "magickgetimageextrema", fg_magickgetimageextrema, (void *)&fh_magickgetimageextrema },
  { "magickgetimagefilename", fg_magickgetimagefilename, (void *)&fh_magickgetimagefilename },
  { "magickgetimageformat", fg_magickgetimageformat, (void *)&fh_magickgetimageformat },
  { "magickgetimagegamma", fg_magickgetimagegamma, (void *)&fh_magickgetimagegamma },
  { "magickgetimagegreenprimary", fg_magickgetimagegreenprimary, (void *)&fh_magickgetimagegreenprimary },
  { "magickgetimageheight", fg_magickgetimageheight, (void *)&fh_magickgetimageheight },
  { "magickgetimagehistogram", fg_magickgetimagehistogram, (void *)&fh_magickgetimagehistogram },
  { "magickgetimageindex", fg_magickgetimageindex, (void *)&fh_magickgetimageindex },
  { "magickgetimageinterlacescheme", fg_magickgetimageinterlacescheme, (void *)&fh_magickgetimageinterlacescheme },
  { "magickgetimageiterations", fg_magickgetimageiterations, (void *)&fh_magickgetimageiterations },
  { "magickgetimagemattecolor", fg_magickgetimagemattecolor, (void *)&fh_magickgetimagemattecolor },
  { "magickgetimagemimetype", fg_magickgetimagemimetype, (void *)&fh_magickgetimagemimetype },
  { "magickgetimagepixels", fg_magickgetimagepixels, (void *)&fh_magickgetimagepixels },
  { "magickgetimageprofile", fg_magickgetimageprofile, (void *)&fh_magickgetimageprofile },
  { "magickgetimageredprimary", fg_magickgetimageredprimary, (void *)&fh_magickgetimageredprimary },
  { "magickgetimagerenderingintent", fg_magickgetimagerenderingintent, (void *)&fh_magickgetimagerenderingintent },
  { "magickgetimageresolution", fg_magickgetimageresolution, (void *)&fh_magickgetimageresolution },
  { "magickgetimagescene", fg_magickgetimagescene, (void *)&fh_magickgetimagescene },
  { "magickgetimagesignature", fg_magickgetimagesignature, (void *)&fh_magickgetimagesignature },
  { "magickgetimagesize", fg_magickgetimagesize, (void *)&fh_magickgetimagesize },
  { "magickgetimagetype", fg_magickgetimagetype, (void *)&fh_magickgetimagetype },
  { "magickgetimageunits", fg_magickgetimageunits, (void *)&fh_magickgetimageunits },
  { "magickgetimagevirtualpixelmethod", fg_magickgetimagevirtualpixelmethod, (void *)&fh_magickgetimagevirtualpixelmethod },
  { "magickgetimagewhitepoint", fg_magickgetimagewhitepoint, (void *)&fh_magickgetimagewhitepoint },
  { "magickgetimagewidth", fg_magickgetimagewidth, (void *)&fh_magickgetimagewidth },
  { "magickgetimagesblob", fg_magickgetimagesblob, (void *)&fh_magickgetimagesblob },
  { "magickgetinterlacescheme", fg_magickgetinterlacescheme, (void *)&fh_magickgetinterlacescheme },
  { "magickgetmaxtextadvance", fg_magickgetmaxtextadvance, (void *)&fh_magickgetmaxtextadvance },
  { "magickgetmimetype", fg_magickgetmimetype, (void *)&fh_magickgetmimetype },
  { "magickgetnumberimages", fg_magickgetnumberimages, (void *)&fh_magickgetnumberimages },
  { "magickgetsamplingfactors", fg_magickgetsamplingfactors, (void *)&fh_magickgetsamplingfactors },
  { "magickgetsize", fg_magickgetsize, (void *)&fh_magickgetsize },
  { "magickgetstringheight", fg_magickgetstringheight, (void *)&fh_magickgetstringheight },
  { "magickgetstringwidth", fg_magickgetstringwidth, (void *)&fh_magickgetstringwidth },
  { "magickgettextascent", fg_magickgettextascent, (void *)&fh_magickgettextascent },
  { "magickgettextdescent", fg_magickgettextdescent, (void *)&fh_magickgettextdescent },
  { "magickgetwandsize", fg_magickgetwandsize, (void *)&fh_magickgetwandsize },
  { "magickhasnextimage", fg_magickhasnextimage, (void *)&fh_magickhasnextimage },
  { "magickhaspreviousimage", fg_magickhaspreviousimage, (void *)&fh_magickhaspreviousimage },
  { "magickimplodeimage", fg_magickimplodeimage, (void *)&fh_magickimplodeimage },
  { "magicklabelimage", fg_magicklabelimage, (void *)&fh_magicklabelimage },
  { "magicklevelimage", fg_magicklevelimage, (void *)&fh_magicklevelimage },
  { "magickmagnifyimage", fg_magickmagnifyimage, (void *)&fh_magickmagnifyimage },
  { "magickmapimage", fg_magickmapimage, (void *)&fh_magickmapimage },
  { "magickmattefloodfillimage", fg_magickmattefloodfillimage, (void *)&fh_magickmattefloodfillimage },
  { "magickmedianfilterimage", fg_magickmedianfilterimage, (void *)&fh_magickmedianfilterimage },
  { "magickminifyimage", fg_magickminifyimage, (void *)&fh_magickminifyimage },
  { "magickmodulateimage", fg_magickmodulateimage, (void *)&fh_magickmodulateimage },
  { "magickmontageimage", fg_magickmontageimage, (void *)&fh_magickmontageimage },
  { "magickmorphimages", fg_magickmorphimages, (void *)&fh_magickmorphimages },
  { "magickmosaicimages", fg_magickmosaicimages, (void *)&fh_magickmosaicimages },
  { "magickmotionblurimage", fg_magickmotionblurimage, (void *)&fh_magickmotionblurimage },
  { "magicknegateimage", fg_magicknegateimage, (void *)&fh_magicknegateimage },
  { "magicknewimage", fg_magicknewimage, (void *)&fh_magicknewimage },
  { "magicknextimage", fg_magicknextimage, (void *)&fh_magicknextimage },
  { "magicknormalizeimage", fg_magicknormalizeimage, (void *)&fh_magicknormalizeimage },
  { "magickoilpaintimage", fg_magickoilpaintimage, (void *)&fh_magickoilpaintimage },
  { "magickpaintopaqueimage", fg_magickpaintopaqueimage, (void *)&fh_magickpaintopaqueimage },
  { "magickpainttransparentimage", fg_magickpainttransparentimage, (void *)&fh_magickpainttransparentimage },
  { "magickpingimage", fg_magickpingimage, (void *)&fh_magickpingimage },
  { "magickposterizeimage", fg_magickposterizeimage, (void *)&fh_magickposterizeimage },
  { "magickpreviewimages", fg_magickpreviewimages, (void *)&fh_magickpreviewimages },
  { "magickpreviousimage", fg_magickpreviousimage, (void *)&fh_magickpreviousimage },
  { "magickprofileimage", fg_magickprofileimage, (void *)&fh_magickprofileimage },
  { "magickquantizeimage", fg_magickquantizeimage, (void *)&fh_magickquantizeimage },
  { "magickquantizeimages", fg_magickquantizeimages, (void *)&fh_magickquantizeimages },
  { "magickqueryfontmetrics", fg_magickqueryfontmetrics, (void *)&fh_magickqueryfontmetrics },
  { "magickradialblurimage", fg_magickradialblurimage, (void *)&fh_magickradialblurimage },
  { "magickraiseimage", fg_magickraiseimage, (void *)&fh_magickraiseimage },
  { "magickreadimage", fg_magickreadimage, (void *)&fh_magickreadimage },
  { "magickreadimageblob", fg_magickreadimageblob, (void *)&fh_magickreadimageblob },
  { "magickreadimagefile", fg_magickreadimagefile, (void *)&fh_magickreadimagefile },
  { "magickreadimages", fg_magickreadimages, (void *)&fh_magickreadimages },
  { "magickreducenoiseimage", fg_magickreducenoiseimage, (void *)&fh_magickreducenoiseimage },
  { "magickremoveimage", fg_magickremoveimage, (void *)&fh_magickremoveimage },
  { "magickremoveimageprofile", fg_magickremoveimageprofile, (void *)&fh_magickremoveimageprofile },
  { "magickremoveimageprofiles", fg_magickremoveimageprofiles, (void *)&fh_magickremoveimageprofiles },
  { "magickresampleimage", fg_magickresampleimage, (void *)&fh_magickresampleimage },
  { "magickresetiterator", fg_magickresetiterator, (void *)&fh_magickresetiterator },
  { "magickresizeimage", fg_magickresizeimage, (void *)&fh_magickresizeimage },
  { "magickrollimage", fg_magickrollimage, (void *)&fh_magickrollimage },
  { "magickrotateimage", fg_magickrotateimage, (void *)&fh_magickrotateimage },
  { "magicksampleimage", fg_magicksampleimage, (void *)&fh_magicksampleimage },
  { "magickscaleimage", fg_magickscaleimage, (void *)&fh_magickscaleimage },
  { "magickseparateimagechannel", fg_magickseparateimagechannel, (void *)&fh_magickseparateimagechannel },
  { "magicksetcompressionquality", fg_magicksetcompressionquality, (void *)&fh_magicksetcompressionquality },
  { "magicksetfilename", fg_magicksetfilename, (void *)&fh_magicksetfilename },
  { "magicksetfirstiterator", fg_magicksetfirstiterator, (void *)&fh_magicksetfirstiterator },
  { "magicksetformat", fg_magicksetformat, (void *)&fh_magicksetformat },
  { "magicksetimage", fg_magicksetimage, (void *)&fh_magicksetimage },
  { "magicksetimagebackgroundcolor", fg_magicksetimagebackgroundcolor, (void *)&fh_magicksetimagebackgroundcolor },
  { "magicksetimagebias", fg_magicksetimagebias, (void *)&fh_magicksetimagebias },
  { "magicksetimageblueprimary", fg_magicksetimageblueprimary, (void *)&fh_magicksetimageblueprimary },
  { "magicksetimagebordercolor", fg_magicksetimagebordercolor, (void *)&fh_magicksetimagebordercolor },
  { "magicksetimagecolormapcolor", fg_magicksetimagecolormapcolor, (void *)&fh_magicksetimagecolormapcolor },
  { "magicksetimagecolorspace", fg_magicksetimagecolorspace, (void *)&fh_magicksetimagecolorspace },
  { "magicksetimagecompose", fg_magicksetimagecompose, (void *)&fh_magicksetimagecompose },
  { "magicksetimagecompression", fg_magicksetimagecompression, (void *)&fh_magicksetimagecompression },
  { "magicksetimagecompressionquality", fg_magicksetimagecompressionquality, (void *)&fh_magicksetimagecompressionquality },
  { "magicksetimagedelay", fg_magicksetimagedelay, (void *)&fh_magicksetimagedelay },
  { "magicksetimagedepth", fg_magicksetimagedepth, (void *)&fh_magicksetimagedepth },
  { "magicksetimagedispose", fg_magicksetimagedispose, (void *)&fh_magicksetimagedispose },
  { "magicksetimagefilename", fg_magicksetimagefilename, (void *)&fh_magicksetimagefilename },
  { "magicksetimageformat", fg_magicksetimageformat, (void *)&fh_magicksetimageformat },
  { "magicksetimagegamma", fg_magicksetimagegamma, (void *)&fh_magicksetimagegamma },
  { "magicksetimagegreenprimary", fg_magicksetimagegreenprimary, (void *)&fh_magicksetimagegreenprimary },
  { "magicksetimageindex", fg_magicksetimageindex, (void *)&fh_magicksetimageindex },
  { "magicksetimageinterlacescheme", fg_magicksetimageinterlacescheme, (void *)&fh_magicksetimageinterlacescheme },
  { "magicksetimageiterations", fg_magicksetimageiterations, (void *)&fh_magicksetimageiterations },
  { "magicksetimagemattecolor", fg_magicksetimagemattecolor, (void *)&fh_magicksetimagemattecolor },
  { "magicksetimageoption", fg_magicksetimageoption, (void *)&fh_magicksetimageoption },
  { "magicksetimagepixels", fg_magicksetimagepixels, (void *)&fh_magicksetimagepixels },
  { "magicksetimageprofile", fg_magicksetimageprofile, (void *)&fh_magicksetimageprofile },
  { "magicksetimageredprimary", fg_magicksetimageredprimary, (void *)&fh_magicksetimageredprimary },
  { "magicksetimagerenderingintent", fg_magicksetimagerenderingintent, (void *)&fh_magicksetimagerenderingintent },
  { "magicksetimageresolution", fg_magicksetimageresolution, (void *)&fh_magicksetimageresolution },
  { "magicksetimagescene", fg_magicksetimagescene, (void *)&fh_magicksetimagescene },
  { "magicksetimagetype", fg_magicksetimagetype, (void *)&fh_magicksetimagetype },
  { "magicksetimageunits", fg_magicksetimageunits, (void *)&fh_magicksetimageunits },
  { "magicksetimagevirtualpixelmethod", fg_magicksetimagevirtualpixelmethod, (void *)&fh_magicksetimagevirtualpixelmethod },
  { "magicksetimagewhitepoint", fg_magicksetimagewhitepoint, (void *)&fh_magicksetimagewhitepoint },
  { "magicksetinterlacescheme", fg_magicksetinterlacescheme, (void *)&fh_magicksetinterlacescheme },
  { "magicksetlastiterator", fg_magicksetlastiterator, (void *)&fh_magicksetlastiterator },
  { "magicksetpassphrase", fg_magicksetpassphrase, (void *)&fh_magicksetpassphrase },
  { "magicksetresolution", fg_magicksetresolution, (void *)&fh_magicksetresolution },
  { "magicksetsamplingfactors", fg_magicksetsamplingfactors, (void *)&fh_magicksetsamplingfactors },
  { "magicksetsize", fg_magicksetsize, (void *)&fh_magicksetsize },
  { "magicksetwandsize", fg_magicksetwandsize, (void *)&fh_magicksetwandsize },
  { "magicksharpenimage", fg_magicksharpenimage, (void *)&fh_magicksharpenimage },
  { "magickshaveimage", fg_magickshaveimage, (void *)&fh_magickshaveimage },
  { "magickshearimage", fg_magickshearimage, (void *)&fh_magickshearimage },
  { "magicksolarizeimage", fg_magicksolarizeimage, (void *)&fh_magicksolarizeimage },
  { "magickspliceimage", fg_magickspliceimage, (void *)&fh_magickspliceimage },
  { "magickspreadimage", fg_magickspreadimage, (void *)&fh_magickspreadimage },
  { "magicksteganoimage", fg_magicksteganoimage, (void *)&fh_magicksteganoimage },
  { "magickstereoimage", fg_magickstereoimage, (void *)&fh_magickstereoimage },
  { "magickstripimage", fg_magickstripimage, (void *)&fh_magickstripimage },
  { "magickswirlimage", fg_magickswirlimage, (void *)&fh_magickswirlimage },
  { "magicktextureimage", fg_magicktextureimage, (void *)&fh_magicktextureimage },
  { "magickthresholdimage", fg_magickthresholdimage, (void *)&fh_magickthresholdimage },
  { "magicktintimage", fg_magicktintimage, (void *)&fh_magicktintimage },
  { "magicktransformimage", fg_magicktransformimage, (void *)&fh_magicktransformimage },
  { "magicktrimimage", fg_magicktrimimage, (void *)&fh_magicktrimimage },
  { "magickunsharpmaskimage", fg_magickunsharpmaskimage, (void *)&fh_magickunsharpmaskimage },
  { "magickwaveimage", fg_magickwaveimage, (void *)&fh_magickwaveimage },
  { "magickwhitethresholdimage", fg_magickwhitethresholdimage, (void *)&fh_magickwhitethresholdimage },
  { "magickwriteimage", fg_magickwriteimage, (void *)&fh_magickwriteimage },
  { "magickwriteimagefile", fg_magickwriteimagefile, (void *)&fh_magickwriteimagefile },
  { "magickwriteimages", fg_magickwriteimages, (void *)&fh_magickwriteimages },
  { "magickwriteimagesfile", fg_magickwriteimagesfile, (void *)&fh_magickwriteimagesfile },
  { "pixelgetalpha", fg_pixelgetalpha, (void *)&fh_pixelgetalpha },
  { "pixelgetalphaquantum", fg_pixelgetalphaquantum, (void *)&fh_pixelgetalphaquantum },
  { "pixelgetblack", fg_pixelgetblack, (void *)&fh_pixelgetblack },
  { "pixelgetblackquantum", fg_pixelgetblackquantum, (void *)&fh_pixelgetblackquantum },
  { "pixelgetblue", fg_pixelgetblue, (void *)&fh_pixelgetblue },
  { "pixelgetbluequantum", fg_pixelgetbluequantum, (void *)&fh_pixelgetbluequantum },
  { "pixelgetcolorasstring", fg_pixelgetcolorasstring, (void *)&fh_pixelgetcolorasstring },
  { "pixelgetcolorcount", fg_pixelgetcolorcount, (void *)&fh_pixelgetcolorcount },
  { "pixelgetcyan", fg_pixelgetcyan, (void *)&fh_pixelgetcyan },
  { "pixelgetcyanquantum", fg_pixelgetcyanquantum, (void *)&fh_pixelgetcyanquantum },
  { "pixelgetexception", fg_pixelgetexception, (void *)&fh_pixelgetexception },
  { "pixelgetexceptionstring", fg_pixelgetexceptionstring, (void *)&fh_pixelgetexceptionstring },
  { "pixelgetexceptiontype", fg_pixelgetexceptiontype, (void *)&fh_pixelgetexceptiontype },
  { "pixelgetgreen", fg_pixelgetgreen, (void *)&fh_pixelgetgreen },
  { "pixelgetgreenquantum", fg_pixelgetgreenquantum, (void *)&fh_pixelgetgreenquantum },
  { "pixelgetindex", fg_pixelgetindex, (void *)&fh_pixelgetindex },
  { "pixelgetmagenta", fg_pixelgetmagenta, (void *)&fh_pixelgetmagenta },
  { "pixelgetmagentaquantum", fg_pixelgetmagentaquantum, (void *)&fh_pixelgetmagentaquantum },
  { "pixelgetopacity", fg_pixelgetopacity, (void *)&fh_pixelgetopacity },
  { "pixelgetopacityquantum", fg_pixelgetopacityquantum, (void *)&fh_pixelgetopacityquantum },
  { "pixelgetquantumcolor", fg_pixelgetquantumcolor, (void *)&fh_pixelgetquantumcolor },
  { "pixelgetred", fg_pixelgetred, (void *)&fh_pixelgetred },
  { "pixelgetredquantum", fg_pixelgetredquantum, (void *)&fh_pixelgetredquantum },
  { "pixelgetyellow", fg_pixelgetyellow, (void *)&fh_pixelgetyellow },
  { "pixelgetyellowquantum", fg_pixelgetyellowquantum, (void *)&fh_pixelgetyellowquantum },
  { "pixelsetalpha", fg_pixelsetalpha, (void *)&fh_pixelsetalpha },
  { "pixelsetalphaquantum", fg_pixelsetalphaquantum, (void *)&fh_pixelsetalphaquantum },
  { "pixelsetblack", fg_pixelsetblack, (void *)&fh_pixelsetblack },
  { "pixelsetblackquantum", fg_pixelsetblackquantum, (void *)&fh_pixelsetblackquantum },
  { "pixelsetblue", fg_pixelsetblue, (void *)&fh_pixelsetblue },
  { "pixelsetbluequantum", fg_pixelsetbluequantum, (void *)&fh_pixelsetbluequantum },
  { "pixelsetcolor", fg_pixelsetcolor, (void *)&fh_pixelsetcolor },
  { "pixelsetcolorcount", fg_pixelsetcolorcount, (void *)&fh_pixelsetcolorcount },
  { "pixelsetcyan", fg_pixelsetcyan, (void *)&fh_pixelsetcyan },
  { "pixelsetcyanquantum", fg_pixelsetcyanquantum, (void *)&fh_pixelsetcyanquantum },
  { "pixelsetgreen", fg_pixelsetgreen, (void *)&fh_pixelsetgreen },
  { "pixelsetgreenquantum", fg_pixelsetgreenquantum, (void *)&fh_pixelsetgreenquantum },
  { "pixelsetindex", fg_pixelsetindex, (void *)&fh_pixelsetindex },
  { "pixelsetmagenta", fg_pixelsetmagenta, (void *)&fh_pixelsetmagenta },
  { "pixelsetmagentaquantum", fg_pixelsetmagentaquantum, (void *)&fh_pixelsetmagentaquantum },
  { "pixelsetopacity", fg_pixelsetopacity, (void *)&fh_pixelsetopacity },
  { "pixelsetopacityquantum", fg_pixelsetopacityquantum, (void *)&fh_pixelsetopacityquantum },
  { "pixelsetquantumcolor", fg_pixelsetquantumcolor, (void *)&fh_pixelsetquantumcolor },
  { "pixelsetred", fg_pixelsetred, (void *)&fh_pixelsetred },
  { "pixelsetredquantum", fg_pixelsetredquantum, (void *)&fh_pixelsetredquantum },
  { "pixelsetyellow", fg_pixelsetyellow, (void *)&fh_pixelsetyellow },
  { "pixelsetyellowquantum", fg_pixelsetyellowquantum, (void *)&fh_pixelsetyellowquantum },
  { "pixelgetiteratorexception", fg_pixelgetiteratorexception, (void *)&fh_pixelgetiteratorexception },
  { "pixelgetiteratorexceptionstring", fg_pixelgetiteratorexceptionstring, (void *)&fh_pixelgetiteratorexceptionstring },
  { "pixelgetiteratorexceptiontype", fg_pixelgetiteratorexceptiontype, (void *)&fh_pixelgetiteratorexceptiontype },
  { "pixelgetnextiteratorrow", fg_pixelgetnextiteratorrow, (void *)&fh_pixelgetnextiteratorrow },
  { "pixelresetiterator", fg_pixelresetiterator, (void *)&fh_pixelresetiterator },
  { "pixelsetiteratorrow", fg_pixelsetiteratorrow, (void *)&fh_pixelsetiteratorrow },
  { "pixelsynciterator", fg_pixelsynciterator, (void *)&fh_pixelsynciterator },
  { "mail", fg_mail, (void *)&fh_mail },
  { "ezmlm_hash", fg_ezmlm_hash, (void *)&fh_ezmlm_hash },
  { "mailparse_msg_create", fg_mailparse_msg_create, (void *)&fh_mailparse_msg_create },
  { "mailparse_msg_free", fg_mailparse_msg_free, (void *)&fh_mailparse_msg_free },
  { "mailparse_msg_parse_file", fg_mailparse_msg_parse_file, (void *)&fh_mailparse_msg_parse_file },
  { "mailparse_msg_parse", fg_mailparse_msg_parse, (void *)&fh_mailparse_msg_parse },
  { "mailparse_msg_extract_part_file", fg_mailparse_msg_extract_part_file, (void *)&fh_mailparse_msg_extract_part_file },
  { "mailparse_msg_extract_whole_part_file", fg_mailparse_msg_extract_whole_part_file, (void *)&fh_mailparse_msg_extract_whole_part_file },
  { "mailparse_msg_extract_part", fg_mailparse_msg_extract_part, (void *)&fh_mailparse_msg_extract_part },
  { "mailparse_msg_get_part_data", fg_mailparse_msg_get_part_data, (void *)&fh_mailparse_msg_get_part_data },
  { "mailparse_msg_get_part", fg_mailparse_msg_get_part, (void *)&fh_mailparse_msg_get_part },
  { "mailparse_msg_get_structure", fg_mailparse_msg_get_structure, (void *)&fh_mailparse_msg_get_structure },
  { "mailparse_rfc822_parse_addresses", fg_mailparse_rfc822_parse_addresses, (void *)&fh_mailparse_rfc822_parse_addresses },
  { "mailparse_stream_encode", fg_mailparse_stream_encode, (void *)&fh_mailparse_stream_encode },
  { "mailparse_uudecode_all", fg_mailparse_uudecode_all, (void *)&fh_mailparse_uudecode_all },
  { "mailparse_determine_best_xfer_encoding", fg_mailparse_determine_best_xfer_encoding, (void *)&fh_mailparse_determine_best_xfer_encoding },
  { "pi", fg_pi, (void *)&fh_pi },
  { "min", fg_min, (void *)&fh_min },
  { "max", fg_max, (void *)&fh_max },
  { "abs", fg_abs, (void *)&fh_abs },
  { "is_finite", fg_is_finite, (void *)&fh_is_finite },
  { "is_infinite", fg_is_infinite, (void *)&fh_is_infinite },
  { "is_nan", fg_is_nan, (void *)&fh_is_nan },
  { "ceil", fg_ceil, (void *)&fh_ceil },
  { "floor", fg_floor, (void *)&fh_floor },
  { "round", fg_round, (void *)&fh_round },
  { "deg2rad", fg_deg2rad, (void *)&fh_deg2rad },
  { "rad2deg", fg_rad2deg, (void *)&fh_rad2deg },
  { "decbin", fg_decbin, (void *)&fh_decbin },
  { "dechex", fg_dechex, (void *)&fh_dechex },
  { "decoct", fg_decoct, (void *)&fh_decoct },
  { "bindec", fg_bindec, (void *)&fh_bindec },
  { "hexdec", fg_hexdec, (void *)&fh_hexdec },
  { "octdec", fg_octdec, (void *)&fh_octdec },
  { "base_convert", fg_base_convert, (void *)&fh_base_convert },
  { "pow", fg_pow, (void *)&fh_pow },
  { "exp", fg_exp, (void *)&fh_exp },
  { "expm1", fg_expm1, (void *)&fh_expm1 },
  { "log10", fg_log10, (void *)&fh_log10 },
  { "log1p", fg_log1p, (void *)&fh_log1p },
  { "log", fg_log, (void *)&fh_log },
  { "cos", fg_cos, (void *)&fh_cos },
  { "cosh", fg_cosh, (void *)&fh_cosh },
  { "sin", fg_sin, (void *)&fh_sin },
  { "sinh", fg_sinh, (void *)&fh_sinh },
  { "tan", fg_tan, (void *)&fh_tan },
  { "tanh", fg_tanh, (void *)&fh_tanh },
  { "acos", fg_acos, (void *)&fh_acos },
  { "acosh", fg_acosh, (void *)&fh_acosh },
  { "asin", fg_asin, (void *)&fh_asin },
  { "asinh", fg_asinh, (void *)&fh_asinh },
  { "atan", fg_atan, (void *)&fh_atan },
  { "atanh", fg_atanh, (void *)&fh_atanh },
  { "atan2", fg_atan2, (void *)&fh_atan2 },
  { "hypot", fg_hypot, (void *)&fh_hypot },
  { "fmod", fg_fmod, (void *)&fh_fmod },
  { "sqrt", fg_sqrt, (void *)&fh_sqrt },
  { "getrandmax", fg_getrandmax, (void *)&fh_getrandmax },
  { "srand", fg_srand, (void *)&fh_srand },
  { "rand", fg_rand, (void *)&fh_rand },
  { "mt_getrandmax", fg_mt_getrandmax, (void *)&fh_mt_getrandmax },
  { "mt_srand", fg_mt_srand, (void *)&fh_mt_srand },
  { "mt_rand", fg_mt_rand, (void *)&fh_mt_rand },
  { "lcg_value", fg_lcg_value, (void *)&fh_lcg_value },
  { "mb_list_encodings", fg_mb_list_encodings, (void *)&fh_mb_list_encodings },
  { "mb_list_encodings_alias_names", fg_mb_list_encodings_alias_names, (void *)&fh_mb_list_encodings_alias_names },
  { "mb_list_mime_names", fg_mb_list_mime_names, (void *)&fh_mb_list_mime_names },
  { "mb_check_encoding", fg_mb_check_encoding, (void *)&fh_mb_check_encoding },
  { "mb_convert_case", fg_mb_convert_case, (void *)&fh_mb_convert_case },
  { "mb_convert_encoding", fg_mb_convert_encoding, (void *)&fh_mb_convert_encoding },
  { "mb_convert_kana", fg_mb_convert_kana, (void *)&fh_mb_convert_kana },
  { "mb_convert_variables", fg_mb_convert_variables, (void *)&fh_mb_convert_variables },
  { "mb_decode_mimeheader", fg_mb_decode_mimeheader, (void *)&fh_mb_decode_mimeheader },
  { "mb_decode_numericentity", fg_mb_decode_numericentity, (void *)&fh_mb_decode_numericentity },
  { "mb_detect_encoding", fg_mb_detect_encoding, (void *)&fh_mb_detect_encoding },
  { "mb_detect_order", fg_mb_detect_order, (void *)&fh_mb_detect_order },
  { "mb_encode_mimeheader", fg_mb_encode_mimeheader, (void *)&fh_mb_encode_mimeheader },
  { "mb_encode_numericentity", fg_mb_encode_numericentity, (void *)&fh_mb_encode_numericentity },
  { "mb_ereg_match", fg_mb_ereg_match, (void *)&fh_mb_ereg_match },
  { "mb_ereg_replace", fg_mb_ereg_replace, (void *)&fh_mb_ereg_replace },
  { "mb_ereg_search_getpos", fg_mb_ereg_search_getpos, (void *)&fh_mb_ereg_search_getpos },
  { "mb_ereg_search_getregs", fg_mb_ereg_search_getregs, (void *)&fh_mb_ereg_search_getregs },
  { "mb_ereg_search_init", fg_mb_ereg_search_init, (void *)&fh_mb_ereg_search_init },
  { "mb_ereg_search_pos", fg_mb_ereg_search_pos, (void *)&fh_mb_ereg_search_pos },
  { "mb_ereg_search_regs", fg_mb_ereg_search_regs, (void *)&fh_mb_ereg_search_regs },
  { "mb_ereg_search_setpos", fg_mb_ereg_search_setpos, (void *)&fh_mb_ereg_search_setpos },
  { "mb_ereg_search", fg_mb_ereg_search, (void *)&fh_mb_ereg_search },
  { "mb_ereg", fg_mb_ereg, (void *)&fh_mb_ereg },
  { "mb_eregi_replace", fg_mb_eregi_replace, (void *)&fh_mb_eregi_replace },
  { "mb_eregi", fg_mb_eregi, (void *)&fh_mb_eregi },
  { "mb_get_info", fg_mb_get_info, (void *)&fh_mb_get_info },
  { "mb_http_input", fg_mb_http_input, (void *)&fh_mb_http_input },
  { "mb_http_output", fg_mb_http_output, (void *)&fh_mb_http_output },
  { "mb_internal_encoding", fg_mb_internal_encoding, (void *)&fh_mb_internal_encoding },
  { "mb_language", fg_mb_language, (void *)&fh_mb_language },
  { "mb_output_handler", fg_mb_output_handler, (void *)&fh_mb_output_handler },
  { "mb_parse_str", fg_mb_parse_str, (void *)&fh_mb_parse_str },
  { "mb_preferred_mime_name", fg_mb_preferred_mime_name, (void *)&fh_mb_preferred_mime_name },
  { "mb_regex_encoding", fg_mb_regex_encoding, (void *)&fh_mb_regex_encoding },
  { "mb_regex_set_options", fg_mb_regex_set_options, (void *)&fh_mb_regex_set_options },
  { "mb_send_mail", fg_mb_send_mail, (void *)&fh_mb_send_mail },
  { "mb_split", fg_mb_split, (void *)&fh_mb_split },
  { "mb_strcut", fg_mb_strcut, (void *)&fh_mb_strcut },
  { "mb_strimwidth", fg_mb_strimwidth, (void *)&fh_mb_strimwidth },
  { "mb_stripos", fg_mb_stripos, (void *)&fh_mb_stripos },
  { "mb_stristr", fg_mb_stristr, (void *)&fh_mb_stristr },
  { "mb_strlen", fg_mb_strlen, (void *)&fh_mb_strlen },
  { "mb_strpos", fg_mb_strpos, (void *)&fh_mb_strpos },
  { "mb_strrchr", fg_mb_strrchr, (void *)&fh_mb_strrchr },
  { "mb_strrichr", fg_mb_strrichr, (void *)&fh_mb_strrichr },
  { "mb_strripos", fg_mb_strripos, (void *)&fh_mb_strripos },
  { "mb_strrpos", fg_mb_strrpos, (void *)&fh_mb_strrpos },
  { "mb_strstr", fg_mb_strstr, (void *)&fh_mb_strstr },
  { "mb_strtolower", fg_mb_strtolower, (void *)&fh_mb_strtolower },
  { "mb_strtoupper", fg_mb_strtoupper, (void *)&fh_mb_strtoupper },
  { "mb_strwidth", fg_mb_strwidth, (void *)&fh_mb_strwidth },
  { "mb_substitute_character", fg_mb_substitute_character, (void *)&fh_mb_substitute_character },
  { "mb_substr_count", fg_mb_substr_count, (void *)&fh_mb_substr_count },
  { "mb_substr", fg_mb_substr, (void *)&fh_mb_substr },
  { "mcrypt_module_open", fg_mcrypt_module_open, (void *)&fh_mcrypt_module_open },
  { "mcrypt_module_close", fg_mcrypt_module_close, (void *)&fh_mcrypt_module_close },
  { "mcrypt_list_algorithms", fg_mcrypt_list_algorithms, (void *)&fh_mcrypt_list_algorithms },
  { "mcrypt_list_modes", fg_mcrypt_list_modes, (void *)&fh_mcrypt_list_modes },
  { "mcrypt_module_get_algo_block_size", fg_mcrypt_module_get_algo_block_size, (void *)&fh_mcrypt_module_get_algo_block_size },
  { "mcrypt_module_get_algo_key_size", fg_mcrypt_module_get_algo_key_size, (void *)&fh_mcrypt_module_get_algo_key_size },
  { "mcrypt_module_get_supported_key_sizes", fg_mcrypt_module_get_supported_key_sizes, (void *)&fh_mcrypt_module_get_supported_key_sizes },
  { "mcrypt_module_is_block_algorithm_mode", fg_mcrypt_module_is_block_algorithm_mode, (void *)&fh_mcrypt_module_is_block_algorithm_mode },
  { "mcrypt_module_is_block_algorithm", fg_mcrypt_module_is_block_algorithm, (void *)&fh_mcrypt_module_is_block_algorithm },
  { "mcrypt_module_is_block_mode", fg_mcrypt_module_is_block_mode, (void *)&fh_mcrypt_module_is_block_mode },
  { "mcrypt_module_self_test", fg_mcrypt_module_self_test, (void *)&fh_mcrypt_module_self_test },
  { "mcrypt_create_iv", fg_mcrypt_create_iv, (void *)&fh_mcrypt_create_iv },
  { "mcrypt_encrypt", fg_mcrypt_encrypt, (void *)&fh_mcrypt_encrypt },
  { "mcrypt_decrypt", fg_mcrypt_decrypt, (void *)&fh_mcrypt_decrypt },
  { "mcrypt_cbc", fg_mcrypt_cbc, (void *)&fh_mcrypt_cbc },
  { "mcrypt_cfb", fg_mcrypt_cfb, (void *)&fh_mcrypt_cfb },
  { "mcrypt_ecb", fg_mcrypt_ecb, (void *)&fh_mcrypt_ecb },
  { "mcrypt_ofb", fg_mcrypt_ofb, (void *)&fh_mcrypt_ofb },
  { "mcrypt_get_block_size", fg_mcrypt_get_block_size, (void *)&fh_mcrypt_get_block_size },
  { "mcrypt_get_cipher_name", fg_mcrypt_get_cipher_name, (void *)&fh_mcrypt_get_cipher_name },
  { "mcrypt_get_iv_size", fg_mcrypt_get_iv_size, (void *)&fh_mcrypt_get_iv_size },
  { "mcrypt_get_key_size", fg_mcrypt_get_key_size, (void *)&fh_mcrypt_get_key_size },
  { "mcrypt_enc_get_algorithms_name", fg_mcrypt_enc_get_algorithms_name, (void *)&fh_mcrypt_enc_get_algorithms_name },
  { "mcrypt_enc_get_block_size", fg_mcrypt_enc_get_block_size, (void *)&fh_mcrypt_enc_get_block_size },
  { "mcrypt_enc_get_iv_size", fg_mcrypt_enc_get_iv_size, (void *)&fh_mcrypt_enc_get_iv_size },
  { "mcrypt_enc_get_key_size", fg_mcrypt_enc_get_key_size, (void *)&fh_mcrypt_enc_get_key_size },
  { "mcrypt_enc_get_modes_name", fg_mcrypt_enc_get_modes_name, (void *)&fh_mcrypt_enc_get_modes_name },
  { "mcrypt_enc_get_supported_key_sizes", fg_mcrypt_enc_get_supported_key_sizes, (void *)&fh_mcrypt_enc_get_supported_key_sizes },
  { "mcrypt_enc_is_block_algorithm_mode", fg_mcrypt_enc_is_block_algorithm_mode, (void *)&fh_mcrypt_enc_is_block_algorithm_mode },
  { "mcrypt_enc_is_block_algorithm", fg_mcrypt_enc_is_block_algorithm, (void *)&fh_mcrypt_enc_is_block_algorithm },
  { "mcrypt_enc_is_block_mode", fg_mcrypt_enc_is_block_mode, (void *)&fh_mcrypt_enc_is_block_mode },
  { "mcrypt_enc_self_test", fg_mcrypt_enc_self_test, (void *)&fh_mcrypt_enc_self_test },
  { "mcrypt_generic", fg_mcrypt_generic, (void *)&fh_mcrypt_generic },
  { "mcrypt_generic_init", fg_mcrypt_generic_init, (void *)&fh_mcrypt_generic_init },
  { "mdecrypt_generic", fg_mdecrypt_generic, (void *)&fh_mdecrypt_generic },
  { "mcrypt_generic_deinit", fg_mcrypt_generic_deinit, (void *)&fh_mcrypt_generic_deinit },
  { "mcrypt_generic_end", fg_mcrypt_generic_end, (void *)&fh_mcrypt_generic_end },
  { "memcache_connect", fg_memcache_connect, (void *)&fh_memcache_connect },
  { "memcache_pconnect", fg_memcache_pconnect, (void *)&fh_memcache_pconnect },
  { "memcache_add", fg_memcache_add, (void *)&fh_memcache_add },
  { "memcache_set", fg_memcache_set, (void *)&fh_memcache_set },
  { "memcache_replace", fg_memcache_replace, (void *)&fh_memcache_replace },
  { "memcache_get", fg_memcache_get, (void *)&fh_memcache_get },
  { "memcache_delete", fg_memcache_delete, (void *)&fh_memcache_delete },
  { "memcache_increment", fg_memcache_increment, (void *)&fh_memcache_increment },
  { "memcache_decrement", fg_memcache_decrement, (void *)&fh_memcache_decrement },
  { "memcache_close", fg_memcache_close, (void *)&fh_memcache_close },
  { "memcache_debug", fg_memcache_debug, (void *)&fh_memcache_debug },
  { "memcache_get_version", fg_memcache_get_version, (void *)&fh_memcache_get_version },
  { "memcache_flush", fg_memcache_flush, (void *)&fh_memcache_flush },
  { "memcache_setoptimeout", fg_memcache_setoptimeout, (void *)&fh_memcache_setoptimeout },
  { "memcache_get_server_status", fg_memcache_get_server_status, (void *)&fh_memcache_get_server_status },
  { "memcache_set_compress_threshold", fg_memcache_set_compress_threshold, (void *)&fh_memcache_set_compress_threshold },
  { "memcache_get_stats", fg_memcache_get_stats, (void *)&fh_memcache_get_stats },
  { "memcache_get_extended_stats", fg_memcache_get_extended_stats, (void *)&fh_memcache_get_extended_stats },
  { "memcache_set_server_params", fg_memcache_set_server_params, (void *)&fh_memcache_set_server_params },
  { "memcache_add_server", fg_memcache_add_server, (void *)&fh_memcache_add_server },
  { "connection_aborted", fg_connection_aborted, (void *)&fh_connection_aborted },
  { "connection_status", fg_connection_status, (void *)&fh_connection_status },
  { "connection_timeout", fg_connection_timeout, (void *)&fh_connection_timeout },
  { "constant", fg_constant, (void *)&fh_constant },
  { "define", fg_define, (void *)&fh_define },
  { "defined", fg_defined, (void *)&fh_defined },
  { "die", fg_die, (void *)&fh_die },
  { "exit", fg_exit, (void *)&fh_exit },
  { "eval", fg_eval, (void *)&fh_eval },
  { "get_browser", fg_get_browser, (void *)&fh_get_browser },
  { "__halt_compiler", fg___halt_compiler, (void *)&fh___halt_compiler },
  { "highlight_file", fg_highlight_file, (void *)&fh_highlight_file },
  { "show_source", fg_show_source, (void *)&fh_show_source },
  { "highlight_string", fg_highlight_string, (void *)&fh_highlight_string },
  { "ignore_user_abort", fg_ignore_user_abort, (void *)&fh_ignore_user_abort },
  { "pack", fg_pack, (void *)&fh_pack },
  { "php_check_syntax", fg_php_check_syntax, (void *)&fh_php_check_syntax },
  { "php_strip_whitespace", fg_php_strip_whitespace, (void *)&fh_php_strip_whitespace },
  { "sleep", fg_sleep, (void *)&fh_sleep },
  { "usleep", fg_usleep, (void *)&fh_usleep },
  { "time_nanosleep", fg_time_nanosleep, (void *)&fh_time_nanosleep },
  { "time_sleep_until", fg_time_sleep_until, (void *)&fh_time_sleep_until },
  { "uniqid", fg_uniqid, (void *)&fh_uniqid },
  { "unpack", fg_unpack, (void *)&fh_unpack },
  { "sys_getloadavg", fg_sys_getloadavg, (void *)&fh_sys_getloadavg },
  { "token_get_all", fg_token_get_all, (void *)&fh_token_get_all },
  { "token_name", fg_token_name, (void *)&fh_token_name },
  { "hphp_process_abort", fg_hphp_process_abort, (void *)&fh_hphp_process_abort },
  { "hphp_to_string", fg_hphp_to_string, (void *)&fh_hphp_to_string },
  { "mysql_connect", fg_mysql_connect, (void *)&fh_mysql_connect },
  { "mysql_async_connect_start", fg_mysql_async_connect_start, (void *)&fh_mysql_async_connect_start },
  { "mysql_async_connect_completed", fg_mysql_async_connect_completed, (void *)&fh_mysql_async_connect_completed },
  { "mysql_async_query_start", fg_mysql_async_query_start, (void *)&fh_mysql_async_query_start },
  { "mysql_async_query_result", fg_mysql_async_query_result, (void *)&fh_mysql_async_query_result },
  { "mysql_async_query_completed", fg_mysql_async_query_completed, (void *)&fh_mysql_async_query_completed },
  { "mysql_async_fetch_array", fg_mysql_async_fetch_array, (void *)&fh_mysql_async_fetch_array },
  { "mysql_async_wait_actionable", fg_mysql_async_wait_actionable, (void *)&fh_mysql_async_wait_actionable },
  { "mysql_async_status", fg_mysql_async_status, (void *)&fh_mysql_async_status },
  { "mysql_pconnect", fg_mysql_pconnect, (void *)&fh_mysql_pconnect },
  { "mysql_connect_with_db", fg_mysql_connect_with_db, (void *)&fh_mysql_connect_with_db },
  { "mysql_pconnect_with_db", fg_mysql_pconnect_with_db, (void *)&fh_mysql_pconnect_with_db },
  { "mysql_set_charset", fg_mysql_set_charset, (void *)&fh_mysql_set_charset },
  { "mysql_ping", fg_mysql_ping, (void *)&fh_mysql_ping },
  { "mysql_escape_string", fg_mysql_escape_string, (void *)&fh_mysql_escape_string },
  { "mysql_real_escape_string", fg_mysql_real_escape_string, (void *)&fh_mysql_real_escape_string },
  { "mysql_client_encoding", fg_mysql_client_encoding, (void *)&fh_mysql_client_encoding },
  { "mysql_close", fg_mysql_close, (void *)&fh_mysql_close },
  { "mysql_errno", fg_mysql_errno, (void *)&fh_mysql_errno },
  { "mysql_error", fg_mysql_error, (void *)&fh_mysql_error },
  { "mysql_warning_count", fg_mysql_warning_count, (void *)&fh_mysql_warning_count },
  { "mysql_get_client_info", fg_mysql_get_client_info, (void *)&fh_mysql_get_client_info },
  { "mysql_get_host_info", fg_mysql_get_host_info, (void *)&fh_mysql_get_host_info },
  { "mysql_get_proto_info", fg_mysql_get_proto_info, (void *)&fh_mysql_get_proto_info },
  { "mysql_get_server_info", fg_mysql_get_server_info, (void *)&fh_mysql_get_server_info },
  { "mysql_info", fg_mysql_info, (void *)&fh_mysql_info },
  { "mysql_insert_id", fg_mysql_insert_id, (void *)&fh_mysql_insert_id },
  { "mysql_stat", fg_mysql_stat, (void *)&fh_mysql_stat },
  { "mysql_thread_id", fg_mysql_thread_id, (void *)&fh_mysql_thread_id },
  { "mysql_create_db", fg_mysql_create_db, (void *)&fh_mysql_create_db },
  { "mysql_select_db", fg_mysql_select_db, (void *)&fh_mysql_select_db },
  { "mysql_drop_db", fg_mysql_drop_db, (void *)&fh_mysql_drop_db },
  { "mysql_affected_rows", fg_mysql_affected_rows, (void *)&fh_mysql_affected_rows },
  { "mysql_set_timeout", fg_mysql_set_timeout, (void *)&fh_mysql_set_timeout },
  { "mysql_query", fg_mysql_query, (void *)&fh_mysql_query },
  { "mysql_multi_query", fg_mysql_multi_query, (void *)&fh_mysql_multi_query },
  { "mysql_next_result", fg_mysql_next_result, (void *)&fh_mysql_next_result },
  { "mysql_more_results", fg_mysql_more_results, (void *)&fh_mysql_more_results },
  { "mysql_fetch_result", fg_mysql_fetch_result, (void *)&fh_mysql_fetch_result },
  { "mysql_unbuffered_query", fg_mysql_unbuffered_query, (void *)&fh_mysql_unbuffered_query },
  { "mysql_db_query", fg_mysql_db_query, (void *)&fh_mysql_db_query },
  { "mysql_list_dbs", fg_mysql_list_dbs, (void *)&fh_mysql_list_dbs },
  { "mysql_list_tables", fg_mysql_list_tables, (void *)&fh_mysql_list_tables },
  { "mysql_list_fields", fg_mysql_list_fields, (void *)&fh_mysql_list_fields },
  { "mysql_list_processes", fg_mysql_list_processes, (void *)&fh_mysql_list_processes },
  { "mysql_db_name", fg_mysql_db_name, (void *)&fh_mysql_db_name },
  { "mysql_tablename", fg_mysql_tablename, (void *)&fh_mysql_tablename },
  { "mysql_num_fields", fg_mysql_num_fields, (void *)&fh_mysql_num_fields },
  { "mysql_num_rows", fg_mysql_num_rows, (void *)&fh_mysql_num_rows },
  { "mysql_free_result", fg_mysql_free_result, (void *)&fh_mysql_free_result },
  { "mysql_data_seek", fg_mysql_data_seek, (void *)&fh_mysql_data_seek },
  { "mysql_fetch_row", fg_mysql_fetch_row, (void *)&fh_mysql_fetch_row },
  { "mysql_fetch_assoc", fg_mysql_fetch_assoc, (void *)&fh_mysql_fetch_assoc },
  { "mysql_fetch_array", fg_mysql_fetch_array, (void *)&fh_mysql_fetch_array },
  { "mysql_fetch_lengths", fg_mysql_fetch_lengths, (void *)&fh_mysql_fetch_lengths },
  { "mysql_fetch_object", fg_mysql_fetch_object, (void *)&fh_mysql_fetch_object },
  { "mysql_result", fg_mysql_result, (void *)&fh_mysql_result },
  { "mysql_fetch_field", fg_mysql_fetch_field, (void *)&fh_mysql_fetch_field },
  { "mysql_field_seek", fg_mysql_field_seek, (void *)&fh_mysql_field_seek },
  { "mysql_field_name", fg_mysql_field_name, (void *)&fh_mysql_field_name },
  { "mysql_field_table", fg_mysql_field_table, (void *)&fh_mysql_field_table },
  { "mysql_field_len", fg_mysql_field_len, (void *)&fh_mysql_field_len },
  { "mysql_field_type", fg_mysql_field_type, (void *)&fh_mysql_field_type },
  { "mysql_field_flags", fg_mysql_field_flags, (void *)&fh_mysql_field_flags },
  { "gethostname", fg_gethostname, (void *)&fh_gethostname },
  { "gethostbyaddr", fg_gethostbyaddr, (void *)&fh_gethostbyaddr },
  { "gethostbyname", fg_gethostbyname, (void *)&fh_gethostbyname },
  { "gethostbynamel", fg_gethostbynamel, (void *)&fh_gethostbynamel },
  { "getprotobyname", fg_getprotobyname, (void *)&fh_getprotobyname },
  { "getprotobynumber", fg_getprotobynumber, (void *)&fh_getprotobynumber },
  { "getservbyname", fg_getservbyname, (void *)&fh_getservbyname },
  { "getservbyport", fg_getservbyport, (void *)&fh_getservbyport },
  { "inet_ntop", fg_inet_ntop, (void *)&fh_inet_ntop },
  { "inet_pton", fg_inet_pton, (void *)&fh_inet_pton },
  { "ip2long", fg_ip2long, (void *)&fh_ip2long },
  { "long2ip", fg_long2ip, (void *)&fh_long2ip },
  { "dns_check_record", fg_dns_check_record, (void *)&fh_dns_check_record },
  { "checkdnsrr", fg_checkdnsrr, (void *)&fh_checkdnsrr },
  { "dns_get_record", fg_dns_get_record, (void *)&fh_dns_get_record },
  { "dns_get_mx", fg_dns_get_mx, (void *)&fh_dns_get_mx },
  { "getmxrr", fg_getmxrr, (void *)&fh_getmxrr },
  { "fsockopen", fg_fsockopen, (void *)&fh_fsockopen },
  { "pfsockopen", fg_pfsockopen, (void *)&fh_pfsockopen },
  { "socket_get_status", fg_socket_get_status, (void *)&fh_socket_get_status },
  { "socket_set_blocking", fg_socket_set_blocking, (void *)&fh_socket_set_blocking },
  { "socket_set_timeout", fg_socket_set_timeout, (void *)&fh_socket_set_timeout },
  { "header", fg_header, (void *)&fh_header },
  { "http_response_code", fg_http_response_code, (void *)&fh_http_response_code },
  { "headers_list", fg_headers_list, (void *)&fh_headers_list },
  { "get_http_request_size", fg_get_http_request_size, (void *)&fh_get_http_request_size },
  { "headers_sent", fg_headers_sent, (void *)&fh_headers_sent },
  { "header_register_callback", fg_header_register_callback, (void *)&fh_header_register_callback },
  { "header_remove", fg_header_remove, (void *)&fh_header_remove },
  { "setcookie", fg_setcookie, (void *)&fh_setcookie },
  { "setrawcookie", fg_setrawcookie, (void *)&fh_setrawcookie },
  { "define_syslog_variables", fg_define_syslog_variables, (void *)&fh_define_syslog_variables },
  { "openlog", fg_openlog, (void *)&fh_openlog },
  { "closelog", fg_closelog, (void *)&fh_closelog },
  { "syslog", fg_syslog, (void *)&fh_syslog },
  { "openssl_csr_export_to_file", fg_openssl_csr_export_to_file, (void *)&fh_openssl_csr_export_to_file },
  { "openssl_csr_export", fg_openssl_csr_export, (void *)&fh_openssl_csr_export },
  { "openssl_csr_get_public_key", fg_openssl_csr_get_public_key, (void *)&fh_openssl_csr_get_public_key },
  { "openssl_csr_get_subject", fg_openssl_csr_get_subject, (void *)&fh_openssl_csr_get_subject },
  { "openssl_csr_new", fg_openssl_csr_new, (void *)&fh_openssl_csr_new },
  { "openssl_csr_sign", fg_openssl_csr_sign, (void *)&fh_openssl_csr_sign },
  { "openssl_error_string", fg_openssl_error_string, (void *)&fh_openssl_error_string },
  { "openssl_open", fg_openssl_open, (void *)&fh_openssl_open },
  { "openssl_pkcs12_export_to_file", fg_openssl_pkcs12_export_to_file, (void *)&fh_openssl_pkcs12_export_to_file },
  { "openssl_pkcs12_export", fg_openssl_pkcs12_export, (void *)&fh_openssl_pkcs12_export },
  { "openssl_pkcs12_read", fg_openssl_pkcs12_read, (void *)&fh_openssl_pkcs12_read },
  { "openssl_pkcs7_decrypt", fg_openssl_pkcs7_decrypt, (void *)&fh_openssl_pkcs7_decrypt },
  { "openssl_pkcs7_encrypt", fg_openssl_pkcs7_encrypt, (void *)&fh_openssl_pkcs7_encrypt },
  { "openssl_pkcs7_sign", fg_openssl_pkcs7_sign, (void *)&fh_openssl_pkcs7_sign },
  { "openssl_pkcs7_verify", fg_openssl_pkcs7_verify, (void *)&fh_openssl_pkcs7_verify },
  { "openssl_pkey_export_to_file", fg_openssl_pkey_export_to_file, (void *)&fh_openssl_pkey_export_to_file },
  { "openssl_pkey_export", fg_openssl_pkey_export, (void *)&fh_openssl_pkey_export },
  { "openssl_pkey_free", fg_openssl_pkey_free, (void *)&fh_openssl_pkey_free },
  { "openssl_free_key", fg_openssl_free_key, (void *)&fh_openssl_free_key },
  { "openssl_pkey_get_details", fg_openssl_pkey_get_details, (void *)&fh_openssl_pkey_get_details },
  { "openssl_pkey_get_private", fg_openssl_pkey_get_private, (void *)&fh_openssl_pkey_get_private },
  { "openssl_get_privatekey", fg_openssl_get_privatekey, (void *)&fh_openssl_get_privatekey },
  { "openssl_pkey_get_public", fg_openssl_pkey_get_public, (void *)&fh_openssl_pkey_get_public },
  { "openssl_get_publickey", fg_openssl_get_publickey, (void *)&fh_openssl_get_publickey },
  { "openssl_pkey_new", fg_openssl_pkey_new, (void *)&fh_openssl_pkey_new },
  { "openssl_private_decrypt", fg_openssl_private_decrypt, (void *)&fh_openssl_private_decrypt },
  { "openssl_private_encrypt", fg_openssl_private_encrypt, (void *)&fh_openssl_private_encrypt },
  { "openssl_public_decrypt", fg_openssl_public_decrypt, (void *)&fh_openssl_public_decrypt },
  { "openssl_public_encrypt", fg_openssl_public_encrypt, (void *)&fh_openssl_public_encrypt },
  { "openssl_seal", fg_openssl_seal, (void *)&fh_openssl_seal },
  { "openssl_sign", fg_openssl_sign, (void *)&fh_openssl_sign },
  { "openssl_verify", fg_openssl_verify, (void *)&fh_openssl_verify },
  { "openssl_x509_check_private_key", fg_openssl_x509_check_private_key, (void *)&fh_openssl_x509_check_private_key },
  { "openssl_x509_checkpurpose", fg_openssl_x509_checkpurpose, (void *)&fh_openssl_x509_checkpurpose },
  { "openssl_x509_export_to_file", fg_openssl_x509_export_to_file, (void *)&fh_openssl_x509_export_to_file },
  { "openssl_x509_export", fg_openssl_x509_export, (void *)&fh_openssl_x509_export },
  { "openssl_x509_free", fg_openssl_x509_free, (void *)&fh_openssl_x509_free },
  { "openssl_x509_parse", fg_openssl_x509_parse, (void *)&fh_openssl_x509_parse },
  { "openssl_x509_read", fg_openssl_x509_read, (void *)&fh_openssl_x509_read },
  { "openssl_random_pseudo_bytes", fg_openssl_random_pseudo_bytes, (void *)&fh_openssl_random_pseudo_bytes },
  { "openssl_cipher_iv_length", fg_openssl_cipher_iv_length, (void *)&fh_openssl_cipher_iv_length },
  { "openssl_encrypt", fg_openssl_encrypt, (void *)&fh_openssl_encrypt },
  { "openssl_decrypt", fg_openssl_decrypt, (void *)&fh_openssl_decrypt },
  { "openssl_digest", fg_openssl_digest, (void *)&fh_openssl_digest },
  { "openssl_get_cipher_methods", fg_openssl_get_cipher_methods, (void *)&fh_openssl_get_cipher_methods },
  { "openssl_get_md_methods", fg_openssl_get_md_methods, (void *)&fh_openssl_get_md_methods },
  { "assert_options", fg_assert_options, (void *)&fh_assert_options },
  { "assert", fg_assert, (void *)&fh_assert },
  { "dl", fg_dl, (void *)&fh_dl },
  { "extension_loaded", fg_extension_loaded, (void *)&fh_extension_loaded },
  { "get_loaded_extensions", fg_get_loaded_extensions, (void *)&fh_get_loaded_extensions },
  { "get_extension_funcs", fg_get_extension_funcs, (void *)&fh_get_extension_funcs },
  { "get_cfg_var", fg_get_cfg_var, (void *)&fh_get_cfg_var },
  { "get_current_user", fg_get_current_user, (void *)&fh_get_current_user },
  { "get_defined_constants", fg_get_defined_constants, (void *)&fh_get_defined_constants },
  { "get_include_path", fg_get_include_path, (void *)&fh_get_include_path },
  { "restore_include_path", fg_restore_include_path, (void *)&fh_restore_include_path },
  { "set_include_path", fg_set_include_path, (void *)&fh_set_include_path },
  { "get_included_files", fg_get_included_files, (void *)&fh_get_included_files },
  { "inclued_get_data", fg_inclued_get_data, (void *)&fh_inclued_get_data },
  { "get_magic_quotes_gpc", fg_get_magic_quotes_gpc, (void *)&fh_get_magic_quotes_gpc },
  { "get_magic_quotes_runtime", fg_get_magic_quotes_runtime, (void *)&fh_get_magic_quotes_runtime },
  { "get_required_files", fg_get_required_files, (void *)&fh_get_required_files },
  { "getenv", fg_getenv, (void *)&fh_getenv },
  { "getlastmod", fg_getlastmod, (void *)&fh_getlastmod },
  { "getmygid", fg_getmygid, (void *)&fh_getmygid },
  { "getmyinode", fg_getmyinode, (void *)&fh_getmyinode },
  { "getmypid", fg_getmypid, (void *)&fh_getmypid },
  { "getmyuid", fg_getmyuid, (void *)&fh_getmyuid },
  { "getopt", fg_getopt, (void *)&fh_getopt },
  { "getrusage", fg_getrusage, (void *)&fh_getrusage },
  { "clock_getres", fg_clock_getres, (void *)&fh_clock_getres },
  { "clock_gettime", fg_clock_gettime, (void *)&fh_clock_gettime },
  { "clock_settime", fg_clock_settime, (void *)&fh_clock_settime },
  { "cpu_get_count", fg_cpu_get_count, (void *)&fh_cpu_get_count },
  { "cpu_get_model", fg_cpu_get_model, (void *)&fh_cpu_get_model },
  { "ini_alter", fg_ini_alter, (void *)&fh_ini_alter },
  { "ini_get_all", fg_ini_get_all, (void *)&fh_ini_get_all },
  { "ini_get", fg_ini_get, (void *)&fh_ini_get },
  { "ini_restore", fg_ini_restore, (void *)&fh_ini_restore },
  { "ini_set", fg_ini_set, (void *)&fh_ini_set },
  { "memory_get_allocation", fg_memory_get_allocation, (void *)&fh_memory_get_allocation },
  { "memory_get_peak_usage", fg_memory_get_peak_usage, (void *)&fh_memory_get_peak_usage },
  { "memory_get_usage", fg_memory_get_usage, (void *)&fh_memory_get_usage },
  { "php_ini_scanned_files", fg_php_ini_scanned_files, (void *)&fh_php_ini_scanned_files },
  { "php_logo_guid", fg_php_logo_guid, (void *)&fh_php_logo_guid },
  { "php_sapi_name", fg_php_sapi_name, (void *)&fh_php_sapi_name },
  { "php_uname", fg_php_uname, (void *)&fh_php_uname },
  { "phpcredits", fg_phpcredits, (void *)&fh_phpcredits },
  { "phpinfo", fg_phpinfo, (void *)&fh_phpinfo },
  { "phpversion", fg_phpversion, (void *)&fh_phpversion },
  { "putenv", fg_putenv, (void *)&fh_putenv },
  { "set_magic_quotes_runtime", fg_set_magic_quotes_runtime, (void *)&fh_set_magic_quotes_runtime },
  { "set_time_limit", fg_set_time_limit, (void *)&fh_set_time_limit },
  { "sys_get_temp_dir", fg_sys_get_temp_dir, (void *)&fh_sys_get_temp_dir },
  { "version_compare", fg_version_compare, (void *)&fh_version_compare },
  { "gc_enabled", fg_gc_enabled, (void *)&fh_gc_enabled },
  { "gc_enable", fg_gc_enable, (void *)&fh_gc_enable },
  { "gc_disable", fg_gc_disable, (void *)&fh_gc_disable },
  { "gc_collect_cycles", fg_gc_collect_cycles, (void *)&fh_gc_collect_cycles },
  { "zend_logo_guid", fg_zend_logo_guid, (void *)&fh_zend_logo_guid },
  { "zend_thread_id", fg_zend_thread_id, (void *)&fh_zend_thread_id },
  { "zend_version", fg_zend_version, (void *)&fh_zend_version },
  { "ob_start", fg_ob_start, (void *)&fh_ob_start },
  { "ob_clean", fg_ob_clean, (void *)&fh_ob_clean },
  { "ob_flush", fg_ob_flush, (void *)&fh_ob_flush },
  { "ob_end_clean", fg_ob_end_clean, (void *)&fh_ob_end_clean },
  { "ob_end_flush", fg_ob_end_flush, (void *)&fh_ob_end_flush },
  { "flush", fg_flush, (void *)&fh_flush },
  { "ob_get_clean", fg_ob_get_clean, (void *)&fh_ob_get_clean },
  { "ob_get_contents", fg_ob_get_contents, (void *)&fh_ob_get_contents },
  { "ob_get_flush", fg_ob_get_flush, (void *)&fh_ob_get_flush },
  { "ob_get_length", fg_ob_get_length, (void *)&fh_ob_get_length },
  { "ob_get_level", fg_ob_get_level, (void *)&fh_ob_get_level },
  { "ob_get_status", fg_ob_get_status, (void *)&fh_ob_get_status },
  { "ob_gzhandler", fg_ob_gzhandler, (void *)&fh_ob_gzhandler },
  { "ob_implicit_flush", fg_ob_implicit_flush, (void *)&fh_ob_implicit_flush },
  { "ob_list_handlers", fg_ob_list_handlers, (void *)&fh_ob_list_handlers },
  { "output_add_rewrite_var", fg_output_add_rewrite_var, (void *)&fh_output_add_rewrite_var },
  { "output_reset_rewrite_vars", fg_output_reset_rewrite_vars, (void *)&fh_output_reset_rewrite_vars },
  { "hphp_crash_log", fg_hphp_crash_log, (void *)&fh_hphp_crash_log },
  { "hphp_stats", fg_hphp_stats, (void *)&fh_hphp_stats },
  { "hphp_get_stats", fg_hphp_get_stats, (void *)&fh_hphp_get_stats },
  { "hphp_get_status", fg_hphp_get_status, (void *)&fh_hphp_get_status },
  { "hphp_get_iostatus", fg_hphp_get_iostatus, (void *)&fh_hphp_get_iostatus },
  { "hphp_set_iostatus_address", fg_hphp_set_iostatus_address, (void *)&fh_hphp_set_iostatus_address },
  { "hphp_get_timers", fg_hphp_get_timers, (void *)&fh_hphp_get_timers },
  { "hphp_output_global_state", fg_hphp_output_global_state, (void *)&fh_hphp_output_global_state },
  { "hphp_instruction_counter", fg_hphp_instruction_counter, (void *)&fh_hphp_instruction_counter },
  { "hphp_get_hardware_counters", fg_hphp_get_hardware_counters, (void *)&fh_hphp_get_hardware_counters },
  { "hphp_set_hardware_events", fg_hphp_set_hardware_events, (void *)&fh_hphp_set_hardware_events },
  { "hphp_clear_hardware_events", fg_hphp_clear_hardware_events, (void *)&fh_hphp_clear_hardware_events },
  { "pdo_drivers", fg_pdo_drivers, (void *)&fh_pdo_drivers },
  { "posix_access", fg_posix_access, (void *)&fh_posix_access },
  { "posix_ctermid", fg_posix_ctermid, (void *)&fh_posix_ctermid },
  { "posix_get_last_error", fg_posix_get_last_error, (void *)&fh_posix_get_last_error },
  { "posix_getcwd", fg_posix_getcwd, (void *)&fh_posix_getcwd },
  { "posix_getegid", fg_posix_getegid, (void *)&fh_posix_getegid },
  { "posix_geteuid", fg_posix_geteuid, (void *)&fh_posix_geteuid },
  { "posix_getgid", fg_posix_getgid, (void *)&fh_posix_getgid },
  { "posix_getgrgid", fg_posix_getgrgid, (void *)&fh_posix_getgrgid },
  { "posix_getgrnam", fg_posix_getgrnam, (void *)&fh_posix_getgrnam },
  { "posix_getgroups", fg_posix_getgroups, (void *)&fh_posix_getgroups },
  { "posix_getlogin", fg_posix_getlogin, (void *)&fh_posix_getlogin },
  { "posix_getpgid", fg_posix_getpgid, (void *)&fh_posix_getpgid },
  { "posix_getpgrp", fg_posix_getpgrp, (void *)&fh_posix_getpgrp },
  { "posix_getpid", fg_posix_getpid, (void *)&fh_posix_getpid },
  { "posix_getppid", fg_posix_getppid, (void *)&fh_posix_getppid },
  { "posix_getpwnam", fg_posix_getpwnam, (void *)&fh_posix_getpwnam },
  { "posix_getpwuid", fg_posix_getpwuid, (void *)&fh_posix_getpwuid },
  { "posix_getrlimit", fg_posix_getrlimit, (void *)&fh_posix_getrlimit },
  { "posix_getsid", fg_posix_getsid, (void *)&fh_posix_getsid },
  { "posix_getuid", fg_posix_getuid, (void *)&fh_posix_getuid },
  { "posix_initgroups", fg_posix_initgroups, (void *)&fh_posix_initgroups },
  { "posix_isatty", fg_posix_isatty, (void *)&fh_posix_isatty },
  { "posix_kill", fg_posix_kill, (void *)&fh_posix_kill },
  { "posix_mkfifo", fg_posix_mkfifo, (void *)&fh_posix_mkfifo },
  { "posix_mknod", fg_posix_mknod, (void *)&fh_posix_mknod },
  { "posix_setegid", fg_posix_setegid, (void *)&fh_posix_setegid },
  { "posix_seteuid", fg_posix_seteuid, (void *)&fh_posix_seteuid },
  { "posix_setgid", fg_posix_setgid, (void *)&fh_posix_setgid },
  { "posix_setpgid", fg_posix_setpgid, (void *)&fh_posix_setpgid },
  { "posix_setsid", fg_posix_setsid, (void *)&fh_posix_setsid },
  { "posix_setuid", fg_posix_setuid, (void *)&fh_posix_setuid },
  { "posix_strerror", fg_posix_strerror, (void *)&fh_posix_strerror },
  { "posix_times", fg_posix_times, (void *)&fh_posix_times },
  { "posix_ttyname", fg_posix_ttyname, (void *)&fh_posix_ttyname },
  { "posix_uname", fg_posix_uname, (void *)&fh_posix_uname },
  { "preg_grep", fg_preg_grep, (void *)&fh_preg_grep },
  { "preg_match", fg_preg_match, (void *)&fh_preg_match },
  { "preg_match_all", fg_preg_match_all, (void *)&fh_preg_match_all },
  { "preg_replace", fg_preg_replace, (void *)&fh_preg_replace },
  { "preg_replace_callback", fg_preg_replace_callback, (void *)&fh_preg_replace_callback },
  { "preg_split", fg_preg_split, (void *)&fh_preg_split },
  { "preg_quote", fg_preg_quote, (void *)&fh_preg_quote },
  { "preg_last_error", fg_preg_last_error, (void *)&fh_preg_last_error },
  { "ereg_replace", fg_ereg_replace, (void *)&fh_ereg_replace },
  { "eregi_replace", fg_eregi_replace, (void *)&fh_eregi_replace },
  { "ereg", fg_ereg, (void *)&fh_ereg },
  { "eregi", fg_eregi, (void *)&fh_eregi },
  { "split", fg_split, (void *)&fh_split },
  { "spliti", fg_spliti, (void *)&fh_spliti },
  { "sql_regcase", fg_sql_regcase, (void *)&fh_sql_regcase },
  { "pcntl_alarm", fg_pcntl_alarm, (void *)&fh_pcntl_alarm },
  { "pcntl_exec", fg_pcntl_exec, (void *)&fh_pcntl_exec },
  { "pcntl_fork", fg_pcntl_fork, (void *)&fh_pcntl_fork },
  { "pcntl_getpriority", fg_pcntl_getpriority, (void *)&fh_pcntl_getpriority },
  { "pcntl_setpriority", fg_pcntl_setpriority, (void *)&fh_pcntl_setpriority },
  { "pcntl_signal", fg_pcntl_signal, (void *)&fh_pcntl_signal },
  { "pcntl_wait", fg_pcntl_wait, (void *)&fh_pcntl_wait },
  { "pcntl_waitpid", fg_pcntl_waitpid, (void *)&fh_pcntl_waitpid },
  { "pcntl_wexitstatus", fg_pcntl_wexitstatus, (void *)&fh_pcntl_wexitstatus },
  { "pcntl_wifexited", fg_pcntl_wifexited, (void *)&fh_pcntl_wifexited },
  { "pcntl_wifsignaled", fg_pcntl_wifsignaled, (void *)&fh_pcntl_wifsignaled },
  { "pcntl_wifstopped", fg_pcntl_wifstopped, (void *)&fh_pcntl_wifstopped },
  { "pcntl_wstopsig", fg_pcntl_wstopsig, (void *)&fh_pcntl_wstopsig },
  { "pcntl_wtermsig", fg_pcntl_wtermsig, (void *)&fh_pcntl_wtermsig },
  { "pcntl_signal_dispatch", fg_pcntl_signal_dispatch, (void *)&fh_pcntl_signal_dispatch },
  { "shell_exec", fg_shell_exec, (void *)&fh_shell_exec },
  { "exec", fg_exec, (void *)&fh_exec },
  { "passthru", fg_passthru, (void *)&fh_passthru },
  { "system", fg_system, (void *)&fh_system },
  { "proc_open", fg_proc_open, (void *)&fh_proc_open },
  { "proc_terminate", fg_proc_terminate, (void *)&fh_proc_terminate },
  { "proc_close", fg_proc_close, (void *)&fh_proc_close },
  { "proc_get_status", fg_proc_get_status, (void *)&fh_proc_get_status },
  { "proc_nice", fg_proc_nice, (void *)&fh_proc_nice },
  { "escapeshellarg", fg_escapeshellarg, (void *)&fh_escapeshellarg },
  { "escapeshellcmd", fg_escapeshellcmd, (void *)&fh_escapeshellcmd },
  { "hphp_get_extension_info", fg_hphp_get_extension_info, (void *)&fh_hphp_get_extension_info },
  { "hphp_get_method_info", fg_hphp_get_method_info, (void *)&fh_hphp_get_method_info },
  { "hphp_get_closure_info", fg_hphp_get_closure_info, (void *)&fh_hphp_get_closure_info },
  { "hphp_get_class_constant", fg_hphp_get_class_constant, (void *)&fh_hphp_get_class_constant },
  { "hphp_get_class_info", fg_hphp_get_class_info, (void *)&fh_hphp_get_class_info },
  { "hphp_get_function_info", fg_hphp_get_function_info, (void *)&fh_hphp_get_function_info },
  { "hphp_invoke", fg_hphp_invoke, (void *)&fh_hphp_invoke },
  { "hphp_invoke_method", fg_hphp_invoke_method, (void *)&fh_hphp_invoke_method },
  { "hphp_instanceof", fg_hphp_instanceof, (void *)&fh_hphp_instanceof },
  { "hphp_create_object", fg_hphp_create_object, (void *)&fh_hphp_create_object },
  { "hphp_create_object_without_constructor", fg_hphp_create_object_without_constructor, (void *)&fh_hphp_create_object_without_constructor },
  { "hphp_get_property", fg_hphp_get_property, (void *)&fh_hphp_get_property },
  { "hphp_set_property", fg_hphp_set_property, (void *)&fh_hphp_set_property },
  { "hphp_get_static_property", fg_hphp_get_static_property, (void *)&fh_hphp_get_static_property },
  { "hphp_set_static_property", fg_hphp_set_static_property, (void *)&fh_hphp_set_static_property },
  { "hphp_get_original_class_name", fg_hphp_get_original_class_name, (void *)&fh_hphp_get_original_class_name },
  { "hphp_scalar_typehints_enabled", fg_hphp_scalar_typehints_enabled, (void *)&fh_hphp_scalar_typehints_enabled },
  { "dangling_server_proxy_old_request", fg_dangling_server_proxy_old_request, (void *)&fh_dangling_server_proxy_old_request },
  { "dangling_server_proxy_new_request", fg_dangling_server_proxy_new_request, (void *)&fh_dangling_server_proxy_new_request },
  { "pagelet_server_is_enabled", fg_pagelet_server_is_enabled, (void *)&fh_pagelet_server_is_enabled },
  { "pagelet_server_task_start", fg_pagelet_server_task_start, (void *)&fh_pagelet_server_task_start },
  { "pagelet_server_task_status", fg_pagelet_server_task_status, (void *)&fh_pagelet_server_task_status },
  { "pagelet_server_task_result", fg_pagelet_server_task_result, (void *)&fh_pagelet_server_task_result },
  { "pagelet_server_flush", fg_pagelet_server_flush, (void *)&fh_pagelet_server_flush },
  { "xbox_send_message", fg_xbox_send_message, (void *)&fh_xbox_send_message },
  { "xbox_post_message", fg_xbox_post_message, (void *)&fh_xbox_post_message },
  { "xbox_task_start", fg_xbox_task_start, (void *)&fh_xbox_task_start },
  { "xbox_task_status", fg_xbox_task_status, (void *)&fh_xbox_task_status },
  { "xbox_task_result", fg_xbox_task_result, (void *)&fh_xbox_task_result },
  { "xbox_process_call_message", fg_xbox_process_call_message, (void *)&fh_xbox_process_call_message },
  { "xbox_get_thread_timeout", fg_xbox_get_thread_timeout, (void *)&fh_xbox_get_thread_timeout },
  { "xbox_set_thread_timeout", fg_xbox_set_thread_timeout, (void *)&fh_xbox_set_thread_timeout },
  { "xbox_schedule_thread_reset", fg_xbox_schedule_thread_reset, (void *)&fh_xbox_schedule_thread_reset },
  { "xbox_get_thread_time", fg_xbox_get_thread_time, (void *)&fh_xbox_get_thread_time },
  { "session_set_cookie_params", fg_session_set_cookie_params, (void *)&fh_session_set_cookie_params },
  { "session_get_cookie_params", fg_session_get_cookie_params, (void *)&fh_session_get_cookie_params },
  { "session_name", fg_session_name, (void *)&fh_session_name },
  { "session_module_name", fg_session_module_name, (void *)&fh_session_module_name },
  { "session_set_save_handler", fg_session_set_save_handler, (void *)&fh_session_set_save_handler },
  { "session_save_path", fg_session_save_path, (void *)&fh_session_save_path },
  { "session_id", fg_session_id, (void *)&fh_session_id },
  { "session_regenerate_id", fg_session_regenerate_id, (void *)&fh_session_regenerate_id },
  { "session_cache_limiter", fg_session_cache_limiter, (void *)&fh_session_cache_limiter },
  { "session_cache_expire", fg_session_cache_expire, (void *)&fh_session_cache_expire },
  { "session_encode", fg_session_encode, (void *)&fh_session_encode },
  { "session_decode", fg_session_decode, (void *)&fh_session_decode },
  { "session_start", fg_session_start, (void *)&fh_session_start },
  { "session_destroy", fg_session_destroy, (void *)&fh_session_destroy },
  { "session_unset", fg_session_unset, (void *)&fh_session_unset },
  { "session_commit", fg_session_commit, (void *)&fh_session_commit },
  { "session_write_close", fg_session_write_close, (void *)&fh_session_write_close },
  { "session_register", fg_session_register, (void *)&fh_session_register },
  { "session_unregister", fg_session_unregister, (void *)&fh_session_unregister },
  { "session_is_registered", fg_session_is_registered, (void *)&fh_session_is_registered },
  { "simplexml_load_string", fg_simplexml_load_string, (void *)&fh_simplexml_load_string },
  { "simplexml_load_file", fg_simplexml_load_file, (void *)&fh_simplexml_load_file },
  { "libxml_get_errors", fg_libxml_get_errors, (void *)&fh_libxml_get_errors },
  { "libxml_get_last_error", fg_libxml_get_last_error, (void *)&fh_libxml_get_last_error },
  { "libxml_clear_errors", fg_libxml_clear_errors, (void *)&fh_libxml_clear_errors },
  { "libxml_use_internal_errors", fg_libxml_use_internal_errors, (void *)&fh_libxml_use_internal_errors },
  { "libxml_set_streams_context", fg_libxml_set_streams_context, (void *)&fh_libxml_set_streams_context },
  { "libxml_disable_entity_loader", fg_libxml_disable_entity_loader, (void *)&fh_libxml_disable_entity_loader },
  { "use_soap_error_handler", fg_use_soap_error_handler, (void *)&fh_use_soap_error_handler },
  { "is_soap_fault", fg_is_soap_fault, (void *)&fh_is_soap_fault },
  { "_soap_active_version", fg__soap_active_version, (void *)&fh__soap_active_version },
  { "socket_create", fg_socket_create, (void *)&fh_socket_create },
  { "socket_create_listen", fg_socket_create_listen, (void *)&fh_socket_create_listen },
  { "socket_create_pair", fg_socket_create_pair, (void *)&fh_socket_create_pair },
  { "socket_get_option", fg_socket_get_option, (void *)&fh_socket_get_option },
  { "socket_getpeername", fg_socket_getpeername, (void *)&fh_socket_getpeername },
  { "socket_getsockname", fg_socket_getsockname, (void *)&fh_socket_getsockname },
  { "socket_set_block", fg_socket_set_block, (void *)&fh_socket_set_block },
  { "socket_set_nonblock", fg_socket_set_nonblock, (void *)&fh_socket_set_nonblock },
  { "socket_set_option", fg_socket_set_option, (void *)&fh_socket_set_option },
  { "socket_connect", fg_socket_connect, (void *)&fh_socket_connect },
  { "socket_bind", fg_socket_bind, (void *)&fh_socket_bind },
  { "socket_listen", fg_socket_listen, (void *)&fh_socket_listen },
  { "socket_select", fg_socket_select, (void *)&fh_socket_select },
  { "socket_server", fg_socket_server, (void *)&fh_socket_server },
  { "socket_accept", fg_socket_accept, (void *)&fh_socket_accept },
  { "socket_read", fg_socket_read, (void *)&fh_socket_read },
  { "socket_write", fg_socket_write, (void *)&fh_socket_write },
  { "socket_send", fg_socket_send, (void *)&fh_socket_send },
  { "socket_sendto", fg_socket_sendto, (void *)&fh_socket_sendto },
  { "socket_recv", fg_socket_recv, (void *)&fh_socket_recv },
  { "socket_recvfrom", fg_socket_recvfrom, (void *)&fh_socket_recvfrom },
  { "socket_shutdown", fg_socket_shutdown, (void *)&fh_socket_shutdown },
  { "socket_close", fg_socket_close, (void *)&fh_socket_close },
  { "socket_strerror", fg_socket_strerror, (void *)&fh_socket_strerror },
  { "socket_last_error", fg_socket_last_error, (void *)&fh_socket_last_error },
  { "socket_clear_error", fg_socket_clear_error, (void *)&fh_socket_clear_error },
  { "getaddrinfo", fg_getaddrinfo, (void *)&fh_getaddrinfo },
  { "spl_classes", fg_spl_classes, (void *)&fh_spl_classes },
  { "spl_object_hash", fg_spl_object_hash, (void *)&fh_spl_object_hash },
  { "hphp_object_pointer", fg_hphp_object_pointer, (void *)&fh_hphp_object_pointer },
  { "hphp_get_this", fg_hphp_get_this, (void *)&fh_hphp_get_this },
  { "class_implements", fg_class_implements, (void *)&fh_class_implements },
  { "class_parents", fg_class_parents, (void *)&fh_class_parents },
  { "class_uses", fg_class_uses, (void *)&fh_class_uses },
  { "iterator_apply", fg_iterator_apply, (void *)&fh_iterator_apply },
  { "iterator_count", fg_iterator_count, (void *)&fh_iterator_count },
  { "iterator_to_array", fg_iterator_to_array, (void *)&fh_iterator_to_array },
  { "spl_autoload_call", fg_spl_autoload_call, (void *)&fh_spl_autoload_call },
  { "spl_autoload_extensions", fg_spl_autoload_extensions, (void *)&fh_spl_autoload_extensions },
  { "spl_autoload_functions", fg_spl_autoload_functions, (void *)&fh_spl_autoload_functions },
  { "spl_autoload_register", fg_spl_autoload_register, (void *)&fh_spl_autoload_register },
  { "spl_autoload_unregister", fg_spl_autoload_unregister, (void *)&fh_spl_autoload_unregister },
  { "spl_autoload", fg_spl_autoload, (void *)&fh_spl_autoload },
  { "hphp_splfileinfo___construct", fg_hphp_splfileinfo___construct, (void *)&fh_hphp_splfileinfo___construct },
  { "hphp_splfileinfo_getatime", fg_hphp_splfileinfo_getatime, (void *)&fh_hphp_splfileinfo_getatime },
  { "hphp_splfileinfo_getbasename", fg_hphp_splfileinfo_getbasename, (void *)&fh_hphp_splfileinfo_getbasename },
  { "hphp_splfileinfo_getctime", fg_hphp_splfileinfo_getctime, (void *)&fh_hphp_splfileinfo_getctime },
  { "hphp_splfileinfo_getfileinfo", fg_hphp_splfileinfo_getfileinfo, (void *)&fh_hphp_splfileinfo_getfileinfo },
  { "hphp_splfileinfo_getfilename", fg_hphp_splfileinfo_getfilename, (void *)&fh_hphp_splfileinfo_getfilename },
  { "hphp_splfileinfo_getgroup", fg_hphp_splfileinfo_getgroup, (void *)&fh_hphp_splfileinfo_getgroup },
  { "hphp_splfileinfo_getinode", fg_hphp_splfileinfo_getinode, (void *)&fh_hphp_splfileinfo_getinode },
  { "hphp_splfileinfo_getlinktarget", fg_hphp_splfileinfo_getlinktarget, (void *)&fh_hphp_splfileinfo_getlinktarget },
  { "hphp_splfileinfo_getmtime", fg_hphp_splfileinfo_getmtime, (void *)&fh_hphp_splfileinfo_getmtime },
  { "hphp_splfileinfo_getowner", fg_hphp_splfileinfo_getowner, (void *)&fh_hphp_splfileinfo_getowner },
  { "hphp_splfileinfo_getpath", fg_hphp_splfileinfo_getpath, (void *)&fh_hphp_splfileinfo_getpath },
  { "hphp_splfileinfo_getpathinfo", fg_hphp_splfileinfo_getpathinfo, (void *)&fh_hphp_splfileinfo_getpathinfo },
  { "hphp_splfileinfo_getpathname", fg_hphp_splfileinfo_getpathname, (void *)&fh_hphp_splfileinfo_getpathname },
  { "hphp_splfileinfo_getperms", fg_hphp_splfileinfo_getperms, (void *)&fh_hphp_splfileinfo_getperms },
  { "hphp_splfileinfo_getrealpath", fg_hphp_splfileinfo_getrealpath, (void *)&fh_hphp_splfileinfo_getrealpath },
  { "hphp_splfileinfo_getsize", fg_hphp_splfileinfo_getsize, (void *)&fh_hphp_splfileinfo_getsize },
  { "hphp_splfileinfo_gettype", fg_hphp_splfileinfo_gettype, (void *)&fh_hphp_splfileinfo_gettype },
  { "hphp_splfileinfo_isdir", fg_hphp_splfileinfo_isdir, (void *)&fh_hphp_splfileinfo_isdir },
  { "hphp_splfileinfo_isexecutable", fg_hphp_splfileinfo_isexecutable, (void *)&fh_hphp_splfileinfo_isexecutable },
  { "hphp_splfileinfo_isfile", fg_hphp_splfileinfo_isfile, (void *)&fh_hphp_splfileinfo_isfile },
  { "hphp_splfileinfo_islink", fg_hphp_splfileinfo_islink, (void *)&fh_hphp_splfileinfo_islink },
  { "hphp_splfileinfo_isreadable", fg_hphp_splfileinfo_isreadable, (void *)&fh_hphp_splfileinfo_isreadable },
  { "hphp_splfileinfo_iswritable", fg_hphp_splfileinfo_iswritable, (void *)&fh_hphp_splfileinfo_iswritable },
  { "hphp_splfileinfo_openfile", fg_hphp_splfileinfo_openfile, (void *)&fh_hphp_splfileinfo_openfile },
  { "hphp_splfileinfo_setfileclass", fg_hphp_splfileinfo_setfileclass, (void *)&fh_hphp_splfileinfo_setfileclass },
  { "hphp_splfileinfo_setinfoclass", fg_hphp_splfileinfo_setinfoclass, (void *)&fh_hphp_splfileinfo_setinfoclass },
  { "hphp_splfileinfo___tostring", fg_hphp_splfileinfo___tostring, (void *)&fh_hphp_splfileinfo___tostring },
  { "hphp_splfileobject___construct", fg_hphp_splfileobject___construct, (void *)&fh_hphp_splfileobject___construct },
  { "hphp_splfileobject_current", fg_hphp_splfileobject_current, (void *)&fh_hphp_splfileobject_current },
  { "hphp_splfileobject_eof", fg_hphp_splfileobject_eof, (void *)&fh_hphp_splfileobject_eof },
  { "hphp_splfileobject_fflush", fg_hphp_splfileobject_fflush, (void *)&fh_hphp_splfileobject_fflush },
  { "hphp_splfileobject_fgetc", fg_hphp_splfileobject_fgetc, (void *)&fh_hphp_splfileobject_fgetc },
  { "hphp_splfileobject_fgetcsv", fg_hphp_splfileobject_fgetcsv, (void *)&fh_hphp_splfileobject_fgetcsv },
  { "hphp_splfileobject_fgets", fg_hphp_splfileobject_fgets, (void *)&fh_hphp_splfileobject_fgets },
  { "hphp_splfileobject_fgetss", fg_hphp_splfileobject_fgetss, (void *)&fh_hphp_splfileobject_fgetss },
  { "hphp_splfileobject_flock", fg_hphp_splfileobject_flock, (void *)&fh_hphp_splfileobject_flock },
  { "hphp_splfileobject_fpassthru", fg_hphp_splfileobject_fpassthru, (void *)&fh_hphp_splfileobject_fpassthru },
  { "hphp_splfileobject_fscanf", fg_hphp_splfileobject_fscanf, (void *)&fh_hphp_splfileobject_fscanf },
  { "hphp_splfileobject_fseek", fg_hphp_splfileobject_fseek, (void *)&fh_hphp_splfileobject_fseek },
  { "hphp_splfileobject_fstat", fg_hphp_splfileobject_fstat, (void *)&fh_hphp_splfileobject_fstat },
  { "hphp_splfileobject_ftell", fg_hphp_splfileobject_ftell, (void *)&fh_hphp_splfileobject_ftell },
  { "hphp_splfileobject_ftruncate", fg_hphp_splfileobject_ftruncate, (void *)&fh_hphp_splfileobject_ftruncate },
  { "hphp_splfileobject_fwrite", fg_hphp_splfileobject_fwrite, (void *)&fh_hphp_splfileobject_fwrite },
  { "hphp_splfileobject_getcvscontrol", fg_hphp_splfileobject_getcvscontrol, (void *)&fh_hphp_splfileobject_getcvscontrol },
  { "hphp_splfileobject_getflags", fg_hphp_splfileobject_getflags, (void *)&fh_hphp_splfileobject_getflags },
  { "hphp_splfileobject_getmaxlinelen", fg_hphp_splfileobject_getmaxlinelen, (void *)&fh_hphp_splfileobject_getmaxlinelen },
  { "hphp_splfileobject_key", fg_hphp_splfileobject_key, (void *)&fh_hphp_splfileobject_key },
  { "hphp_splfileobject_next", fg_hphp_splfileobject_next, (void *)&fh_hphp_splfileobject_next },
  { "hphp_splfileobject_rewind", fg_hphp_splfileobject_rewind, (void *)&fh_hphp_splfileobject_rewind },
  { "hphp_splfileobject_valid", fg_hphp_splfileobject_valid, (void *)&fh_hphp_splfileobject_valid },
  { "hphp_splfileobject_seek", fg_hphp_splfileobject_seek, (void *)&fh_hphp_splfileobject_seek },
  { "hphp_splfileobject_setcsvcontrol", fg_hphp_splfileobject_setcsvcontrol, (void *)&fh_hphp_splfileobject_setcsvcontrol },
  { "hphp_splfileobject_setflags", fg_hphp_splfileobject_setflags, (void *)&fh_hphp_splfileobject_setflags },
  { "hphp_splfileobject_setmaxlinelen", fg_hphp_splfileobject_setmaxlinelen, (void *)&fh_hphp_splfileobject_setmaxlinelen },
  { "stream_context_create", fg_stream_context_create, (void *)&fh_stream_context_create },
  { "stream_context_get_default", fg_stream_context_get_default, (void *)&fh_stream_context_get_default },
  { "stream_context_get_options", fg_stream_context_get_options, (void *)&fh_stream_context_get_options },
  { "stream_context_set_option", fg_stream_context_set_option, (void *)&fh_stream_context_set_option },
  { "stream_context_set_param", fg_stream_context_set_param, (void *)&fh_stream_context_set_param },
  { "stream_copy_to_stream", fg_stream_copy_to_stream, (void *)&fh_stream_copy_to_stream },
  { "stream_encoding", fg_stream_encoding, (void *)&fh_stream_encoding },
  { "stream_bucket_append", fg_stream_bucket_append, (void *)&fh_stream_bucket_append },
  { "stream_bucket_prepend", fg_stream_bucket_prepend, (void *)&fh_stream_bucket_prepend },
  { "stream_bucket_make_writeable", fg_stream_bucket_make_writeable, (void *)&fh_stream_bucket_make_writeable },
  { "stream_bucket_new", fg_stream_bucket_new, (void *)&fh_stream_bucket_new },
  { "stream_filter_register", fg_stream_filter_register, (void *)&fh_stream_filter_register },
  { "stream_filter_remove", fg_stream_filter_remove, (void *)&fh_stream_filter_remove },
  { "stream_filter_append", fg_stream_filter_append, (void *)&fh_stream_filter_append },
  { "stream_filter_prepend", fg_stream_filter_prepend, (void *)&fh_stream_filter_prepend },
  { "stream_get_contents", fg_stream_get_contents, (void *)&fh_stream_get_contents },
  { "stream_get_filters", fg_stream_get_filters, (void *)&fh_stream_get_filters },
  { "stream_get_line", fg_stream_get_line, (void *)&fh_stream_get_line },
  { "stream_get_meta_data", fg_stream_get_meta_data, (void *)&fh_stream_get_meta_data },
  { "stream_get_transports", fg_stream_get_transports, (void *)&fh_stream_get_transports },
  { "stream_get_wrappers", fg_stream_get_wrappers, (void *)&fh_stream_get_wrappers },
  { "stream_register_wrapper", fg_stream_register_wrapper, (void *)&fh_stream_register_wrapper },
  { "stream_wrapper_register", fg_stream_wrapper_register, (void *)&fh_stream_wrapper_register },
  { "stream_wrapper_restore", fg_stream_wrapper_restore, (void *)&fh_stream_wrapper_restore },
  { "stream_wrapper_unregister", fg_stream_wrapper_unregister, (void *)&fh_stream_wrapper_unregister },
  { "stream_resolve_include_path", fg_stream_resolve_include_path, (void *)&fh_stream_resolve_include_path },
  { "stream_select", fg_stream_select, (void *)&fh_stream_select },
  { "stream_set_blocking", fg_stream_set_blocking, (void *)&fh_stream_set_blocking },
  { "stream_set_timeout", fg_stream_set_timeout, (void *)&fh_stream_set_timeout },
  { "stream_set_write_buffer", fg_stream_set_write_buffer, (void *)&fh_stream_set_write_buffer },
  { "set_file_buffer", fg_set_file_buffer, (void *)&fh_set_file_buffer },
  { "stream_socket_accept", fg_stream_socket_accept, (void *)&fh_stream_socket_accept },
  { "stream_socket_server", fg_stream_socket_server, (void *)&fh_stream_socket_server },
  { "stream_socket_client", fg_stream_socket_client, (void *)&fh_stream_socket_client },
  { "stream_socket_enable_crypto", fg_stream_socket_enable_crypto, (void *)&fh_stream_socket_enable_crypto },
  { "stream_socket_get_name", fg_stream_socket_get_name, (void *)&fh_stream_socket_get_name },
  { "stream_socket_pair", fg_stream_socket_pair, (void *)&fh_stream_socket_pair },
  { "stream_socket_recvfrom", fg_stream_socket_recvfrom, (void *)&fh_stream_socket_recvfrom },
  { "stream_socket_sendto", fg_stream_socket_sendto, (void *)&fh_stream_socket_sendto },
  { "stream_socket_shutdown", fg_stream_socket_shutdown, (void *)&fh_stream_socket_shutdown },
  { "addcslashes", fg_addcslashes, (void *)&fh_addcslashes },
  { "stripcslashes", fg_stripcslashes, (void *)&fh_stripcslashes },
  { "addslashes", fg_addslashes, (void *)&fh_addslashes },
  { "stripslashes", fg_stripslashes, (void *)&fh_stripslashes },
  { "bin2hex", fg_bin2hex, (void *)&fh_bin2hex },
  { "hex2bin", fg_hex2bin, (void *)&fh_hex2bin },
  { "nl2br", fg_nl2br, (void *)&fh_nl2br },
  { "quotemeta", fg_quotemeta, (void *)&fh_quotemeta },
  { "str_shuffle", fg_str_shuffle, (void *)&fh_str_shuffle },
  { "strrev", fg_strrev, (void *)&fh_strrev },
  { "strtolower", fg_strtolower, (void *)&fh_strtolower },
  { "strtoupper", fg_strtoupper, (void *)&fh_strtoupper },
  { "ucfirst", fg_ucfirst, (void *)&fh_ucfirst },
  { "lcfirst", fg_lcfirst, (void *)&fh_lcfirst },
  { "ucwords", fg_ucwords, (void *)&fh_ucwords },
  { "strip_tags", fg_strip_tags, (void *)&fh_strip_tags },
  { "trim", fg_trim, (void *)&fh_trim },
  { "ltrim", fg_ltrim, (void *)&fh_ltrim },
  { "rtrim", fg_rtrim, (void *)&fh_rtrim },
  { "chop", fg_chop, (void *)&fh_chop },
  { "explode", fg_explode, (void *)&fh_explode },
  { "implode", fg_implode, (void *)&fh_implode },
  { "join", fg_join, (void *)&fh_join },
  { "str_split", fg_str_split, (void *)&fh_str_split },
  { "chunk_split", fg_chunk_split, (void *)&fh_chunk_split },
  { "strtok", fg_strtok, (void *)&fh_strtok },
  { "str_replace", fg_str_replace, (void *)&fh_str_replace },
  { "str_ireplace", fg_str_ireplace, (void *)&fh_str_ireplace },
  { "substr_replace", fg_substr_replace, (void *)&fh_substr_replace },
  { "substr", fg_substr, (void *)&fh_substr },
  { "str_pad", fg_str_pad, (void *)&fh_str_pad },
  { "str_repeat", fg_str_repeat, (void *)&fh_str_repeat },
  { "wordwrap", fg_wordwrap, (void *)&fh_wordwrap },
  { "html_entity_decode", fg_html_entity_decode, (void *)&fh_html_entity_decode },
  { "htmlentities", fg_htmlentities, (void *)&fh_htmlentities },
  { "htmlspecialchars_decode", fg_htmlspecialchars_decode, (void *)&fh_htmlspecialchars_decode },
  { "htmlspecialchars", fg_htmlspecialchars, (void *)&fh_htmlspecialchars },
  { "fb_htmlspecialchars", fg_fb_htmlspecialchars, (void *)&fh_fb_htmlspecialchars },
  { "quoted_printable_encode", fg_quoted_printable_encode, (void *)&fh_quoted_printable_encode },
  { "quoted_printable_decode", fg_quoted_printable_decode, (void *)&fh_quoted_printable_decode },
  { "convert_uudecode", fg_convert_uudecode, (void *)&fh_convert_uudecode },
  { "convert_uuencode", fg_convert_uuencode, (void *)&fh_convert_uuencode },
  { "str_rot13", fg_str_rot13, (void *)&fh_str_rot13 },
  { "crc32", fg_crc32, (void *)&fh_crc32 },
  { "crypt", fg_crypt, (void *)&fh_crypt },
  { "md5", fg_md5, (void *)&fh_md5 },
  { "sha1", fg_sha1, (void *)&fh_sha1 },
  { "strtr", fg_strtr, (void *)&fh_strtr },
  { "convert_cyr_string", fg_convert_cyr_string, (void *)&fh_convert_cyr_string },
  { "get_html_translation_table", fg_get_html_translation_table, (void *)&fh_get_html_translation_table },
  { "hebrev", fg_hebrev, (void *)&fh_hebrev },
  { "hebrevc", fg_hebrevc, (void *)&fh_hebrevc },
  { "setlocale", fg_setlocale, (void *)&fh_setlocale },
  { "localeconv", fg_localeconv, (void *)&fh_localeconv },
  { "nl_langinfo", fg_nl_langinfo, (void *)&fh_nl_langinfo },
  { "printf", fg_printf, (void *)&fh_printf },
  { "vprintf", fg_vprintf, (void *)&fh_vprintf },
  { "sprintf", fg_sprintf, (void *)&fh_sprintf },
  { "vsprintf", fg_vsprintf, (void *)&fh_vsprintf },
  { "sscanf", fg_sscanf, (void *)&fh_sscanf },
  { "chr", fg_chr, (void *)&fh_chr },
  { "ord", fg_ord, (void *)&fh_ord },
  { "money_format", fg_money_format, (void *)&fh_money_format },
  { "number_format", fg_number_format, (void *)&fh_number_format },
  { "strcmp", fg_strcmp, (void *)&fh_strcmp },
  { "strncmp", fg_strncmp, (void *)&fh_strncmp },
  { "strnatcmp", fg_strnatcmp, (void *)&fh_strnatcmp },
  { "strcasecmp", fg_strcasecmp, (void *)&fh_strcasecmp },
  { "strncasecmp", fg_strncasecmp, (void *)&fh_strncasecmp },
  { "strnatcasecmp", fg_strnatcasecmp, (void *)&fh_strnatcasecmp },
  { "strcoll", fg_strcoll, (void *)&fh_strcoll },
  { "substr_compare", fg_substr_compare, (void *)&fh_substr_compare },
  { "strchr", fg_strchr, (void *)&fh_strchr },
  { "strrchr", fg_strrchr, (void *)&fh_strrchr },
  { "strstr", fg_strstr, (void *)&fh_strstr },
  { "stristr", fg_stristr, (void *)&fh_stristr },
  { "strpbrk", fg_strpbrk, (void *)&fh_strpbrk },
  { "strpos", fg_strpos, (void *)&fh_strpos },
  { "stripos", fg_stripos, (void *)&fh_stripos },
  { "strrpos", fg_strrpos, (void *)&fh_strrpos },
  { "strripos", fg_strripos, (void *)&fh_strripos },
  { "substr_count", fg_substr_count, (void *)&fh_substr_count },
  { "strspn", fg_strspn, (void *)&fh_strspn },
  { "strcspn", fg_strcspn, (void *)&fh_strcspn },
  { "strlen", fg_strlen, (void *)&fh_strlen },
  { "count_chars", fg_count_chars, (void *)&fh_count_chars },
  { "str_word_count", fg_str_word_count, (void *)&fh_str_word_count },
  { "levenshtein", fg_levenshtein, (void *)&fh_levenshtein },
  { "similar_text", fg_similar_text, (void *)&fh_similar_text },
  { "soundex", fg_soundex, (void *)&fh_soundex },
  { "metaphone", fg_metaphone, (void *)&fh_metaphone },
  { "parse_str", fg_parse_str, (void *)&fh_parse_str },
  { "hphp_is_service_thread", fg_hphp_is_service_thread, (void *)&fh_hphp_is_service_thread },
  { "hphp_service_thread_started", fg_hphp_service_thread_started, (void *)&fh_hphp_service_thread_started },
  { "hphp_service_thread_stopped", fg_hphp_service_thread_stopped, (void *)&fh_hphp_service_thread_stopped },
  { "hphp_thread_is_warmup_enabled", fg_hphp_thread_is_warmup_enabled, (void *)&fh_hphp_thread_is_warmup_enabled },
  { "hphp_thread_set_warmup_enabled", fg_hphp_thread_set_warmup_enabled, (void *)&fh_hphp_thread_set_warmup_enabled },
  { "hphp_get_thread_id", fg_hphp_get_thread_id, (void *)&fh_hphp_get_thread_id },
  { "hphp_gettid", fg_hphp_gettid, (void *)&fh_hphp_gettid },
  { "thrift_protocol_write_binary", fg_thrift_protocol_write_binary, (void *)&fh_thrift_protocol_write_binary },
  { "thrift_protocol_read_binary", fg_thrift_protocol_read_binary, (void *)&fh_thrift_protocol_read_binary },
  { "thrift_protocol_set_compact_version", fg_thrift_protocol_set_compact_version, (void *)&fh_thrift_protocol_set_compact_version },
  { "thrift_protocol_write_compact", fg_thrift_protocol_write_compact, (void *)&fh_thrift_protocol_write_compact },
  { "thrift_protocol_read_compact", fg_thrift_protocol_read_compact, (void *)&fh_thrift_protocol_read_compact },
  { "base64_decode", fg_base64_decode, (void *)&fh_base64_decode },
  { "base64_encode", fg_base64_encode, (void *)&fh_base64_encode },
  { "get_headers", fg_get_headers, (void *)&fh_get_headers },
  { "get_meta_tags", fg_get_meta_tags, (void *)&fh_get_meta_tags },
  { "http_build_query", fg_http_build_query, (void *)&fh_http_build_query },
  { "parse_url", fg_parse_url, (void *)&fh_parse_url },
  { "rawurldecode", fg_rawurldecode, (void *)&fh_rawurldecode },
  { "rawurlencode", fg_rawurlencode, (void *)&fh_rawurlencode },
  { "urldecode", fg_urldecode, (void *)&fh_urldecode },
  { "urlencode", fg_urlencode, (void *)&fh_urlencode },
  { "is_bool", fg_is_bool, (void *)&fh_is_bool },
  { "is_int", fg_is_int, (void *)&fh_is_int },
  { "is_integer", fg_is_integer, (void *)&fh_is_integer },
  { "is_long", fg_is_long, (void *)&fh_is_long },
  { "is_double", fg_is_double, (void *)&fh_is_double },
  { "is_float", fg_is_float, (void *)&fh_is_float },
  { "is_numeric", fg_is_numeric, (void *)&fh_is_numeric },
  { "is_real", fg_is_real, (void *)&fh_is_real },
  { "is_string", fg_is_string, (void *)&fh_is_string },
  { "is_scalar", fg_is_scalar, (void *)&fh_is_scalar },
  { "is_array", fg_is_array, (void *)&fh_is_array },
  { "is_object", fg_is_object, (void *)&fh_is_object },
  { "is_resource", fg_is_resource, (void *)&fh_is_resource },
  { "is_null", fg_is_null, (void *)&fh_is_null },
  { "gettype", fg_gettype, (void *)&fh_gettype },
  { "get_resource_type", fg_get_resource_type, (void *)&fh_get_resource_type },
  { "intval", fg_intval, (void *)&fh_intval },
  { "doubleval", fg_doubleval, (void *)&fh_doubleval },
  { "floatval", fg_floatval, (void *)&fh_floatval },
  { "strval", fg_strval, (void *)&fh_strval },
  { "settype", fg_settype, (void *)&fh_settype },
  { "print_r", fg_print_r, (void *)&fh_print_r },
  { "var_export", fg_var_export, (void *)&fh_var_export },
  { "var_dump", fg_var_dump, (void *)&fh_var_dump },
  { "debug_zval_dump", fg_debug_zval_dump, (void *)&fh_debug_zval_dump },
  { "serialize", fg_serialize, (void *)&fh_serialize },
  { "unserialize", fg_unserialize, (void *)&fh_unserialize },
  { "get_defined_vars", fg_get_defined_vars, (void *)&fh_get_defined_vars },
  { "import_request_variables", fg_import_request_variables, (void *)&fh_import_request_variables },
  { "extract", fg_extract, (void *)&fh_extract },
  { "xml_parser_create", fg_xml_parser_create, (void *)&fh_xml_parser_create },
  { "xml_parser_free", fg_xml_parser_free, (void *)&fh_xml_parser_free },
  { "xml_parse", fg_xml_parse, (void *)&fh_xml_parse },
  { "xml_parse_into_struct", fg_xml_parse_into_struct, (void *)&fh_xml_parse_into_struct },
  { "xml_parser_create_ns", fg_xml_parser_create_ns, (void *)&fh_xml_parser_create_ns },
  { "xml_parser_get_option", fg_xml_parser_get_option, (void *)&fh_xml_parser_get_option },
  { "xml_parser_set_option", fg_xml_parser_set_option, (void *)&fh_xml_parser_set_option },
  { "xml_set_character_data_handler", fg_xml_set_character_data_handler, (void *)&fh_xml_set_character_data_handler },
  { "xml_set_default_handler", fg_xml_set_default_handler, (void *)&fh_xml_set_default_handler },
  { "xml_set_element_handler", fg_xml_set_element_handler, (void *)&fh_xml_set_element_handler },
  { "xml_set_processing_instruction_handler", fg_xml_set_processing_instruction_handler, (void *)&fh_xml_set_processing_instruction_handler },
  { "xml_set_start_namespace_decl_handler", fg_xml_set_start_namespace_decl_handler, (void *)&fh_xml_set_start_namespace_decl_handler },
  { "xml_set_end_namespace_decl_handler", fg_xml_set_end_namespace_decl_handler, (void *)&fh_xml_set_end_namespace_decl_handler },
  { "xml_set_unparsed_entity_decl_handler", fg_xml_set_unparsed_entity_decl_handler, (void *)&fh_xml_set_unparsed_entity_decl_handler },
  { "xml_set_external_entity_ref_handler", fg_xml_set_external_entity_ref_handler, (void *)&fh_xml_set_external_entity_ref_handler },
  { "xml_set_notation_decl_handler", fg_xml_set_notation_decl_handler, (void *)&fh_xml_set_notation_decl_handler },
  { "xml_set_object", fg_xml_set_object, (void *)&fh_xml_set_object },
  { "xml_get_current_byte_index", fg_xml_get_current_byte_index, (void *)&fh_xml_get_current_byte_index },
  { "xml_get_current_column_number", fg_xml_get_current_column_number, (void *)&fh_xml_get_current_column_number },
  { "xml_get_current_line_number", fg_xml_get_current_line_number, (void *)&fh_xml_get_current_line_number },
  { "xml_get_error_code", fg_xml_get_error_code, (void *)&fh_xml_get_error_code },
  { "xml_error_string", fg_xml_error_string, (void *)&fh_xml_error_string },
  { "utf8_decode", fg_utf8_decode, (void *)&fh_utf8_decode },
  { "utf8_encode", fg_utf8_encode, (void *)&fh_utf8_encode },
  { "xmlwriter_open_memory", fg_xmlwriter_open_memory, (void *)&fh_xmlwriter_open_memory },
  { "xmlwriter_open_uri", fg_xmlwriter_open_uri, (void *)&fh_xmlwriter_open_uri },
  { "xmlwriter_set_indent_string", fg_xmlwriter_set_indent_string, (void *)&fh_xmlwriter_set_indent_string },
  { "xmlwriter_set_indent", fg_xmlwriter_set_indent, (void *)&fh_xmlwriter_set_indent },
  { "xmlwriter_start_document", fg_xmlwriter_start_document, (void *)&fh_xmlwriter_start_document },
  { "xmlwriter_start_element", fg_xmlwriter_start_element, (void *)&fh_xmlwriter_start_element },
  { "xmlwriter_start_element_ns", fg_xmlwriter_start_element_ns, (void *)&fh_xmlwriter_start_element_ns },
  { "xmlwriter_write_element_ns", fg_xmlwriter_write_element_ns, (void *)&fh_xmlwriter_write_element_ns },
  { "xmlwriter_write_element", fg_xmlwriter_write_element, (void *)&fh_xmlwriter_write_element },
  { "xmlwriter_end_element", fg_xmlwriter_end_element, (void *)&fh_xmlwriter_end_element },
  { "xmlwriter_full_end_element", fg_xmlwriter_full_end_element, (void *)&fh_xmlwriter_full_end_element },
  { "xmlwriter_start_attribute_ns", fg_xmlwriter_start_attribute_ns, (void *)&fh_xmlwriter_start_attribute_ns },
  { "xmlwriter_start_attribute", fg_xmlwriter_start_attribute, (void *)&fh_xmlwriter_start_attribute },
  { "xmlwriter_write_attribute_ns", fg_xmlwriter_write_attribute_ns, (void *)&fh_xmlwriter_write_attribute_ns },
  { "xmlwriter_write_attribute", fg_xmlwriter_write_attribute, (void *)&fh_xmlwriter_write_attribute },
  { "xmlwriter_end_attribute", fg_xmlwriter_end_attribute, (void *)&fh_xmlwriter_end_attribute },
  { "xmlwriter_start_cdata", fg_xmlwriter_start_cdata, (void *)&fh_xmlwriter_start_cdata },
  { "xmlwriter_write_cdata", fg_xmlwriter_write_cdata, (void *)&fh_xmlwriter_write_cdata },
  { "xmlwriter_end_cdata", fg_xmlwriter_end_cdata, (void *)&fh_xmlwriter_end_cdata },
  { "xmlwriter_start_comment", fg_xmlwriter_start_comment, (void *)&fh_xmlwriter_start_comment },
  { "xmlwriter_write_comment", fg_xmlwriter_write_comment, (void *)&fh_xmlwriter_write_comment },
  { "xmlwriter_end_comment", fg_xmlwriter_end_comment, (void *)&fh_xmlwriter_end_comment },
  { "xmlwriter_end_document", fg_xmlwriter_end_document, (void *)&fh_xmlwriter_end_document },
  { "xmlwriter_start_pi", fg_xmlwriter_start_pi, (void *)&fh_xmlwriter_start_pi },
  { "xmlwriter_write_pi", fg_xmlwriter_write_pi, (void *)&fh_xmlwriter_write_pi },
  { "xmlwriter_end_pi", fg_xmlwriter_end_pi, (void *)&fh_xmlwriter_end_pi },
  { "xmlwriter_text", fg_xmlwriter_text, (void *)&fh_xmlwriter_text },
  { "xmlwriter_write_raw", fg_xmlwriter_write_raw, (void *)&fh_xmlwriter_write_raw },
  { "xmlwriter_start_dtd", fg_xmlwriter_start_dtd, (void *)&fh_xmlwriter_start_dtd },
  { "xmlwriter_write_dtd", fg_xmlwriter_write_dtd, (void *)&fh_xmlwriter_write_dtd },
  { "xmlwriter_start_dtd_element", fg_xmlwriter_start_dtd_element, (void *)&fh_xmlwriter_start_dtd_element },
  { "xmlwriter_write_dtd_element", fg_xmlwriter_write_dtd_element, (void *)&fh_xmlwriter_write_dtd_element },
  { "xmlwriter_end_dtd_element", fg_xmlwriter_end_dtd_element, (void *)&fh_xmlwriter_end_dtd_element },
  { "xmlwriter_start_dtd_attlist", fg_xmlwriter_start_dtd_attlist, (void *)&fh_xmlwriter_start_dtd_attlist },
  { "xmlwriter_write_dtd_attlist", fg_xmlwriter_write_dtd_attlist, (void *)&fh_xmlwriter_write_dtd_attlist },
  { "xmlwriter_end_dtd_attlist", fg_xmlwriter_end_dtd_attlist, (void *)&fh_xmlwriter_end_dtd_attlist },
  { "xmlwriter_start_dtd_entity", fg_xmlwriter_start_dtd_entity, (void *)&fh_xmlwriter_start_dtd_entity },
  { "xmlwriter_write_dtd_entity", fg_xmlwriter_write_dtd_entity, (void *)&fh_xmlwriter_write_dtd_entity },
  { "xmlwriter_end_dtd_entity", fg_xmlwriter_end_dtd_entity, (void *)&fh_xmlwriter_end_dtd_entity },
  { "xmlwriter_end_dtd", fg_xmlwriter_end_dtd, (void *)&fh_xmlwriter_end_dtd },
  { "xmlwriter_flush", fg_xmlwriter_flush, (void *)&fh_xmlwriter_flush },
  { "xmlwriter_output_memory", fg_xmlwriter_output_memory, (void *)&fh_xmlwriter_output_memory },
  { "readgzfile", fg_readgzfile, (void *)&fh_readgzfile },
  { "gzfile", fg_gzfile, (void *)&fh_gzfile },
  { "gzcompress", fg_gzcompress, (void *)&fh_gzcompress },
  { "gzuncompress", fg_gzuncompress, (void *)&fh_gzuncompress },
  { "gzdeflate", fg_gzdeflate, (void *)&fh_gzdeflate },
  { "gzinflate", fg_gzinflate, (void *)&fh_gzinflate },
  { "gzencode", fg_gzencode, (void *)&fh_gzencode },
  { "gzdecode", fg_gzdecode, (void *)&fh_gzdecode },
  { "zlib_get_coding_type", fg_zlib_get_coding_type, (void *)&fh_zlib_get_coding_type },
  { "gzopen", fg_gzopen, (void *)&fh_gzopen },
  { "gzclose", fg_gzclose, (void *)&fh_gzclose },
  { "gzrewind", fg_gzrewind, (void *)&fh_gzrewind },
  { "gzeof", fg_gzeof, (void *)&fh_gzeof },
  { "gzgetc", fg_gzgetc, (void *)&fh_gzgetc },
  { "gzgets", fg_gzgets, (void *)&fh_gzgets },
  { "gzgetss", fg_gzgetss, (void *)&fh_gzgetss },
  { "gzread", fg_gzread, (void *)&fh_gzread },
  { "gzpassthru", fg_gzpassthru, (void *)&fh_gzpassthru },
  { "gzseek", fg_gzseek, (void *)&fh_gzseek },
  { "gztell", fg_gztell, (void *)&fh_gztell },
  { "gzwrite", fg_gzwrite, (void *)&fh_gzwrite },
  { "gzputs", fg_gzputs, (void *)&fh_gzputs },
  { "qlzcompress", fg_qlzcompress, (void *)&fh_qlzcompress },
  { "qlzuncompress", fg_qlzuncompress, (void *)&fh_qlzuncompress },
  { "sncompress", fg_sncompress, (void *)&fh_sncompress },
  { "snuncompress", fg_snuncompress, (void *)&fh_snuncompress },
  { "nzcompress", fg_nzcompress, (void *)&fh_nzcompress },
  { "nzuncompress", fg_nzuncompress, (void *)&fh_nzuncompress },
  { "lz4compress", fg_lz4compress, (void *)&fh_lz4compress },
  { "lz4hccompress", fg_lz4hccompress, (void *)&fh_lz4hccompress },
  { "lz4uncompress", fg_lz4uncompress, (void *)&fh_lz4uncompress }
};

static const long long hhbc_ext_method_count_WaitHandle = 10;
static const HhbcExtMethodInfo hhbc_ext_methods_WaitHandle[] = {
  { "__construct", tg_10WaitHandle___construct },
  { "getWaitHandle", tg_10WaitHandle_getWaitHandle },
  { "import", tg_10WaitHandle_import },
  { "join", tg_10WaitHandle_join },
  { "isFinished", tg_10WaitHandle_isFinished },
  { "isSucceeded", tg_10WaitHandle_isSucceeded },
  { "isFailed", tg_10WaitHandle_isFailed },
  { "getID", tg_10WaitHandle_getID },
  { "getName", tg_10WaitHandle_getName },
  { "getExceptionIfFailed", tg_10WaitHandle_getExceptionIfFailed }
};

static const long long hhbc_ext_method_count_StaticWaitHandle = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_StaticWaitHandle[] = {
  { "__construct", tg_16StaticWaitHandle___construct }
};

static const long long hhbc_ext_method_count_StaticResultWaitHandle = 2;
static const HhbcExtMethodInfo hhbc_ext_methods_StaticResultWaitHandle[] = {
  { "__construct", tg_22StaticResultWaitHandle___construct },
  { "create", tg_22StaticResultWaitHandle_create }
};

static const long long hhbc_ext_method_count_StaticExceptionWaitHandle = 2;
static const HhbcExtMethodInfo hhbc_ext_methods_StaticExceptionWaitHandle[] = {
  { "__construct", tg_25StaticExceptionWaitHandle___construct },
  { "create", tg_25StaticExceptionWaitHandle_create }
};

static const long long hhbc_ext_method_count_WaitableWaitHandle = 5;
static const HhbcExtMethodInfo hhbc_ext_methods_WaitableWaitHandle[] = {
  { "__construct", tg_18WaitableWaitHandle___construct },
  { "getContextIdx", tg_18WaitableWaitHandle_getContextIdx },
  { "getCreator", tg_18WaitableWaitHandle_getCreator },
  { "getParents", tg_18WaitableWaitHandle_getParents },
  { "getStackTrace", tg_18WaitableWaitHandle_getStackTrace }
};

static const long long hhbc_ext_method_count_BlockableWaitHandle = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_BlockableWaitHandle[] = {
  { "__construct", tg_19BlockableWaitHandle___construct }
};

static const long long hhbc_ext_method_count_ContinuationWaitHandle = 4;
static const HhbcExtMethodInfo hhbc_ext_methods_ContinuationWaitHandle[] = {
  { "__construct", tg_22ContinuationWaitHandle___construct },
  { "start", tg_22ContinuationWaitHandle_start },
  { "getPrivData", tg_22ContinuationWaitHandle_getPrivData },
  { "setPrivData", tg_22ContinuationWaitHandle_setPrivData }
};

static const long long hhbc_ext_method_count_GenArrayWaitHandle = 2;
static const HhbcExtMethodInfo hhbc_ext_methods_GenArrayWaitHandle[] = {
  { "__construct", tg_18GenArrayWaitHandle___construct },
  { "create", tg_18GenArrayWaitHandle_create }
};

static const long long hhbc_ext_method_count_SetResultToRefWaitHandle = 2;
static const HhbcExtMethodInfo hhbc_ext_methods_SetResultToRefWaitHandle[] = {
  { "__construct", tg_24SetResultToRefWaitHandle___construct },
  { "create", tg_24SetResultToRefWaitHandle_create }
};

static const long long hhbc_ext_method_count_RescheduleWaitHandle = 2;
static const HhbcExtMethodInfo hhbc_ext_methods_RescheduleWaitHandle[] = {
  { "__construct", tg_20RescheduleWaitHandle___construct },
  { "create", tg_20RescheduleWaitHandle_create }
};

static const long long hhbc_ext_method_count_Closure = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_Closure[] = {
  { "__construct", tg_7Closure___construct }
};

static const long long hhbc_ext_method_count_DummyClosure = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_DummyClosure[] = {
  { "__construct", tg_12DummyClosure___construct }
};

static const long long hhbc_ext_method_count_Vector = 40;
static const HhbcExtMethodInfo hhbc_ext_methods_Vector[] = {
  { "__construct", tg_6Vector___construct },
  { "isEmpty", tg_6Vector_isEmpty },
  { "count", tg_6Vector_count },
  { "items", tg_6Vector_items },
  { "keys", tg_6Vector_keys },
  { "view", tg_6Vector_view },
  { "kvzip", tg_6Vector_kvzip },
  { "at", tg_6Vector_at },
  { "get", tg_6Vector_get },
  { "set", tg_6Vector_set },
  { "setAll", tg_6Vector_setAll },
  { "put", tg_6Vector_put },
  { "clear", tg_6Vector_clear },
  { "contains", tg_6Vector_contains },
  { "containsKey", tg_6Vector_containsKey },
  { "removeKey", tg_6Vector_removeKey },
  { "append", tg_6Vector_append },
  { "add", tg_6Vector_add },
  { "addAll", tg_6Vector_addAll },
  { "pop", tg_6Vector_pop },
  { "resize", tg_6Vector_resize },
  { "toArray", tg_6Vector_toArray },
  { "getIterator", tg_6Vector_getIterator },
  { "map", tg_6Vector_map },
  { "filter", tg_6Vector_filter },
  { "zip", tg_6Vector_zip },
  { "sort", tg_6Vector_sort },
  { "reverse", tg_6Vector_reverse },
  { "splice", tg_6Vector_splice },
  { "linearSearch", tg_6Vector_linearSearch },
  { "shuffle", tg_6Vector_shuffle },
  { "__toString", tg_6Vector___toString },
  { "__get", tg_6Vector___get },
  { "__set", tg_6Vector___set },
  { "__isset", tg_6Vector___isset },
  { "__unset", tg_6Vector___unset },
  { "fromItems", tg_6Vector_fromItems },
  { "fromArray", tg_6Vector_fromArray },
  { "fromVector", tg_6Vector_fromVector },
  { "slice", tg_6Vector_slice }
};

static const long long hhbc_ext_method_count_VectorIterator = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_VectorIterator[] = {
  { "__construct", tg_14VectorIterator___construct },
  { "current", tg_14VectorIterator_current },
  { "key", tg_14VectorIterator_key },
  { "valid", tg_14VectorIterator_valid },
  { "next", tg_14VectorIterator_next },
  { "rewind", tg_14VectorIterator_rewind }
};

static const long long hhbc_ext_method_count_Map = 40;
static const HhbcExtMethodInfo hhbc_ext_methods_Map[] = {
  { "__construct", tg_3Map___construct },
  { "isEmpty", tg_3Map_isEmpty },
  { "count", tg_3Map_count },
  { "items", tg_3Map_items },
  { "keys", tg_3Map_keys },
  { "view", tg_3Map_view },
  { "kvzip", tg_3Map_kvzip },
  { "at", tg_3Map_at },
  { "get", tg_3Map_get },
  { "set", tg_3Map_set },
  { "setAll", tg_3Map_setAll },
  { "put", tg_3Map_put },
  { "clear", tg_3Map_clear },
  { "contains", tg_3Map_contains },
  { "containsKey", tg_3Map_containsKey },
  { "remove", tg_3Map_remove },
  { "removeKey", tg_3Map_removeKey },
  { "discard", tg_3Map_discard },
  { "add", tg_3Map_add },
  { "addAll", tg_3Map_addAll },
  { "toArray", tg_3Map_toArray },
  { "copyAsArray", tg_3Map_copyAsArray },
  { "toKeysArray", tg_3Map_toKeysArray },
  { "values", tg_3Map_values },
  { "toValuesArray", tg_3Map_toValuesArray },
  { "updateFromArray", tg_3Map_updateFromArray },
  { "updateFromIterable", tg_3Map_updateFromIterable },
  { "differenceByKey", tg_3Map_differenceByKey },
  { "getIterator", tg_3Map_getIterator },
  { "map", tg_3Map_map },
  { "filter", tg_3Map_filter },
  { "zip", tg_3Map_zip },
  { "__toString", tg_3Map___toString },
  { "__get", tg_3Map___get },
  { "__set", tg_3Map___set },
  { "__isset", tg_3Map___isset },
  { "__unset", tg_3Map___unset },
  { "fromItems", tg_3Map_fromItems },
  { "fromArray", tg_3Map_fromArray },
  { "fromIterable", tg_3Map_fromIterable }
};

static const long long hhbc_ext_method_count_MapIterator = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_MapIterator[] = {
  { "__construct", tg_11MapIterator___construct },
  { "current", tg_11MapIterator_current },
  { "key", tg_11MapIterator_key },
  { "valid", tg_11MapIterator_valid },
  { "next", tg_11MapIterator_next },
  { "rewind", tg_11MapIterator_rewind }
};

static const long long hhbc_ext_method_count_StableMap = 40;
static const HhbcExtMethodInfo hhbc_ext_methods_StableMap[] = {
  { "__construct", tg_9StableMap___construct },
  { "isEmpty", tg_9StableMap_isEmpty },
  { "count", tg_9StableMap_count },
  { "items", tg_9StableMap_items },
  { "keys", tg_9StableMap_keys },
  { "view", tg_9StableMap_view },
  { "kvzip", tg_9StableMap_kvzip },
  { "at", tg_9StableMap_at },
  { "get", tg_9StableMap_get },
  { "set", tg_9StableMap_set },
  { "setAll", tg_9StableMap_setAll },
  { "put", tg_9StableMap_put },
  { "clear", tg_9StableMap_clear },
  { "contains", tg_9StableMap_contains },
  { "containsKey", tg_9StableMap_containsKey },
  { "remove", tg_9StableMap_remove },
  { "removeKey", tg_9StableMap_removeKey },
  { "discard", tg_9StableMap_discard },
  { "add", tg_9StableMap_add },
  { "addAll", tg_9StableMap_addAll },
  { "toArray", tg_9StableMap_toArray },
  { "copyAsArray", tg_9StableMap_copyAsArray },
  { "toKeysArray", tg_9StableMap_toKeysArray },
  { "values", tg_9StableMap_values },
  { "toValuesArray", tg_9StableMap_toValuesArray },
  { "updateFromArray", tg_9StableMap_updateFromArray },
  { "updateFromIterable", tg_9StableMap_updateFromIterable },
  { "differenceByKey", tg_9StableMap_differenceByKey },
  { "getIterator", tg_9StableMap_getIterator },
  { "map", tg_9StableMap_map },
  { "filter", tg_9StableMap_filter },
  { "zip", tg_9StableMap_zip },
  { "__get", tg_9StableMap___get },
  { "__set", tg_9StableMap___set },
  { "__isset", tg_9StableMap___isset },
  { "__unset", tg_9StableMap___unset },
  { "fromItems", tg_9StableMap_fromItems },
  { "__toString", tg_9StableMap___toString },
  { "fromArray", tg_9StableMap_fromArray },
  { "fromIterable", tg_9StableMap_fromIterable }
};

static const long long hhbc_ext_method_count_StableMapIterator = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_StableMapIterator[] = {
  { "__construct", tg_17StableMapIterator___construct },
  { "current", tg_17StableMapIterator_current },
  { "key", tg_17StableMapIterator_key },
  { "valid", tg_17StableMapIterator_valid },
  { "next", tg_17StableMapIterator_next },
  { "rewind", tg_17StableMapIterator_rewind }
};

static const long long hhbc_ext_method_count_Pair = 15;
static const HhbcExtMethodInfo hhbc_ext_methods_Pair[] = {
  { "__construct", tg_4Pair___construct },
  { "isEmpty", tg_4Pair_isEmpty },
  { "count", tg_4Pair_count },
  { "items", tg_4Pair_items },
  { "keys", tg_4Pair_keys },
  { "view", tg_4Pair_view },
  { "kvzip", tg_4Pair_kvzip },
  { "toArray", tg_4Pair_toArray },
  { "getIterator", tg_4Pair_getIterator },
  { "map", tg_4Pair_map },
  { "filter", tg_4Pair_filter },
  { "zip", tg_4Pair_zip },
  { "at", tg_4Pair_at },
  { "get", tg_4Pair_get },
  { "containsKey", tg_4Pair_containsKey }
};

static const long long hhbc_ext_method_count_PairIterator = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_PairIterator[] = {
  { "__construct", tg_12PairIterator___construct },
  { "current", tg_12PairIterator_current },
  { "key", tg_12PairIterator_key },
  { "valid", tg_12PairIterator_valid },
  { "next", tg_12PairIterator_next },
  { "rewind", tg_12PairIterator_rewind }
};

static const long long hhbc_ext_method_count_Continuation = 16;
static const HhbcExtMethodInfo hhbc_ext_methods_Continuation[] = {
  { "__construct", tg_12Continuation___construct },
  { "update", tg_12Continuation_update },
  { "getWaitHandle", tg_12Continuation_getWaitHandle },
  { "getLabel", tg_12Continuation_getLabel },
  { "num_args", tg_12Continuation_num_args },
  { "get_args", tg_12Continuation_get_args },
  { "get_arg", tg_12Continuation_get_arg },
  { "current", tg_12Continuation_current },
  { "key", tg_12Continuation_key },
  { "next", tg_12Continuation_next },
  { "rewind", tg_12Continuation_rewind },
  { "valid", tg_12Continuation_valid },
  { "send", tg_12Continuation_send },
  { "raise", tg_12Continuation_raise },
  { "getOrigFuncName", tg_12Continuation_getOrigFuncName },
  { "__clone", tg_12Continuation___clone }
};

static const long long hhbc_ext_method_count_DummyContinuation = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_DummyContinuation[] = {
  { "__construct", tg_17DummyContinuation___construct },
  { "current", tg_17DummyContinuation_current },
  { "key", tg_17DummyContinuation_key },
  { "next", tg_17DummyContinuation_next },
  { "rewind", tg_17DummyContinuation_rewind },
  { "valid", tg_17DummyContinuation_valid }
};

static const long long hhbc_ext_method_count_DateTime = 16;
static const HhbcExtMethodInfo hhbc_ext_methods_DateTime[] = {
  { "add", tg_8DateTime_add },
  { "__construct", tg_8DateTime___construct },
  { "createFromFormat", tg_8DateTime_createFromFormat },
  { "diff", tg_8DateTime_diff },
  { "format", tg_8DateTime_format },
  { "getLastErrors", tg_8DateTime_getLastErrors },
  { "getOffset", tg_8DateTime_getOffset },
  { "getTimestamp", tg_8DateTime_getTimestamp },
  { "getTimezone", tg_8DateTime_getTimezone },
  { "modify", tg_8DateTime_modify },
  { "setDate", tg_8DateTime_setDate },
  { "setISODate", tg_8DateTime_setISODate },
  { "setTime", tg_8DateTime_setTime },
  { "setTimestamp", tg_8DateTime_setTimestamp },
  { "setTimezone", tg_8DateTime_setTimezone },
  { "sub", tg_8DateTime_sub }
};

static const long long hhbc_ext_method_count_DateTimeZone = 7;
static const HhbcExtMethodInfo hhbc_ext_methods_DateTimeZone[] = {
  { "__construct", tg_12DateTimeZone___construct },
  { "getLocation", tg_12DateTimeZone_getLocation },
  { "getName", tg_12DateTimeZone_getName },
  { "getOffset", tg_12DateTimeZone_getOffset },
  { "getTransitions", tg_12DateTimeZone_getTransitions },
  { "listAbbreviations", tg_12DateTimeZone_listAbbreviations },
  { "listIdentifiers", tg_12DateTimeZone_listIdentifiers }
};

static const long long hhbc_ext_method_count_DateInterval = 5;
static const HhbcExtMethodInfo hhbc_ext_methods_DateInterval[] = {
  { "__construct", tg_12DateInterval___construct },
  { "__get", tg_12DateInterval___get },
  { "__set", tg_12DateInterval___set },
  { "createFromDateString", tg_12DateInterval_createFromDateString },
  { "format", tg_12DateInterval_format }
};

static const long long hhbc_ext_method_count_DebuggerProxyCmdUser = 3;
static const HhbcExtMethodInfo hhbc_ext_methods_DebuggerProxyCmdUser[] = {
  { "__construct", tg_20DebuggerProxyCmdUser___construct },
  { "isLocal", tg_20DebuggerProxyCmdUser_isLocal },
  { "send", tg_20DebuggerProxyCmdUser_send }
};

static const long long hhbc_ext_method_count_DebuggerClientCmdUser = 29;
static const HhbcExtMethodInfo hhbc_ext_methods_DebuggerClientCmdUser[] = {
  { "__construct", tg_21DebuggerClientCmdUser___construct },
  { "quit", tg_21DebuggerClientCmdUser_quit },
  { "print", tg_21DebuggerClientCmdUser_print },
  { "help", tg_21DebuggerClientCmdUser_help },
  { "info", tg_21DebuggerClientCmdUser_info },
  { "output", tg_21DebuggerClientCmdUser_output },
  { "error", tg_21DebuggerClientCmdUser_error },
  { "code", tg_21DebuggerClientCmdUser_code },
  { "ask", tg_21DebuggerClientCmdUser_ask },
  { "wrap", tg_21DebuggerClientCmdUser_wrap },
  { "helpTitle", tg_21DebuggerClientCmdUser_helpTitle },
  { "helpCmds", tg_21DebuggerClientCmdUser_helpCmds },
  { "helpBody", tg_21DebuggerClientCmdUser_helpBody },
  { "helpSection", tg_21DebuggerClientCmdUser_helpSection },
  { "tutorial", tg_21DebuggerClientCmdUser_tutorial },
  { "getCode", tg_21DebuggerClientCmdUser_getCode },
  { "getCommand", tg_21DebuggerClientCmdUser_getCommand },
  { "arg", tg_21DebuggerClientCmdUser_arg },
  { "argCount", tg_21DebuggerClientCmdUser_argCount },
  { "argValue", tg_21DebuggerClientCmdUser_argValue },
  { "lineRest", tg_21DebuggerClientCmdUser_lineRest },
  { "args", tg_21DebuggerClientCmdUser_args },
  { "send", tg_21DebuggerClientCmdUser_send },
  { "xend", tg_21DebuggerClientCmdUser_xend },
  { "getCurrentLocation", tg_21DebuggerClientCmdUser_getCurrentLocation },
  { "getStackTrace", tg_21DebuggerClientCmdUser_getStackTrace },
  { "getFrame", tg_21DebuggerClientCmdUser_getFrame },
  { "printFrame", tg_21DebuggerClientCmdUser_printFrame },
  { "addCompletion", tg_21DebuggerClientCmdUser_addCompletion }
};

static const long long hhbc_ext_method_count_DebuggerClient = 4;
static const HhbcExtMethodInfo hhbc_ext_methods_DebuggerClient[] = {
  { "__construct", tg_14DebuggerClient___construct },
  { "getState", tg_14DebuggerClient_getState },
  { "init", tg_14DebuggerClient_init },
  { "processCmd", tg_14DebuggerClient_processCmd }
};

static const long long hhbc_ext_method_count_DOMNode = 21;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMNode[] = {
  { "__construct", tg_7DOMNode___construct },
  { "appendChild", tg_7DOMNode_appendChild },
  { "cloneNode", tg_7DOMNode_cloneNode },
  { "getLineNo", tg_7DOMNode_getLineNo },
  { "hasAttributes", tg_7DOMNode_hasAttributes },
  { "hasChildNodes", tg_7DOMNode_hasChildNodes },
  { "insertBefore", tg_7DOMNode_insertBefore },
  { "isDefaultNamespace", tg_7DOMNode_isDefaultNamespace },
  { "isSameNode", tg_7DOMNode_isSameNode },
  { "isSupported", tg_7DOMNode_isSupported },
  { "lookupNamespaceUri", tg_7DOMNode_lookupNamespaceUri },
  { "lookupPrefix", tg_7DOMNode_lookupPrefix },
  { "normalize", tg_7DOMNode_normalize },
  { "removeChild", tg_7DOMNode_removeChild },
  { "replaceChild", tg_7DOMNode_replaceChild },
  { "c14n", tg_7DOMNode_c14n },
  { "c14nfile", tg_7DOMNode_c14nfile },
  { "getNodePath", tg_7DOMNode_getNodePath },
  { "__get", tg_7DOMNode___get },
  { "__set", tg_7DOMNode___set },
  { "__isset", tg_7DOMNode___isset }
};

static const long long hhbc_ext_method_count_DOMAttr = 5;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMAttr[] = {
  { "__construct", tg_7DOMAttr___construct },
  { "isId", tg_7DOMAttr_isId },
  { "__get", tg_7DOMAttr___get },
  { "__set", tg_7DOMAttr___set },
  { "__isset", tg_7DOMAttr___isset }
};

static const long long hhbc_ext_method_count_DOMCharacterData = 9;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMCharacterData[] = {
  { "__construct", tg_16DOMCharacterData___construct },
  { "appendData", tg_16DOMCharacterData_appendData },
  { "deleteData", tg_16DOMCharacterData_deleteData },
  { "insertData", tg_16DOMCharacterData_insertData },
  { "replaceData", tg_16DOMCharacterData_replaceData },
  { "substringData", tg_16DOMCharacterData_substringData },
  { "__get", tg_16DOMCharacterData___get },
  { "__set", tg_16DOMCharacterData___set },
  { "__isset", tg_16DOMCharacterData___isset }
};

static const long long hhbc_ext_method_count_DOMComment = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMComment[] = {
  { "__construct", tg_10DOMComment___construct }
};

static const long long hhbc_ext_method_count_DOMText = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMText[] = {
  { "__construct", tg_7DOMText___construct },
  { "isWhitespaceInElementContent", tg_7DOMText_isWhitespaceInElementContent },
  { "splitText", tg_7DOMText_splitText },
  { "__get", tg_7DOMText___get },
  { "__set", tg_7DOMText___set },
  { "__isset", tg_7DOMText___isset }
};

static const long long hhbc_ext_method_count_DOMCDATASection = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMCDATASection[] = {
  { "__construct", tg_15DOMCDATASection___construct }
};

static const long long hhbc_ext_method_count_DOMDocument = 34;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMDocument[] = {
  { "__construct", tg_11DOMDocument___construct },
  { "createAttribute", tg_11DOMDocument_createAttribute },
  { "createAttributens", tg_11DOMDocument_createAttributens },
  { "createCDATASection", tg_11DOMDocument_createCDATASection },
  { "createComment", tg_11DOMDocument_createComment },
  { "createDocumentFragment", tg_11DOMDocument_createDocumentFragment },
  { "createElement", tg_11DOMDocument_createElement },
  { "createElementNS", tg_11DOMDocument_createElementNS },
  { "createEntityReference", tg_11DOMDocument_createEntityReference },
  { "createProcessingInstruction", tg_11DOMDocument_createProcessingInstruction },
  { "createTextNode", tg_11DOMDocument_createTextNode },
  { "getElementById", tg_11DOMDocument_getElementById },
  { "getElementsByTagName", tg_11DOMDocument_getElementsByTagName },
  { "getElementsByTagNameNS", tg_11DOMDocument_getElementsByTagNameNS },
  { "importNode", tg_11DOMDocument_importNode },
  { "load", tg_11DOMDocument_load },
  { "loadHTML", tg_11DOMDocument_loadHTML },
  { "loadHTMLFile", tg_11DOMDocument_loadHTMLFile },
  { "loadXML", tg_11DOMDocument_loadXML },
  { "normalizeDocument", tg_11DOMDocument_normalizeDocument },
  { "registerNodeClass", tg_11DOMDocument_registerNodeClass },
  { "relaxNGValidate", tg_11DOMDocument_relaxNGValidate },
  { "relaxNGValidateSource", tg_11DOMDocument_relaxNGValidateSource },
  { "save", tg_11DOMDocument_save },
  { "saveHTML", tg_11DOMDocument_saveHTML },
  { "saveHTMLFile", tg_11DOMDocument_saveHTMLFile },
  { "saveXML", tg_11DOMDocument_saveXML },
  { "schemaValidate", tg_11DOMDocument_schemaValidate },
  { "schemaValidateSource", tg_11DOMDocument_schemaValidateSource },
  { "validate", tg_11DOMDocument_validate },
  { "xinclude", tg_11DOMDocument_xinclude },
  { "__get", tg_11DOMDocument___get },
  { "__set", tg_11DOMDocument___set },
  { "__isset", tg_11DOMDocument___isset }
};

static const long long hhbc_ext_method_count_DOMDocumentFragment = 2;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMDocumentFragment[] = {
  { "__construct", tg_19DOMDocumentFragment___construct },
  { "appendXML", tg_19DOMDocumentFragment_appendXML }
};

static const long long hhbc_ext_method_count_DOMDocumentType = 4;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMDocumentType[] = {
  { "__construct", tg_15DOMDocumentType___construct },
  { "__get", tg_15DOMDocumentType___get },
  { "__set", tg_15DOMDocumentType___set },
  { "__isset", tg_15DOMDocumentType___isset }
};

static const long long hhbc_ext_method_count_DOMElement = 22;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMElement[] = {
  { "__construct", tg_10DOMElement___construct },
  { "getAttribute", tg_10DOMElement_getAttribute },
  { "getAttributeNode", tg_10DOMElement_getAttributeNode },
  { "getAttributeNodeNS", tg_10DOMElement_getAttributeNodeNS },
  { "getAttributeNS", tg_10DOMElement_getAttributeNS },
  { "getElementsByTagName", tg_10DOMElement_getElementsByTagName },
  { "getElementsByTagNameNS", tg_10DOMElement_getElementsByTagNameNS },
  { "hasAttribute", tg_10DOMElement_hasAttribute },
  { "hasAttributeNS", tg_10DOMElement_hasAttributeNS },
  { "removeAttribute", tg_10DOMElement_removeAttribute },
  { "removeAttributeNode", tg_10DOMElement_removeAttributeNode },
  { "removeAttributeNS", tg_10DOMElement_removeAttributeNS },
  { "setAttribute", tg_10DOMElement_setAttribute },
  { "setAttributeNode", tg_10DOMElement_setAttributeNode },
  { "setAttributeNodeNS", tg_10DOMElement_setAttributeNodeNS },
  { "setAttributeNS", tg_10DOMElement_setAttributeNS },
  { "setIDAttribute", tg_10DOMElement_setIDAttribute },
  { "setIDAttributeNode", tg_10DOMElement_setIDAttributeNode },
  { "setIDAttributeNS", tg_10DOMElement_setIDAttributeNS },
  { "__get", tg_10DOMElement___get },
  { "__set", tg_10DOMElement___set },
  { "__isset", tg_10DOMElement___isset }
};

static const long long hhbc_ext_method_count_DOMEntity = 4;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMEntity[] = {
  { "__construct", tg_9DOMEntity___construct },
  { "__get", tg_9DOMEntity___get },
  { "__set", tg_9DOMEntity___set },
  { "__isset", tg_9DOMEntity___isset }
};

static const long long hhbc_ext_method_count_DOMEntityReference = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMEntityReference[] = {
  { "__construct", tg_18DOMEntityReference___construct }
};

static const long long hhbc_ext_method_count_DOMNotation = 4;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMNotation[] = {
  { "__construct", tg_11DOMNotation___construct },
  { "__get", tg_11DOMNotation___get },
  { "__set", tg_11DOMNotation___set },
  { "__isset", tg_11DOMNotation___isset }
};

static const long long hhbc_ext_method_count_DOMProcessingInstruction = 4;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMProcessingInstruction[] = {
  { "__construct", tg_24DOMProcessingInstruction___construct },
  { "__get", tg_24DOMProcessingInstruction___get },
  { "__set", tg_24DOMProcessingInstruction___set },
  { "__isset", tg_24DOMProcessingInstruction___isset }
};

static const long long hhbc_ext_method_count_DOMNodeIterator = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMNodeIterator[] = {
  { "__construct", tg_15DOMNodeIterator___construct },
  { "current", tg_15DOMNodeIterator_current },
  { "key", tg_15DOMNodeIterator_key },
  { "next", tg_15DOMNodeIterator_next },
  { "rewind", tg_15DOMNodeIterator_rewind },
  { "valid", tg_15DOMNodeIterator_valid }
};

static const long long hhbc_ext_method_count_DOMNamedNodeMap = 8;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMNamedNodeMap[] = {
  { "__construct", tg_15DOMNamedNodeMap___construct },
  { "getNamedItem", tg_15DOMNamedNodeMap_getNamedItem },
  { "getNamedItemNS", tg_15DOMNamedNodeMap_getNamedItemNS },
  { "item", tg_15DOMNamedNodeMap_item },
  { "__get", tg_15DOMNamedNodeMap___get },
  { "__set", tg_15DOMNamedNodeMap___set },
  { "__isset", tg_15DOMNamedNodeMap___isset },
  { "getIterator", tg_15DOMNamedNodeMap_getIterator }
};

static const long long hhbc_ext_method_count_DOMNodeList = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMNodeList[] = {
  { "__construct", tg_11DOMNodeList___construct },
  { "item", tg_11DOMNodeList_item },
  { "__get", tg_11DOMNodeList___get },
  { "__set", tg_11DOMNodeList___set },
  { "__isset", tg_11DOMNodeList___isset },
  { "getIterator", tg_11DOMNodeList_getIterator }
};

static const long long hhbc_ext_method_count_DOMImplementation = 4;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMImplementation[] = {
  { "__construct", tg_17DOMImplementation___construct },
  { "createDocument", tg_17DOMImplementation_createDocument },
  { "createDocumentType", tg_17DOMImplementation_createDocumentType },
  { "hasFeature", tg_17DOMImplementation_hasFeature }
};

static const long long hhbc_ext_method_count_DOMXPath = 8;
static const HhbcExtMethodInfo hhbc_ext_methods_DOMXPath[] = {
  { "__construct", tg_8DOMXPath___construct },
  { "evaluate", tg_8DOMXPath_evaluate },
  { "query", tg_8DOMXPath_query },
  { "registerNamespace", tg_8DOMXPath_registerNamespace },
  { "registerPHPFunctions", tg_8DOMXPath_registerPHPFunctions },
  { "__get", tg_8DOMXPath___get },
  { "__set", tg_8DOMXPath___set },
  { "__isset", tg_8DOMXPath___isset }
};

static const long long hhbc_ext_method_count_UConverter = 22;
static const HhbcExtMethodInfo hhbc_ext_methods_UConverter[] = {
  { "__construct", tg_10UConverter___construct },
  { "__destruct", tg_10UConverter___destruct },
  { "getSourceEncoding", tg_10UConverter_getSourceEncoding },
  { "setSourceEncoding", tg_10UConverter_setSourceEncoding },
  { "getDestinationEncoding", tg_10UConverter_getDestinationEncoding },
  { "setDestinationEncoding", tg_10UConverter_setDestinationEncoding },
  { "getSourceType", tg_10UConverter_getSourceType },
  { "getDestinationType", tg_10UConverter_getDestinationType },
  { "getSubstChars", tg_10UConverter_getSubstChars },
  { "setSubstChars", tg_10UConverter_setSubstChars },
  { "fromUCallback", tg_10UConverter_fromUCallback },
  { "toUCallback", tg_10UConverter_toUCallback },
  { "convert", tg_10UConverter_convert },
  { "transcode", tg_10UConverter_transcode },
  { "getErrorCode", tg_10UConverter_getErrorCode },
  { "getErrorMessage", tg_10UConverter_getErrorMessage },
  { "reasonText", tg_10UConverter_reasonText },
  { "getAvailable", tg_10UConverter_getAvailable },
  { "getAliases", tg_10UConverter_getAliases },
  { "getStandards", tg_10UConverter_getStandards },
  { "getStandardName", tg_10UConverter_getStandardName },
  { "getMIMEName", tg_10UConverter_getMIMEName }
};

static const long long hhbc_ext_method_count_EncodingDetector = 5;
static const HhbcExtMethodInfo hhbc_ext_methods_EncodingDetector[] = {
  { "__construct", tg_16EncodingDetector___construct },
  { "setText", tg_16EncodingDetector_setText },
  { "setDeclaredEncoding", tg_16EncodingDetector_setDeclaredEncoding },
  { "detect", tg_16EncodingDetector_detect },
  { "detectAll", tg_16EncodingDetector_detectAll }
};

static const long long hhbc_ext_method_count_EncodingMatch = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_EncodingMatch[] = {
  { "__construct", tg_13EncodingMatch___construct },
  { "isValid", tg_13EncodingMatch_isValid },
  { "getEncoding", tg_13EncodingMatch_getEncoding },
  { "getConfidence", tg_13EncodingMatch_getConfidence },
  { "getLanguage", tg_13EncodingMatch_getLanguage },
  { "getUTF8", tg_13EncodingMatch_getUTF8 }
};

static const long long hhbc_ext_method_count_SpoofChecker = 5;
static const HhbcExtMethodInfo hhbc_ext_methods_SpoofChecker[] = {
  { "__construct", tg_12SpoofChecker___construct },
  { "isSuspicious", tg_12SpoofChecker_isSuspicious },
  { "areConfusable", tg_12SpoofChecker_areConfusable },
  { "setAllowedLocales", tg_12SpoofChecker_setAllowedLocales },
  { "setChecks", tg_12SpoofChecker_setChecks }
};

static const long long hhbc_ext_method_count_ImageSprite = 12;
static const HhbcExtMethodInfo hhbc_ext_methods_ImageSprite[] = {
  { "__construct", tg_11ImageSprite___construct },
  { "addFile", tg_11ImageSprite_addFile },
  { "addString", tg_11ImageSprite_addString },
  { "addUrl", tg_11ImageSprite_addUrl },
  { "clear", tg_11ImageSprite_clear },
  { "loadDims", tg_11ImageSprite_loadDims },
  { "loadImages", tg_11ImageSprite_loadImages },
  { "output", tg_11ImageSprite_output },
  { "css", tg_11ImageSprite_css },
  { "getErrors", tg_11ImageSprite_getErrors },
  { "mapping", tg_11ImageSprite_mapping },
  { "__destruct", tg_11ImageSprite___destruct }
};

static const long long hhbc_ext_method_count_Collator = 13;
static const HhbcExtMethodInfo hhbc_ext_methods_Collator[] = {
  { "__construct", tg_8Collator___construct },
  { "asort", tg_8Collator_asort },
  { "compare", tg_8Collator_compare },
  { "create", tg_8Collator_create },
  { "getattribute", tg_8Collator_getattribute },
  { "geterrorcode", tg_8Collator_geterrorcode },
  { "geterrormessage", tg_8Collator_geterrormessage },
  { "getlocale", tg_8Collator_getlocale },
  { "getstrength", tg_8Collator_getstrength },
  { "setattribute", tg_8Collator_setattribute },
  { "setstrength", tg_8Collator_setstrength },
  { "sortwithsortkeys", tg_8Collator_sortwithsortkeys },
  { "sort", tg_8Collator_sort }
};

static const long long hhbc_ext_method_count_Locale = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_Locale[] = {
  { "__construct", tg_6Locale___construct }
};

static const long long hhbc_ext_method_count_Normalizer = 3;
static const HhbcExtMethodInfo hhbc_ext_methods_Normalizer[] = {
  { "__construct", tg_10Normalizer___construct },
  { "isnormalized", tg_10Normalizer_isnormalized },
  { "normalize", tg_10Normalizer_normalize }
};

static const long long hhbc_ext_method_count_MutableArrayIterator = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_MutableArrayIterator[] = {
  { "__construct", tg_20MutableArrayIterator___construct },
  { "currentRef", tg_20MutableArrayIterator_currentRef },
  { "current", tg_20MutableArrayIterator_current },
  { "key", tg_20MutableArrayIterator_key },
  { "next", tg_20MutableArrayIterator_next },
  { "valid", tg_20MutableArrayIterator_valid }
};

static const long long hhbc_ext_method_count_Memcache = 21;
static const HhbcExtMethodInfo hhbc_ext_methods_Memcache[] = {
  { "__construct", tg_8Memcache___construct },
  { "connect", tg_8Memcache_connect },
  { "pconnect", tg_8Memcache_pconnect },
  { "add", tg_8Memcache_add },
  { "set", tg_8Memcache_set },
  { "replace", tg_8Memcache_replace },
  { "get", tg_8Memcache_get },
  { "delete", tg_8Memcache_delete },
  { "increment", tg_8Memcache_increment },
  { "decrement", tg_8Memcache_decrement },
  { "getversion", tg_8Memcache_getversion },
  { "flush", tg_8Memcache_flush },
  { "setoptimeout", tg_8Memcache_setoptimeout },
  { "close", tg_8Memcache_close },
  { "getserverstatus", tg_8Memcache_getserverstatus },
  { "setcompressthreshold", tg_8Memcache_setcompressthreshold },
  { "getstats", tg_8Memcache_getstats },
  { "getextendedstats", tg_8Memcache_getextendedstats },
  { "setserverparams", tg_8Memcache_setserverparams },
  { "addserver", tg_8Memcache_addserver },
  { "__destruct", tg_8Memcache___destruct }
};

static const long long hhbc_ext_method_count_Memcached = 38;
static const HhbcExtMethodInfo hhbc_ext_methods_Memcached[] = {
  { "__construct", tg_9Memcached___construct },
  { "add", tg_9Memcached_add },
  { "addByKey", tg_9Memcached_addByKey },
  { "addServer", tg_9Memcached_addServer },
  { "addServers", tg_9Memcached_addServers },
  { "append", tg_9Memcached_append },
  { "appendByKey", tg_9Memcached_appendByKey },
  { "cas", tg_9Memcached_cas },
  { "casByKey", tg_9Memcached_casByKey },
  { "decrement", tg_9Memcached_decrement },
  { "delete", tg_9Memcached_delete },
  { "deleteByKey", tg_9Memcached_deleteByKey },
  { "fetch", tg_9Memcached_fetch },
  { "fetchAll", tg_9Memcached_fetchAll },
  { "flush", tg_9Memcached_flush },
  { "get", tg_9Memcached_get },
  { "getByKey", tg_9Memcached_getByKey },
  { "getDelayed", tg_9Memcached_getDelayed },
  { "getDelayedByKey", tg_9Memcached_getDelayedByKey },
  { "getMulti", tg_9Memcached_getMulti },
  { "getMultiByKey", tg_9Memcached_getMultiByKey },
  { "getOption", tg_9Memcached_getOption },
  { "getResultCode", tg_9Memcached_getResultCode },
  { "getResultMessage", tg_9Memcached_getResultMessage },
  { "getServerByKey", tg_9Memcached_getServerByKey },
  { "getServerList", tg_9Memcached_getServerList },
  { "getStats", tg_9Memcached_getStats },
  { "getVersion", tg_9Memcached_getVersion },
  { "increment", tg_9Memcached_increment },
  { "prepend", tg_9Memcached_prepend },
  { "prependByKey", tg_9Memcached_prependByKey },
  { "replace", tg_9Memcached_replace },
  { "replaceByKey", tg_9Memcached_replaceByKey },
  { "set", tg_9Memcached_set },
  { "setByKey", tg_9Memcached_setByKey },
  { "setMulti", tg_9Memcached_setMulti },
  { "setMultiByKey", tg_9Memcached_setMultiByKey },
  { "setOption", tg_9Memcached_setOption }
};

static const long long hhbc_ext_method_count_PDO = 16;
static const HhbcExtMethodInfo hhbc_ext_methods_PDO[] = {
  { "__construct", tg_3PDO___construct },
  { "prepare", tg_3PDO_prepare },
  { "begintransaction", tg_3PDO_begintransaction },
  { "commit", tg_3PDO_commit },
  { "rollback", tg_3PDO_rollback },
  { "setattribute", tg_3PDO_setattribute },
  { "getattribute", tg_3PDO_getattribute },
  { "exec", tg_3PDO_exec },
  { "lastinsertid", tg_3PDO_lastinsertid },
  { "errorcode", tg_3PDO_errorcode },
  { "errorinfo", tg_3PDO_errorinfo },
  { "query", tg_3PDO_query },
  { "quote", tg_3PDO_quote },
  { "__wakeup", tg_3PDO___wakeup },
  { "__sleep", tg_3PDO___sleep },
  { "getavailabledrivers", tg_3PDO_getavailabledrivers }
};

static const long long hhbc_ext_method_count_PDOStatement = 27;
static const HhbcExtMethodInfo hhbc_ext_methods_PDOStatement[] = {
  { "__construct", tg_12PDOStatement___construct },
  { "execute", tg_12PDOStatement_execute },
  { "fetch", tg_12PDOStatement_fetch },
  { "fetchobject", tg_12PDOStatement_fetchobject },
  { "fetchcolumn", tg_12PDOStatement_fetchcolumn },
  { "fetchall", tg_12PDOStatement_fetchall },
  { "bindvalue", tg_12PDOStatement_bindvalue },
  { "bindparam", tg_12PDOStatement_bindparam },
  { "bindcolumn", tg_12PDOStatement_bindcolumn },
  { "rowcount", tg_12PDOStatement_rowcount },
  { "errorcode", tg_12PDOStatement_errorcode },
  { "errorinfo", tg_12PDOStatement_errorinfo },
  { "setattribute", tg_12PDOStatement_setattribute },
  { "getattribute", tg_12PDOStatement_getattribute },
  { "columncount", tg_12PDOStatement_columncount },
  { "getcolumnmeta", tg_12PDOStatement_getcolumnmeta },
  { "setfetchmode", tg_12PDOStatement_setfetchmode },
  { "nextrowset", tg_12PDOStatement_nextrowset },
  { "closecursor", tg_12PDOStatement_closecursor },
  { "debugdumpparams", tg_12PDOStatement_debugdumpparams },
  { "current", tg_12PDOStatement_current },
  { "key", tg_12PDOStatement_key },
  { "next", tg_12PDOStatement_next },
  { "rewind", tg_12PDOStatement_rewind },
  { "valid", tg_12PDOStatement_valid },
  { "__wakeup", tg_12PDOStatement___wakeup },
  { "__sleep", tg_12PDOStatement___sleep }
};

static const long long hhbc_ext_method_count_SimpleXMLElement = 22;
static const HhbcExtMethodInfo hhbc_ext_methods_SimpleXMLElement[] = {
  { "__construct", tg_16SimpleXMLElement___construct },
  { "offsetExists", tg_16SimpleXMLElement_offsetExists },
  { "offsetGet", tg_16SimpleXMLElement_offsetGet },
  { "offsetSet", tg_16SimpleXMLElement_offsetSet },
  { "offsetUnset", tg_16SimpleXMLElement_offsetUnset },
  { "getIterator", tg_16SimpleXMLElement_getIterator },
  { "count", tg_16SimpleXMLElement_count },
  { "xpath", tg_16SimpleXMLElement_xpath },
  { "registerXPathNamespace", tg_16SimpleXMLElement_registerXPathNamespace },
  { "asXML", tg_16SimpleXMLElement_asXML },
  { "getNamespaces", tg_16SimpleXMLElement_getNamespaces },
  { "getDocNamespaces", tg_16SimpleXMLElement_getDocNamespaces },
  { "children", tg_16SimpleXMLElement_children },
  { "getName", tg_16SimpleXMLElement_getName },
  { "attributes", tg_16SimpleXMLElement_attributes },
  { "addChild", tg_16SimpleXMLElement_addChild },
  { "addAttribute", tg_16SimpleXMLElement_addAttribute },
  { "__toString", tg_16SimpleXMLElement___toString },
  { "__get", tg_16SimpleXMLElement___get },
  { "__set", tg_16SimpleXMLElement___set },
  { "__isset", tg_16SimpleXMLElement___isset },
  { "__unset", tg_16SimpleXMLElement___unset }
};

static const long long hhbc_ext_method_count_LibXMLError = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_LibXMLError[] = {
  { "__construct", tg_11LibXMLError___construct }
};

static const long long hhbc_ext_method_count_SimpleXMLElementIterator = 6;
static const HhbcExtMethodInfo hhbc_ext_methods_SimpleXMLElementIterator[] = {
  { "__construct", tg_24SimpleXMLElementIterator___construct },
  { "current", tg_24SimpleXMLElementIterator_current },
  { "key", tg_24SimpleXMLElementIterator_key },
  { "next", tg_24SimpleXMLElementIterator_next },
  { "rewind", tg_24SimpleXMLElementIterator_rewind },
  { "valid", tg_24SimpleXMLElementIterator_valid }
};

static const long long hhbc_ext_method_count_SoapServer = 9;
static const HhbcExtMethodInfo hhbc_ext_methods_SoapServer[] = {
  { "__construct", tg_10SoapServer___construct },
  { "setclass", tg_10SoapServer_setclass },
  { "setobject", tg_10SoapServer_setobject },
  { "addfunction", tg_10SoapServer_addfunction },
  { "getfunctions", tg_10SoapServer_getfunctions },
  { "handle", tg_10SoapServer_handle },
  { "setpersistence", tg_10SoapServer_setpersistence },
  { "fault", tg_10SoapServer_fault },
  { "addsoapheader", tg_10SoapServer_addsoapheader }
};

static const long long hhbc_ext_method_count_SoapClient = 13;
static const HhbcExtMethodInfo hhbc_ext_methods_SoapClient[] = {
  { "__construct", tg_10SoapClient___construct },
  { "__call", tg_10SoapClient___call },
  { "__soapcall", tg_10SoapClient___soapcall },
  { "__getlastrequest", tg_10SoapClient___getlastrequest },
  { "__getlastresponse", tg_10SoapClient___getlastresponse },
  { "__getlastrequestheaders", tg_10SoapClient___getlastrequestheaders },
  { "__getlastresponseheaders", tg_10SoapClient___getlastresponseheaders },
  { "__getfunctions", tg_10SoapClient___getfunctions },
  { "__gettypes", tg_10SoapClient___gettypes },
  { "__dorequest", tg_10SoapClient___dorequest },
  { "__setcookie", tg_10SoapClient___setcookie },
  { "__setlocation", tg_10SoapClient___setlocation },
  { "__setsoapheaders", tg_10SoapClient___setsoapheaders }
};

static const long long hhbc_ext_method_count_SoapVar = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_SoapVar[] = {
  { "__construct", tg_7SoapVar___construct }
};

static const long long hhbc_ext_method_count_SoapParam = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_SoapParam[] = {
  { "__construct", tg_9SoapParam___construct }
};

static const long long hhbc_ext_method_count_SoapHeader = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_SoapHeader[] = {
  { "__construct", tg_10SoapHeader___construct }
};

static const long long hhbc_ext_method_count_SQLite3 = 18;
static const HhbcExtMethodInfo hhbc_ext_methods_SQLite3[] = {
  { "__construct", tg_7SQLite3___construct },
  { "open", tg_7SQLite3_open },
  { "busytimeout", tg_7SQLite3_busytimeout },
  { "close", tg_7SQLite3_close },
  { "exec", tg_7SQLite3_exec },
  { "version", tg_7SQLite3_version },
  { "lastinsertrowid", tg_7SQLite3_lastinsertrowid },
  { "lasterrorcode", tg_7SQLite3_lasterrorcode },
  { "lasterrormsg", tg_7SQLite3_lasterrormsg },
  { "loadextension", tg_7SQLite3_loadextension },
  { "changes", tg_7SQLite3_changes },
  { "escapestring", tg_7SQLite3_escapestring },
  { "prepare", tg_7SQLite3_prepare },
  { "query", tg_7SQLite3_query },
  { "querysingle", tg_7SQLite3_querysingle },
  { "createfunction", tg_7SQLite3_createfunction },
  { "createaggregate", tg_7SQLite3_createaggregate },
  { "openblob", tg_7SQLite3_openblob }
};

static const long long hhbc_ext_method_count_SQLite3Stmt = 8;
static const HhbcExtMethodInfo hhbc_ext_methods_SQLite3Stmt[] = {
  { "__construct", tg_11SQLite3Stmt___construct },
  { "paramcount", tg_11SQLite3Stmt_paramcount },
  { "close", tg_11SQLite3Stmt_close },
  { "reset", tg_11SQLite3Stmt_reset },
  { "clear", tg_11SQLite3Stmt_clear },
  { "bindparam", tg_11SQLite3Stmt_bindparam },
  { "bindvalue", tg_11SQLite3Stmt_bindvalue },
  { "execute", tg_11SQLite3Stmt_execute }
};

static const long long hhbc_ext_method_count_SQLite3Result = 7;
static const HhbcExtMethodInfo hhbc_ext_methods_SQLite3Result[] = {
  { "__construct", tg_13SQLite3Result___construct },
  { "numcolumns", tg_13SQLite3Result_numcolumns },
  { "columnname", tg_13SQLite3Result_columnname },
  { "columntype", tg_13SQLite3Result_columntype },
  { "fetcharray", tg_13SQLite3Result_fetcharray },
  { "reset", tg_13SQLite3Result_reset },
  { "finalize", tg_13SQLite3Result_finalize }
};

static const long long hhbc_ext_method_count_XMLReader = 27;
static const HhbcExtMethodInfo hhbc_ext_methods_XMLReader[] = {
  { "__construct", tg_9XMLReader___construct },
  { "open", tg_9XMLReader_open },
  { "XML", tg_9XMLReader_XML },
  { "close", tg_9XMLReader_close },
  { "read", tg_9XMLReader_read },
  { "next", tg_9XMLReader_next },
  { "readString", tg_9XMLReader_readString },
  { "readInnerXML", tg_9XMLReader_readInnerXML },
  { "readOuterXML", tg_9XMLReader_readOuterXML },
  { "moveToNextAttribute", tg_9XMLReader_moveToNextAttribute },
  { "getAttribute", tg_9XMLReader_getAttribute },
  { "getAttributeNo", tg_9XMLReader_getAttributeNo },
  { "getAttributeNs", tg_9XMLReader_getAttributeNs },
  { "moveToAttribute", tg_9XMLReader_moveToAttribute },
  { "moveToAttributeNo", tg_9XMLReader_moveToAttributeNo },
  { "moveToAttributeNs", tg_9XMLReader_moveToAttributeNs },
  { "moveToElement", tg_9XMLReader_moveToElement },
  { "moveToFirstAttribute", tg_9XMLReader_moveToFirstAttribute },
  { "isValid", tg_9XMLReader_isValid },
  { "expand", tg_9XMLReader_expand },
  { "__get", tg_9XMLReader___get },
  { "getParserProperty", tg_9XMLReader_getParserProperty },
  { "lookupNamespace", tg_9XMLReader_lookupNamespace },
  { "setSchema", tg_9XMLReader_setSchema },
  { "setParserProperty", tg_9XMLReader_setParserProperty },
  { "setRelaxNGSchema", tg_9XMLReader_setRelaxNGSchema },
  { "setRelaxNGSchemaSource", tg_9XMLReader_setRelaxNGSchemaSource }
};

static const long long hhbc_ext_method_count_XMLWriter = 43;
static const HhbcExtMethodInfo hhbc_ext_methods_XMLWriter[] = {
  { "__construct", tg_9XMLWriter___construct },
  { "openMemory", tg_9XMLWriter_openMemory },
  { "openURI", tg_9XMLWriter_openURI },
  { "setIndentString", tg_9XMLWriter_setIndentString },
  { "setIndent", tg_9XMLWriter_setIndent },
  { "startDocument", tg_9XMLWriter_startDocument },
  { "startElement", tg_9XMLWriter_startElement },
  { "startElementNS", tg_9XMLWriter_startElementNS },
  { "writeElementNS", tg_9XMLWriter_writeElementNS },
  { "writeElement", tg_9XMLWriter_writeElement },
  { "endElement", tg_9XMLWriter_endElement },
  { "fullEndElement", tg_9XMLWriter_fullEndElement },
  { "startAttributens", tg_9XMLWriter_startAttributens },
  { "startAttribute", tg_9XMLWriter_startAttribute },
  { "writeAttributeNS", tg_9XMLWriter_writeAttributeNS },
  { "writeAttribute", tg_9XMLWriter_writeAttribute },
  { "endAttribute", tg_9XMLWriter_endAttribute },
  { "startCData", tg_9XMLWriter_startCData },
  { "writeCData", tg_9XMLWriter_writeCData },
  { "endCData", tg_9XMLWriter_endCData },
  { "startComment", tg_9XMLWriter_startComment },
  { "writeComment", tg_9XMLWriter_writeComment },
  { "endComment", tg_9XMLWriter_endComment },
  { "endDocument", tg_9XMLWriter_endDocument },
  { "startPI", tg_9XMLWriter_startPI },
  { "writePI", tg_9XMLWriter_writePI },
  { "endPI", tg_9XMLWriter_endPI },
  { "text", tg_9XMLWriter_text },
  { "writeRaw", tg_9XMLWriter_writeRaw },
  { "startDTD", tg_9XMLWriter_startDTD },
  { "writeDTD", tg_9XMLWriter_writeDTD },
  { "startDTDElement", tg_9XMLWriter_startDTDElement },
  { "writeDTDElement", tg_9XMLWriter_writeDTDElement },
  { "endDTDElement", tg_9XMLWriter_endDTDElement },
  { "startDTDAttlist", tg_9XMLWriter_startDTDAttlist },
  { "writeDTDAttlist", tg_9XMLWriter_writeDTDAttlist },
  { "endDTDAttlist", tg_9XMLWriter_endDTDAttlist },
  { "startDTDEntity", tg_9XMLWriter_startDTDEntity },
  { "writeDTDEntity", tg_9XMLWriter_writeDTDEntity },
  { "endDTDEntity", tg_9XMLWriter_endDTDEntity },
  { "endDTD", tg_9XMLWriter_endDTD },
  { "flush", tg_9XMLWriter_flush },
  { "outputMemory", tg_9XMLWriter_outputMemory }
};

const long long hhbc_ext_class_count = 73;
const HhbcExtClassInfo hhbc_ext_classes[] = {
  { "WaitHandle", nullptr, sizeof(c_WaitHandle), hhbc_ext_method_count_WaitHandle, hhbc_ext_methods_WaitHandle, &c_WaitHandle::s_cls },
  { "StaticWaitHandle", nullptr, sizeof(c_StaticWaitHandle), hhbc_ext_method_count_StaticWaitHandle, hhbc_ext_methods_StaticWaitHandle, &c_StaticWaitHandle::s_cls },
  { "StaticResultWaitHandle", new_StaticResultWaitHandle_Instance, sizeof(c_StaticResultWaitHandle), hhbc_ext_method_count_StaticResultWaitHandle, hhbc_ext_methods_StaticResultWaitHandle, &c_StaticResultWaitHandle::s_cls },
  { "StaticExceptionWaitHandle", new_StaticExceptionWaitHandle_Instance, sizeof(c_StaticExceptionWaitHandle), hhbc_ext_method_count_StaticExceptionWaitHandle, hhbc_ext_methods_StaticExceptionWaitHandle, &c_StaticExceptionWaitHandle::s_cls },
  { "WaitableWaitHandle", nullptr, sizeof(c_WaitableWaitHandle), hhbc_ext_method_count_WaitableWaitHandle, hhbc_ext_methods_WaitableWaitHandle, &c_WaitableWaitHandle::s_cls },
  { "BlockableWaitHandle", nullptr, sizeof(c_BlockableWaitHandle), hhbc_ext_method_count_BlockableWaitHandle, hhbc_ext_methods_BlockableWaitHandle, &c_BlockableWaitHandle::s_cls },
  { "ContinuationWaitHandle", new_ContinuationWaitHandle_Instance, sizeof(c_ContinuationWaitHandle), hhbc_ext_method_count_ContinuationWaitHandle, hhbc_ext_methods_ContinuationWaitHandle, &c_ContinuationWaitHandle::s_cls },
  { "GenArrayWaitHandle", new_GenArrayWaitHandle_Instance, sizeof(c_GenArrayWaitHandle), hhbc_ext_method_count_GenArrayWaitHandle, hhbc_ext_methods_GenArrayWaitHandle, &c_GenArrayWaitHandle::s_cls },
  { "SetResultToRefWaitHandle", new_SetResultToRefWaitHandle_Instance, sizeof(c_SetResultToRefWaitHandle), hhbc_ext_method_count_SetResultToRefWaitHandle, hhbc_ext_methods_SetResultToRefWaitHandle, &c_SetResultToRefWaitHandle::s_cls },
  { "RescheduleWaitHandle", new_RescheduleWaitHandle_Instance, sizeof(c_RescheduleWaitHandle), hhbc_ext_method_count_RescheduleWaitHandle, hhbc_ext_methods_RescheduleWaitHandle, &c_RescheduleWaitHandle::s_cls },
  { "Closure", new_Closure_Instance, sizeof(c_Closure), hhbc_ext_method_count_Closure, hhbc_ext_methods_Closure, &c_Closure::s_cls },
  { "DummyClosure", new_DummyClosure_Instance, sizeof(c_DummyClosure), hhbc_ext_method_count_DummyClosure, hhbc_ext_methods_DummyClosure, &c_DummyClosure::s_cls },
  { "Vector", new_Vector_Instance, sizeof(c_Vector), hhbc_ext_method_count_Vector, hhbc_ext_methods_Vector, &c_Vector::s_cls },
  { "VectorIterator", new_VectorIterator_Instance, sizeof(c_VectorIterator), hhbc_ext_method_count_VectorIterator, hhbc_ext_methods_VectorIterator, &c_VectorIterator::s_cls },
  { "Map", new_Map_Instance, sizeof(c_Map), hhbc_ext_method_count_Map, hhbc_ext_methods_Map, &c_Map::s_cls },
  { "MapIterator", new_MapIterator_Instance, sizeof(c_MapIterator), hhbc_ext_method_count_MapIterator, hhbc_ext_methods_MapIterator, &c_MapIterator::s_cls },
  { "StableMap", new_StableMap_Instance, sizeof(c_StableMap), hhbc_ext_method_count_StableMap, hhbc_ext_methods_StableMap, &c_StableMap::s_cls },
  { "StableMapIterator", new_StableMapIterator_Instance, sizeof(c_StableMapIterator), hhbc_ext_method_count_StableMapIterator, hhbc_ext_methods_StableMapIterator, &c_StableMapIterator::s_cls },
  { "Pair", new_Pair_Instance, sizeof(c_Pair), hhbc_ext_method_count_Pair, hhbc_ext_methods_Pair, &c_Pair::s_cls },
  { "PairIterator", new_PairIterator_Instance, sizeof(c_PairIterator), hhbc_ext_method_count_PairIterator, hhbc_ext_methods_PairIterator, &c_PairIterator::s_cls },
  { "Continuation", new_Continuation_Instance, sizeof(c_Continuation), hhbc_ext_method_count_Continuation, hhbc_ext_methods_Continuation, &c_Continuation::s_cls },
  { "DummyContinuation", new_DummyContinuation_Instance, sizeof(c_DummyContinuation), hhbc_ext_method_count_DummyContinuation, hhbc_ext_methods_DummyContinuation, &c_DummyContinuation::s_cls },
  { "DateTime", new_DateTime_Instance, sizeof(c_DateTime), hhbc_ext_method_count_DateTime, hhbc_ext_methods_DateTime, &c_DateTime::s_cls },
  { "DateTimeZone", new_DateTimeZone_Instance, sizeof(c_DateTimeZone), hhbc_ext_method_count_DateTimeZone, hhbc_ext_methods_DateTimeZone, &c_DateTimeZone::s_cls },
  { "DateInterval", new_DateInterval_Instance, sizeof(c_DateInterval), hhbc_ext_method_count_DateInterval, hhbc_ext_methods_DateInterval, &c_DateInterval::s_cls },
  { "DebuggerProxyCmdUser", new_DebuggerProxyCmdUser_Instance, sizeof(c_DebuggerProxyCmdUser), hhbc_ext_method_count_DebuggerProxyCmdUser, hhbc_ext_methods_DebuggerProxyCmdUser, &c_DebuggerProxyCmdUser::s_cls },
  { "DebuggerClientCmdUser", new_DebuggerClientCmdUser_Instance, sizeof(c_DebuggerClientCmdUser), hhbc_ext_method_count_DebuggerClientCmdUser, hhbc_ext_methods_DebuggerClientCmdUser, &c_DebuggerClientCmdUser::s_cls },
  { "DebuggerClient", new_DebuggerClient_Instance, sizeof(c_DebuggerClient), hhbc_ext_method_count_DebuggerClient, hhbc_ext_methods_DebuggerClient, &c_DebuggerClient::s_cls },
  { "DOMNode", new_DOMNode_Instance, sizeof(c_DOMNode), hhbc_ext_method_count_DOMNode, hhbc_ext_methods_DOMNode, &c_DOMNode::s_cls },
  { "DOMAttr", new_DOMAttr_Instance, sizeof(c_DOMAttr), hhbc_ext_method_count_DOMAttr, hhbc_ext_methods_DOMAttr, &c_DOMAttr::s_cls },
  { "DOMCharacterData", new_DOMCharacterData_Instance, sizeof(c_DOMCharacterData), hhbc_ext_method_count_DOMCharacterData, hhbc_ext_methods_DOMCharacterData, &c_DOMCharacterData::s_cls },
  { "DOMComment", new_DOMComment_Instance, sizeof(c_DOMComment), hhbc_ext_method_count_DOMComment, hhbc_ext_methods_DOMComment, &c_DOMComment::s_cls },
  { "DOMText", new_DOMText_Instance, sizeof(c_DOMText), hhbc_ext_method_count_DOMText, hhbc_ext_methods_DOMText, &c_DOMText::s_cls },
  { "DOMCDATASection", new_DOMCDATASection_Instance, sizeof(c_DOMCDATASection), hhbc_ext_method_count_DOMCDATASection, hhbc_ext_methods_DOMCDATASection, &c_DOMCDATASection::s_cls },
  { "DOMDocument", new_DOMDocument_Instance, sizeof(c_DOMDocument), hhbc_ext_method_count_DOMDocument, hhbc_ext_methods_DOMDocument, &c_DOMDocument::s_cls },
  { "DOMDocumentFragment", new_DOMDocumentFragment_Instance, sizeof(c_DOMDocumentFragment), hhbc_ext_method_count_DOMDocumentFragment, hhbc_ext_methods_DOMDocumentFragment, &c_DOMDocumentFragment::s_cls },
  { "DOMDocumentType", new_DOMDocumentType_Instance, sizeof(c_DOMDocumentType), hhbc_ext_method_count_DOMDocumentType, hhbc_ext_methods_DOMDocumentType, &c_DOMDocumentType::s_cls },
  { "DOMElement", new_DOMElement_Instance, sizeof(c_DOMElement), hhbc_ext_method_count_DOMElement, hhbc_ext_methods_DOMElement, &c_DOMElement::s_cls },
  { "DOMEntity", new_DOMEntity_Instance, sizeof(c_DOMEntity), hhbc_ext_method_count_DOMEntity, hhbc_ext_methods_DOMEntity, &c_DOMEntity::s_cls },
  { "DOMEntityReference", new_DOMEntityReference_Instance, sizeof(c_DOMEntityReference), hhbc_ext_method_count_DOMEntityReference, hhbc_ext_methods_DOMEntityReference, &c_DOMEntityReference::s_cls },
  { "DOMNotation", new_DOMNotation_Instance, sizeof(c_DOMNotation), hhbc_ext_method_count_DOMNotation, hhbc_ext_methods_DOMNotation, &c_DOMNotation::s_cls },
  { "DOMProcessingInstruction", new_DOMProcessingInstruction_Instance, sizeof(c_DOMProcessingInstruction), hhbc_ext_method_count_DOMProcessingInstruction, hhbc_ext_methods_DOMProcessingInstruction, &c_DOMProcessingInstruction::s_cls },
  { "DOMNodeIterator", new_DOMNodeIterator_Instance, sizeof(c_DOMNodeIterator), hhbc_ext_method_count_DOMNodeIterator, hhbc_ext_methods_DOMNodeIterator, &c_DOMNodeIterator::s_cls },
  { "DOMNamedNodeMap", new_DOMNamedNodeMap_Instance, sizeof(c_DOMNamedNodeMap), hhbc_ext_method_count_DOMNamedNodeMap, hhbc_ext_methods_DOMNamedNodeMap, &c_DOMNamedNodeMap::s_cls },
  { "DOMNodeList", new_DOMNodeList_Instance, sizeof(c_DOMNodeList), hhbc_ext_method_count_DOMNodeList, hhbc_ext_methods_DOMNodeList, &c_DOMNodeList::s_cls },
  { "DOMImplementation", new_DOMImplementation_Instance, sizeof(c_DOMImplementation), hhbc_ext_method_count_DOMImplementation, hhbc_ext_methods_DOMImplementation, &c_DOMImplementation::s_cls },
  { "DOMXPath", new_DOMXPath_Instance, sizeof(c_DOMXPath), hhbc_ext_method_count_DOMXPath, hhbc_ext_methods_DOMXPath, &c_DOMXPath::s_cls },
  { "UConverter", new_UConverter_Instance, sizeof(c_UConverter), hhbc_ext_method_count_UConverter, hhbc_ext_methods_UConverter, &c_UConverter::s_cls },
  { "EncodingDetector", new_EncodingDetector_Instance, sizeof(c_EncodingDetector), hhbc_ext_method_count_EncodingDetector, hhbc_ext_methods_EncodingDetector, &c_EncodingDetector::s_cls },
  { "EncodingMatch", new_EncodingMatch_Instance, sizeof(c_EncodingMatch), hhbc_ext_method_count_EncodingMatch, hhbc_ext_methods_EncodingMatch, &c_EncodingMatch::s_cls },
  { "SpoofChecker", new_SpoofChecker_Instance, sizeof(c_SpoofChecker), hhbc_ext_method_count_SpoofChecker, hhbc_ext_methods_SpoofChecker, &c_SpoofChecker::s_cls },
  { "ImageSprite", new_ImageSprite_Instance, sizeof(c_ImageSprite), hhbc_ext_method_count_ImageSprite, hhbc_ext_methods_ImageSprite, &c_ImageSprite::s_cls },
  { "Collator", new_Collator_Instance, sizeof(c_Collator), hhbc_ext_method_count_Collator, hhbc_ext_methods_Collator, &c_Collator::s_cls },
  { "Locale", new_Locale_Instance, sizeof(c_Locale), hhbc_ext_method_count_Locale, hhbc_ext_methods_Locale, &c_Locale::s_cls },
  { "Normalizer", new_Normalizer_Instance, sizeof(c_Normalizer), hhbc_ext_method_count_Normalizer, hhbc_ext_methods_Normalizer, &c_Normalizer::s_cls },
  { "MutableArrayIterator", new_MutableArrayIterator_Instance, sizeof(c_MutableArrayIterator), hhbc_ext_method_count_MutableArrayIterator, hhbc_ext_methods_MutableArrayIterator, &c_MutableArrayIterator::s_cls },
  { "Memcache", new_Memcache_Instance, sizeof(c_Memcache), hhbc_ext_method_count_Memcache, hhbc_ext_methods_Memcache, &c_Memcache::s_cls },
  { "Memcached", new_Memcached_Instance, sizeof(c_Memcached), hhbc_ext_method_count_Memcached, hhbc_ext_methods_Memcached, &c_Memcached::s_cls },
  { "PDO", new_PDO_Instance, sizeof(c_PDO), hhbc_ext_method_count_PDO, hhbc_ext_methods_PDO, &c_PDO::s_cls },
  { "PDOStatement", new_PDOStatement_Instance, sizeof(c_PDOStatement), hhbc_ext_method_count_PDOStatement, hhbc_ext_methods_PDOStatement, &c_PDOStatement::s_cls },
  { "SimpleXMLElement", new_SimpleXMLElement_Instance, sizeof(c_SimpleXMLElement), hhbc_ext_method_count_SimpleXMLElement, hhbc_ext_methods_SimpleXMLElement, &c_SimpleXMLElement::s_cls },
  { "LibXMLError", new_LibXMLError_Instance, sizeof(c_LibXMLError), hhbc_ext_method_count_LibXMLError, hhbc_ext_methods_LibXMLError, &c_LibXMLError::s_cls },
  { "SimpleXMLElementIterator", new_SimpleXMLElementIterator_Instance, sizeof(c_SimpleXMLElementIterator), hhbc_ext_method_count_SimpleXMLElementIterator, hhbc_ext_methods_SimpleXMLElementIterator, &c_SimpleXMLElementIterator::s_cls },
  { "SoapServer", new_SoapServer_Instance, sizeof(c_SoapServer), hhbc_ext_method_count_SoapServer, hhbc_ext_methods_SoapServer, &c_SoapServer::s_cls },
  { "SoapClient", new_SoapClient_Instance, sizeof(c_SoapClient), hhbc_ext_method_count_SoapClient, hhbc_ext_methods_SoapClient, &c_SoapClient::s_cls },
  { "SoapVar", new_SoapVar_Instance, sizeof(c_SoapVar), hhbc_ext_method_count_SoapVar, hhbc_ext_methods_SoapVar, &c_SoapVar::s_cls },
  { "SoapParam", new_SoapParam_Instance, sizeof(c_SoapParam), hhbc_ext_method_count_SoapParam, hhbc_ext_methods_SoapParam, &c_SoapParam::s_cls },
  { "SoapHeader", new_SoapHeader_Instance, sizeof(c_SoapHeader), hhbc_ext_method_count_SoapHeader, hhbc_ext_methods_SoapHeader, &c_SoapHeader::s_cls },
  { "SQLite3", new_SQLite3_Instance, sizeof(c_SQLite3), hhbc_ext_method_count_SQLite3, hhbc_ext_methods_SQLite3, &c_SQLite3::s_cls },
  { "SQLite3Stmt", new_SQLite3Stmt_Instance, sizeof(c_SQLite3Stmt), hhbc_ext_method_count_SQLite3Stmt, hhbc_ext_methods_SQLite3Stmt, &c_SQLite3Stmt::s_cls },
  { "SQLite3Result", new_SQLite3Result_Instance, sizeof(c_SQLite3Result), hhbc_ext_method_count_SQLite3Result, hhbc_ext_methods_SQLite3Result, &c_SQLite3Result::s_cls },
  { "XMLReader", new_XMLReader_Instance, sizeof(c_XMLReader), hhbc_ext_method_count_XMLReader, hhbc_ext_methods_XMLReader, &c_XMLReader::s_cls },
  { "XMLWriter", new_XMLWriter_Instance, sizeof(c_XMLWriter), hhbc_ext_method_count_XMLWriter, hhbc_ext_methods_XMLWriter, &c_XMLWriter::s_cls }
};

} // namespace HPHP
