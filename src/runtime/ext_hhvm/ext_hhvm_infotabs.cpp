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
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/ext/ext.h>
namespace HPHP {
  struct TypedValue;
  namespace VM { struct ActRec; struct Class; }
}

namespace HPHP {

TypedValue* fg_apache_note(VM::ActRec *ar);
TypedValue* fg_apache_request_headers(VM::ActRec *ar);
TypedValue* fg_apache_response_headers(VM::ActRec *ar);
TypedValue* fg_apache_setenv(VM::ActRec *ar);
TypedValue* fg_getallheaders(VM::ActRec *ar);
TypedValue* fg_virtual(VM::ActRec *ar);
TypedValue* fg_apache_get_config(VM::ActRec *ar);
TypedValue* fg_apache_get_scoreboard(VM::ActRec *ar);
TypedValue* fg_apache_get_rewrite_rules(VM::ActRec *ar);
TypedValue* fg_apc_add(VM::ActRec *ar);
TypedValue* fg_apc_store(VM::ActRec *ar);
TypedValue* fg_apc_fetch(VM::ActRec *ar);
TypedValue* fg_apc_delete(VM::ActRec *ar);
TypedValue* fg_apc_compile_file(VM::ActRec *ar);
TypedValue* fg_apc_cache_info(VM::ActRec *ar);
TypedValue* fg_apc_clear_cache(VM::ActRec *ar);
TypedValue* fg_apc_define_constants(VM::ActRec *ar);
TypedValue* fg_apc_load_constants(VM::ActRec *ar);
TypedValue* fg_apc_sma_info(VM::ActRec *ar);
TypedValue* fg_apc_filehits(VM::ActRec *ar);
TypedValue* fg_apc_delete_file(VM::ActRec *ar);
TypedValue* fg_apc_inc(VM::ActRec *ar);
TypedValue* fg_apc_dec(VM::ActRec *ar);
TypedValue* fg_apc_cas(VM::ActRec *ar);
TypedValue* fg_apc_exists(VM::ActRec *ar);
TypedValue* fg_apc_bin_dump(VM::ActRec *ar);
TypedValue* fg_apc_bin_load(VM::ActRec *ar);
TypedValue* fg_apc_bin_dumpfile(VM::ActRec *ar);
TypedValue* fg_apc_bin_loadfile(VM::ActRec *ar);
TypedValue* fg_override_function(VM::ActRec *ar);
TypedValue* fg_rename_function(VM::ActRec *ar);
TypedValue* fg_apd_set_browser_trace(VM::ActRec *ar);
TypedValue* fg_apd_set_pprof_trace(VM::ActRec *ar);
TypedValue* fg_apd_set_session_trace_socket(VM::ActRec *ar);
TypedValue* fg_apd_stop_trace(VM::ActRec *ar);
TypedValue* fg_apd_breakpoint(VM::ActRec *ar);
TypedValue* fg_apd_continue(VM::ActRec *ar);
TypedValue* fg_apd_echo(VM::ActRec *ar);
TypedValue* fg_array_change_key_case(VM::ActRec *ar);
TypedValue* fg_array_chunk(VM::ActRec *ar);
TypedValue* fg_array_combine(VM::ActRec *ar);
TypedValue* fg_array_count_values(VM::ActRec *ar);
TypedValue* fg_array_fill_keys(VM::ActRec *ar);
TypedValue* fg_array_fill(VM::ActRec *ar);
TypedValue* fg_array_filter(VM::ActRec *ar);
TypedValue* fg_array_flip(VM::ActRec *ar);
TypedValue* fg_array_key_exists(VM::ActRec *ar);
TypedValue* fg_key_exists(VM::ActRec *ar);
TypedValue* fg_array_keys(VM::ActRec *ar);
TypedValue* fg_array_map(VM::ActRec *ar);
TypedValue* fg_array_merge_recursive(VM::ActRec *ar);
TypedValue* fg_array_merge(VM::ActRec *ar);
TypedValue* fg_array_replace_recursive(VM::ActRec *ar);
TypedValue* fg_array_replace(VM::ActRec *ar);
TypedValue* fg_array_multisort(VM::ActRec *ar);
TypedValue* fg_array_pad(VM::ActRec *ar);
TypedValue* fg_array_pop(VM::ActRec *ar);
TypedValue* fg_array_product(VM::ActRec *ar);
TypedValue* fg_array_push(VM::ActRec *ar);
TypedValue* fg_array_rand(VM::ActRec *ar);
TypedValue* fg_array_reduce(VM::ActRec *ar);
TypedValue* fg_array_reverse(VM::ActRec *ar);
TypedValue* fg_array_search(VM::ActRec *ar);
TypedValue* fg_array_shift(VM::ActRec *ar);
TypedValue* fg_array_slice(VM::ActRec *ar);
TypedValue* fg_array_splice(VM::ActRec *ar);
TypedValue* fg_array_sum(VM::ActRec *ar);
TypedValue* fg_array_unique(VM::ActRec *ar);
TypedValue* fg_array_unshift(VM::ActRec *ar);
TypedValue* fg_array_values(VM::ActRec *ar);
TypedValue* fg_array_walk_recursive(VM::ActRec *ar);
TypedValue* fg_array_walk(VM::ActRec *ar);
TypedValue* fg_compact(VM::ActRec *ar);
TypedValue* fg_shuffle(VM::ActRec *ar);
TypedValue* fg_count(VM::ActRec *ar);
TypedValue* fg_sizeof(VM::ActRec *ar);
TypedValue* fg_each(VM::ActRec *ar);
TypedValue* fg_current(VM::ActRec *ar);
TypedValue* fg_hphp_current_ref(VM::ActRec *ar);
TypedValue* fg_next(VM::ActRec *ar);
TypedValue* fg_pos(VM::ActRec *ar);
TypedValue* fg_prev(VM::ActRec *ar);
TypedValue* fg_reset(VM::ActRec *ar);
TypedValue* fg_end(VM::ActRec *ar);
TypedValue* fg_key(VM::ActRec *ar);
TypedValue* fg_hphp_get_iterator(VM::ActRec *ar);
TypedValue* fg_hphp_get_mutable_iterator(VM::ActRec *ar);
TypedValue* fg_in_array(VM::ActRec *ar);
TypedValue* fg_range(VM::ActRec *ar);
TypedValue* fg_array_diff(VM::ActRec *ar);
TypedValue* fg_array_udiff(VM::ActRec *ar);
TypedValue* fg_array_diff_assoc(VM::ActRec *ar);
TypedValue* fg_array_diff_uassoc(VM::ActRec *ar);
TypedValue* fg_array_udiff_assoc(VM::ActRec *ar);
TypedValue* fg_array_udiff_uassoc(VM::ActRec *ar);
TypedValue* fg_array_diff_key(VM::ActRec *ar);
TypedValue* fg_array_diff_ukey(VM::ActRec *ar);
TypedValue* fg_array_intersect(VM::ActRec *ar);
TypedValue* fg_array_uintersect(VM::ActRec *ar);
TypedValue* fg_array_intersect_assoc(VM::ActRec *ar);
TypedValue* fg_array_intersect_uassoc(VM::ActRec *ar);
TypedValue* fg_array_uintersect_assoc(VM::ActRec *ar);
TypedValue* fg_array_uintersect_uassoc(VM::ActRec *ar);
TypedValue* fg_array_intersect_key(VM::ActRec *ar);
TypedValue* fg_array_intersect_ukey(VM::ActRec *ar);
TypedValue* fg_sort(VM::ActRec *ar);
TypedValue* fg_rsort(VM::ActRec *ar);
TypedValue* fg_asort(VM::ActRec *ar);
TypedValue* fg_arsort(VM::ActRec *ar);
TypedValue* fg_ksort(VM::ActRec *ar);
TypedValue* fg_krsort(VM::ActRec *ar);
TypedValue* fg_usort(VM::ActRec *ar);
TypedValue* fg_uasort(VM::ActRec *ar);
TypedValue* fg_uksort(VM::ActRec *ar);
TypedValue* fg_natsort(VM::ActRec *ar);
TypedValue* fg_natcasesort(VM::ActRec *ar);
TypedValue* fg_i18n_loc_get_default(VM::ActRec *ar);
TypedValue* fg_i18n_loc_set_default(VM::ActRec *ar);
TypedValue* fg_i18n_loc_set_attribute(VM::ActRec *ar);
TypedValue* fg_i18n_loc_set_strength(VM::ActRec *ar);
TypedValue* fg_i18n_loc_get_error_code(VM::ActRec *ar);
TypedValue* fg_bcscale(VM::ActRec *ar);
TypedValue* fg_bcadd(VM::ActRec *ar);
TypedValue* fg_bcsub(VM::ActRec *ar);
TypedValue* fg_bccomp(VM::ActRec *ar);
TypedValue* fg_bcmul(VM::ActRec *ar);
TypedValue* fg_bcdiv(VM::ActRec *ar);
TypedValue* fg_bcmod(VM::ActRec *ar);
TypedValue* fg_bcpow(VM::ActRec *ar);
TypedValue* fg_bcpowmod(VM::ActRec *ar);
TypedValue* fg_bcsqrt(VM::ActRec *ar);
TypedValue* fg_bzclose(VM::ActRec *ar);
TypedValue* fg_bzopen(VM::ActRec *ar);
TypedValue* fg_bzread(VM::ActRec *ar);
TypedValue* fg_bzwrite(VM::ActRec *ar);
TypedValue* fg_bzflush(VM::ActRec *ar);
TypedValue* fg_bzerrstr(VM::ActRec *ar);
TypedValue* fg_bzerror(VM::ActRec *ar);
TypedValue* fg_bzerrno(VM::ActRec *ar);
TypedValue* fg_bzcompress(VM::ActRec *ar);
TypedValue* fg_bzdecompress(VM::ActRec *ar);
TypedValue* fg_get_declared_classes(VM::ActRec *ar);
TypedValue* fg_get_declared_interfaces(VM::ActRec *ar);
TypedValue* fg_get_declared_traits(VM::ActRec *ar);
TypedValue* fg_class_exists(VM::ActRec *ar);
TypedValue* fg_interface_exists(VM::ActRec *ar);
TypedValue* fg_trait_exists(VM::ActRec *ar);
TypedValue* fg_get_class_methods(VM::ActRec *ar);
TypedValue* fg_get_class_vars(VM::ActRec *ar);
TypedValue* fg_get_class_constants(VM::ActRec *ar);
TypedValue* fg_get_class(VM::ActRec *ar);
TypedValue* fg_get_parent_class(VM::ActRec *ar);
TypedValue* fg_is_a(VM::ActRec *ar);
TypedValue* fg_is_subclass_of(VM::ActRec *ar);
TypedValue* fg_method_exists(VM::ActRec *ar);
TypedValue* fg_property_exists(VM::ActRec *ar);
TypedValue* fg_get_object_vars(VM::ActRec *ar);
TypedValue* fg_call_user_method_array(VM::ActRec *ar);
TypedValue* fg_call_user_method(VM::ActRec *ar);
TypedValue* fg_hphp_create_continuation(VM::ActRec *ar);
TypedValue* fg_hphp_pack_continuation(VM::ActRec *ar);
TypedValue* fg_hphp_unpack_continuation(VM::ActRec *ar);
TypedValue* fg_ctype_alnum(VM::ActRec *ar);
TypedValue* fg_ctype_alpha(VM::ActRec *ar);
TypedValue* fg_ctype_cntrl(VM::ActRec *ar);
TypedValue* fg_ctype_digit(VM::ActRec *ar);
TypedValue* fg_ctype_graph(VM::ActRec *ar);
TypedValue* fg_ctype_lower(VM::ActRec *ar);
TypedValue* fg_ctype_print(VM::ActRec *ar);
TypedValue* fg_ctype_punct(VM::ActRec *ar);
TypedValue* fg_ctype_space(VM::ActRec *ar);
TypedValue* fg_ctype_upper(VM::ActRec *ar);
TypedValue* fg_ctype_xdigit(VM::ActRec *ar);
TypedValue* fg_curl_init(VM::ActRec *ar);
TypedValue* fg_curl_copy_handle(VM::ActRec *ar);
TypedValue* fg_curl_version(VM::ActRec *ar);
TypedValue* fg_curl_setopt(VM::ActRec *ar);
TypedValue* fg_curl_setopt_array(VM::ActRec *ar);
TypedValue* fg_fb_curl_getopt(VM::ActRec *ar);
TypedValue* fg_curl_exec(VM::ActRec *ar);
TypedValue* fg_curl_getinfo(VM::ActRec *ar);
TypedValue* fg_curl_errno(VM::ActRec *ar);
TypedValue* fg_curl_error(VM::ActRec *ar);
TypedValue* fg_curl_close(VM::ActRec *ar);
TypedValue* fg_curl_multi_init(VM::ActRec *ar);
TypedValue* fg_curl_multi_add_handle(VM::ActRec *ar);
TypedValue* fg_curl_multi_remove_handle(VM::ActRec *ar);
TypedValue* fg_curl_multi_exec(VM::ActRec *ar);
TypedValue* fg_curl_multi_select(VM::ActRec *ar);
TypedValue* fg_fb_curl_multi_fdset(VM::ActRec *ar);
TypedValue* fg_curl_multi_getcontent(VM::ActRec *ar);
TypedValue* fg_curl_multi_info_read(VM::ActRec *ar);
TypedValue* fg_curl_multi_close(VM::ActRec *ar);
TypedValue* fg_evhttp_set_cache(VM::ActRec *ar);
TypedValue* fg_evhttp_get(VM::ActRec *ar);
TypedValue* fg_evhttp_post(VM::ActRec *ar);
TypedValue* fg_evhttp_async_get(VM::ActRec *ar);
TypedValue* fg_evhttp_async_post(VM::ActRec *ar);
TypedValue* fg_evhttp_recv(VM::ActRec *ar);
TypedValue* fg_checkdate(VM::ActRec *ar);
TypedValue* fg_date_add(VM::ActRec *ar);
TypedValue* fg_date_create_from_format(VM::ActRec *ar);
TypedValue* fg_date_create(VM::ActRec *ar);
TypedValue* fg_date_date_set(VM::ActRec *ar);
TypedValue* fg_date_default_timezone_get(VM::ActRec *ar);
TypedValue* fg_date_default_timezone_set(VM::ActRec *ar);
TypedValue* fg_date_diff(VM::ActRec *ar);
TypedValue* fg_date_format(VM::ActRec *ar);
TypedValue* fg_date_get_last_errors(VM::ActRec *ar);
TypedValue* fg_date_interval_create_from_date_string(VM::ActRec *ar);
TypedValue* fg_date_interval_format(VM::ActRec *ar);
TypedValue* fg_date_isodate_set(VM::ActRec *ar);
TypedValue* fg_date_modify(VM::ActRec *ar);
TypedValue* fg_date_offset_get(VM::ActRec *ar);
TypedValue* fg_date_parse(VM::ActRec *ar);
TypedValue* fg_date_sub(VM::ActRec *ar);
TypedValue* fg_date_sun_info(VM::ActRec *ar);
TypedValue* fg_date_sunrise(VM::ActRec *ar);
TypedValue* fg_date_sunset(VM::ActRec *ar);
TypedValue* fg_date_time_set(VM::ActRec *ar);
TypedValue* fg_date_timestamp_get(VM::ActRec *ar);
TypedValue* fg_date_timestamp_set(VM::ActRec *ar);
TypedValue* fg_date_timezone_get(VM::ActRec *ar);
TypedValue* fg_date_timezone_set(VM::ActRec *ar);
TypedValue* fg_date(VM::ActRec *ar);
TypedValue* fg_getdate(VM::ActRec *ar);
TypedValue* fg_gettimeofday(VM::ActRec *ar);
TypedValue* fg_gmdate(VM::ActRec *ar);
TypedValue* fg_gmmktime(VM::ActRec *ar);
TypedValue* fg_gmstrftime(VM::ActRec *ar);
TypedValue* fg_idate(VM::ActRec *ar);
TypedValue* fg_localtime(VM::ActRec *ar);
TypedValue* fg_microtime(VM::ActRec *ar);
TypedValue* fg_mktime(VM::ActRec *ar);
TypedValue* fg_strftime(VM::ActRec *ar);
TypedValue* fg_strptime(VM::ActRec *ar);
TypedValue* fg_strtotime(VM::ActRec *ar);
TypedValue* fg_time(VM::ActRec *ar);
TypedValue* fg_timezone_abbreviations_list(VM::ActRec *ar);
TypedValue* fg_timezone_identifiers_list(VM::ActRec *ar);
TypedValue* fg_timezone_location_get(VM::ActRec *ar);
TypedValue* fg_timezone_name_from_abbr(VM::ActRec *ar);
TypedValue* fg_timezone_name_get(VM::ActRec *ar);
TypedValue* fg_timezone_offset_get(VM::ActRec *ar);
TypedValue* fg_timezone_open(VM::ActRec *ar);
TypedValue* fg_timezone_transitions_get(VM::ActRec *ar);
TypedValue* fg_timezone_version_get(VM::ActRec *ar);
TypedValue* fg_hphpd_install_user_command(VM::ActRec *ar);
TypedValue* fg_hphpd_get_user_commands(VM::ActRec *ar);
TypedValue* fg_hphpd_break(VM::ActRec *ar);
TypedValue* fg_hphpd_get_client(VM::ActRec *ar);
TypedValue* fg_hphpd_client_ctrl(VM::ActRec *ar);
TypedValue* fg_dom_document_create_element(VM::ActRec *ar);
TypedValue* fg_dom_document_create_document_fragment(VM::ActRec *ar);
TypedValue* fg_dom_document_create_text_node(VM::ActRec *ar);
TypedValue* fg_dom_document_create_comment(VM::ActRec *ar);
TypedValue* fg_dom_document_create_cdatasection(VM::ActRec *ar);
TypedValue* fg_dom_document_create_processing_instruction(VM::ActRec *ar);
TypedValue* fg_dom_document_create_attribute(VM::ActRec *ar);
TypedValue* fg_dom_document_create_entity_reference(VM::ActRec *ar);
TypedValue* fg_dom_document_get_elements_by_tag_name(VM::ActRec *ar);
TypedValue* fg_dom_document_import_node(VM::ActRec *ar);
TypedValue* fg_dom_document_create_element_ns(VM::ActRec *ar);
TypedValue* fg_dom_document_create_attribute_ns(VM::ActRec *ar);
TypedValue* fg_dom_document_get_elements_by_tag_name_ns(VM::ActRec *ar);
TypedValue* fg_dom_document_get_element_by_id(VM::ActRec *ar);
TypedValue* fg_dom_document_normalize_document(VM::ActRec *ar);
TypedValue* fg_dom_document_save(VM::ActRec *ar);
TypedValue* fg_dom_document_savexml(VM::ActRec *ar);
TypedValue* fg_dom_document_validate(VM::ActRec *ar);
TypedValue* fg_dom_document_xinclude(VM::ActRec *ar);
TypedValue* fg_dom_document_save_html(VM::ActRec *ar);
TypedValue* fg_dom_document_save_html_file(VM::ActRec *ar);
TypedValue* fg_dom_document_schema_validate_file(VM::ActRec *ar);
TypedValue* fg_dom_document_schema_validate_xml(VM::ActRec *ar);
TypedValue* fg_dom_document_relaxng_validate_file(VM::ActRec *ar);
TypedValue* fg_dom_document_relaxng_validate_xml(VM::ActRec *ar);
TypedValue* fg_dom_node_insert_before(VM::ActRec *ar);
TypedValue* fg_dom_node_replace_child(VM::ActRec *ar);
TypedValue* fg_dom_node_remove_child(VM::ActRec *ar);
TypedValue* fg_dom_node_append_child(VM::ActRec *ar);
TypedValue* fg_dom_node_has_child_nodes(VM::ActRec *ar);
TypedValue* fg_dom_node_clone_node(VM::ActRec *ar);
TypedValue* fg_dom_node_normalize(VM::ActRec *ar);
TypedValue* fg_dom_node_is_supported(VM::ActRec *ar);
TypedValue* fg_dom_node_has_attributes(VM::ActRec *ar);
TypedValue* fg_dom_node_is_same_node(VM::ActRec *ar);
TypedValue* fg_dom_node_lookup_prefix(VM::ActRec *ar);
TypedValue* fg_dom_node_is_default_namespace(VM::ActRec *ar);
TypedValue* fg_dom_node_lookup_namespace_uri(VM::ActRec *ar);
TypedValue* fg_dom_nodelist_item(VM::ActRec *ar);
TypedValue* fg_dom_namednodemap_get_named_item(VM::ActRec *ar);
TypedValue* fg_dom_namednodemap_item(VM::ActRec *ar);
TypedValue* fg_dom_namednodemap_get_named_item_ns(VM::ActRec *ar);
TypedValue* fg_dom_characterdata_substring_data(VM::ActRec *ar);
TypedValue* fg_dom_characterdata_append_data(VM::ActRec *ar);
TypedValue* fg_dom_characterdata_insert_data(VM::ActRec *ar);
TypedValue* fg_dom_characterdata_delete_data(VM::ActRec *ar);
TypedValue* fg_dom_characterdata_replace_data(VM::ActRec *ar);
TypedValue* fg_dom_attr_is_id(VM::ActRec *ar);
TypedValue* fg_dom_element_get_attribute(VM::ActRec *ar);
TypedValue* fg_dom_element_set_attribute(VM::ActRec *ar);
TypedValue* fg_dom_element_remove_attribute(VM::ActRec *ar);
TypedValue* fg_dom_element_get_attribute_node(VM::ActRec *ar);
TypedValue* fg_dom_element_set_attribute_node(VM::ActRec *ar);
TypedValue* fg_dom_element_remove_attribute_node(VM::ActRec *ar);
TypedValue* fg_dom_element_get_elements_by_tag_name(VM::ActRec *ar);
TypedValue* fg_dom_element_get_attribute_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_set_attribute_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_remove_attribute_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_get_attribute_node_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_set_attribute_node_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_get_elements_by_tag_name_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_has_attribute(VM::ActRec *ar);
TypedValue* fg_dom_element_has_attribute_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_set_id_attribute(VM::ActRec *ar);
TypedValue* fg_dom_element_set_id_attribute_ns(VM::ActRec *ar);
TypedValue* fg_dom_element_set_id_attribute_node(VM::ActRec *ar);
TypedValue* fg_dom_text_split_text(VM::ActRec *ar);
TypedValue* fg_dom_text_is_whitespace_in_element_content(VM::ActRec *ar);
TypedValue* fg_dom_xpath_register_ns(VM::ActRec *ar);
TypedValue* fg_dom_xpath_query(VM::ActRec *ar);
TypedValue* fg_dom_xpath_evaluate(VM::ActRec *ar);
TypedValue* fg_dom_xpath_register_php_functions(VM::ActRec *ar);
TypedValue* fg_debug_backtrace(VM::ActRec *ar);
TypedValue* fg_debug_print_backtrace(VM::ActRec *ar);
TypedValue* fg_error_get_last(VM::ActRec *ar);
TypedValue* fg_error_log(VM::ActRec *ar);
TypedValue* fg_error_reporting(VM::ActRec *ar);
TypedValue* fg_restore_error_handler(VM::ActRec *ar);
TypedValue* fg_restore_exception_handler(VM::ActRec *ar);
TypedValue* fg_set_error_handler(VM::ActRec *ar);
TypedValue* fg_set_exception_handler(VM::ActRec *ar);
TypedValue* fg_hphp_set_error_page(VM::ActRec *ar);
TypedValue* fg_hphp_throw_fatal_error(VM::ActRec *ar);
TypedValue* fg_hphp_clear_unflushed(VM::ActRec *ar);
TypedValue* fg_hphp_debug_caller_info(VM::ActRec *ar);
TypedValue* fg_trigger_error(VM::ActRec *ar);
TypedValue* fg_user_error(VM::ActRec *ar);
TypedValue* fg_fb_thrift_serialize(VM::ActRec *ar);
TypedValue* fg_fb_thrift_unserialize(VM::ActRec *ar);
TypedValue* fg_fb_serialize(VM::ActRec *ar);
TypedValue* fg_fb_unserialize(VM::ActRec *ar);
TypedValue* fg_fb_compact_serialize(VM::ActRec *ar);
TypedValue* fg_fb_compact_unserialize(VM::ActRec *ar);
TypedValue* fg_fb_could_include(VM::ActRec *ar);
TypedValue* fg_fb_intercept(VM::ActRec *ar);
TypedValue* fg_fb_stubout_intercept_handler(VM::ActRec *ar);
TypedValue* fg_fb_rpc_intercept_handler(VM::ActRec *ar);
TypedValue* fg_fb_renamed_functions(VM::ActRec *ar);
TypedValue* fg_fb_rename_function(VM::ActRec *ar);
TypedValue* fg_fb_utf8ize(VM::ActRec *ar);
TypedValue* fg_fb_utf8_strlen_deprecated(VM::ActRec *ar);
TypedValue* fg_fb_utf8_strlen(VM::ActRec *ar);
TypedValue* fg_fb_utf8_substr(VM::ActRec *ar);
TypedValue* fg_fb_call_user_func_safe(VM::ActRec *ar);
TypedValue* fg_fb_call_user_func_safe_return(VM::ActRec *ar);
TypedValue* fg_fb_call_user_func_array_safe(VM::ActRec *ar);
TypedValue* fg_fb_get_code_coverage(VM::ActRec *ar);
TypedValue* fg_fb_enable_code_coverage(VM::ActRec *ar);
TypedValue* fg_fb_disable_code_coverage(VM::ActRec *ar);
TypedValue* fg_xhprof_enable(VM::ActRec *ar);
TypedValue* fg_xhprof_disable(VM::ActRec *ar);
TypedValue* fg_xhprof_network_enable(VM::ActRec *ar);
TypedValue* fg_xhprof_network_disable(VM::ActRec *ar);
TypedValue* fg_xhprof_frame_begin(VM::ActRec *ar);
TypedValue* fg_xhprof_frame_end(VM::ActRec *ar);
TypedValue* fg_xhprof_run_trace(VM::ActRec *ar);
TypedValue* fg_xhprof_sample_enable(VM::ActRec *ar);
TypedValue* fg_xhprof_sample_disable(VM::ActRec *ar);
TypedValue* fg_fb_load_local_databases(VM::ActRec *ar);
TypedValue* fg_fb_parallel_query(VM::ActRec *ar);
TypedValue* fg_fb_crossall_query(VM::ActRec *ar);
TypedValue* fg_fb_set_taint(VM::ActRec *ar);
TypedValue* fg_fb_unset_taint(VM::ActRec *ar);
TypedValue* fg_fb_get_taint(VM::ActRec *ar);
TypedValue* fg_fb_get_taint_warning_counts(VM::ActRec *ar);
TypedValue* fg_fb_enable_html_taint_trace(VM::ActRec *ar);
TypedValue* fg_fb_const_fetch(VM::ActRec *ar);
TypedValue* fg_fb_output_compression(VM::ActRec *ar);
TypedValue* fg_fb_set_exit_callback(VM::ActRec *ar);
TypedValue* fg_fb_get_flush_stat(VM::ActRec *ar);
TypedValue* fg_fb_get_last_flush_size(VM::ActRec *ar);
TypedValue* fg_fb_lazy_stat(VM::ActRec *ar);
TypedValue* fg_fb_lazy_lstat(VM::ActRec *ar);
TypedValue* fg_fb_lazy_realpath(VM::ActRec *ar);
TypedValue* fg_fb_setprofile(VM::ActRec *ar);
TypedValue* fg_fb_gc_collect_cycles(VM::ActRec *ar);
TypedValue* fg_fb_gc_detect_cycles(VM::ActRec *ar);
TypedValue* fg_fopen(VM::ActRec *ar);
TypedValue* fg_popen(VM::ActRec *ar);
TypedValue* fg_fclose(VM::ActRec *ar);
TypedValue* fg_pclose(VM::ActRec *ar);
TypedValue* fg_fseek(VM::ActRec *ar);
TypedValue* fg_rewind(VM::ActRec *ar);
TypedValue* fg_ftell(VM::ActRec *ar);
TypedValue* fg_feof(VM::ActRec *ar);
TypedValue* fg_fstat(VM::ActRec *ar);
TypedValue* fg_fread(VM::ActRec *ar);
TypedValue* fg_fgetc(VM::ActRec *ar);
TypedValue* fg_fgets(VM::ActRec *ar);
TypedValue* fg_fgetss(VM::ActRec *ar);
TypedValue* fg_fscanf(VM::ActRec *ar);
TypedValue* fg_fpassthru(VM::ActRec *ar);
TypedValue* fg_fwrite(VM::ActRec *ar);
TypedValue* fg_fputs(VM::ActRec *ar);
TypedValue* fg_fprintf(VM::ActRec *ar);
TypedValue* fg_vfprintf(VM::ActRec *ar);
TypedValue* fg_fflush(VM::ActRec *ar);
TypedValue* fg_ftruncate(VM::ActRec *ar);
TypedValue* fg_flock(VM::ActRec *ar);
TypedValue* fg_fputcsv(VM::ActRec *ar);
TypedValue* fg_fgetcsv(VM::ActRec *ar);
TypedValue* fg_file_get_contents(VM::ActRec *ar);
TypedValue* fg_file_put_contents(VM::ActRec *ar);
TypedValue* fg_file(VM::ActRec *ar);
TypedValue* fg_readfile(VM::ActRec *ar);
TypedValue* fg_move_uploaded_file(VM::ActRec *ar);
TypedValue* fg_parse_ini_file(VM::ActRec *ar);
TypedValue* fg_parse_ini_string(VM::ActRec *ar);
TypedValue* fg_parse_hdf_file(VM::ActRec *ar);
TypedValue* fg_parse_hdf_string(VM::ActRec *ar);
TypedValue* fg_write_hdf_file(VM::ActRec *ar);
TypedValue* fg_write_hdf_string(VM::ActRec *ar);
TypedValue* fg_md5_file(VM::ActRec *ar);
TypedValue* fg_sha1_file(VM::ActRec *ar);
TypedValue* fg_chmod(VM::ActRec *ar);
TypedValue* fg_chown(VM::ActRec *ar);
TypedValue* fg_lchown(VM::ActRec *ar);
TypedValue* fg_chgrp(VM::ActRec *ar);
TypedValue* fg_lchgrp(VM::ActRec *ar);
TypedValue* fg_touch(VM::ActRec *ar);
TypedValue* fg_copy(VM::ActRec *ar);
TypedValue* fg_rename(VM::ActRec *ar);
TypedValue* fg_umask(VM::ActRec *ar);
TypedValue* fg_unlink(VM::ActRec *ar);
TypedValue* fg_link(VM::ActRec *ar);
TypedValue* fg_symlink(VM::ActRec *ar);
TypedValue* fg_basename(VM::ActRec *ar);
TypedValue* fg_fnmatch(VM::ActRec *ar);
TypedValue* fg_glob(VM::ActRec *ar);
TypedValue* fg_tempnam(VM::ActRec *ar);
TypedValue* fg_tmpfile(VM::ActRec *ar);
TypedValue* fg_fileperms(VM::ActRec *ar);
TypedValue* fg_fileinode(VM::ActRec *ar);
TypedValue* fg_filesize(VM::ActRec *ar);
TypedValue* fg_fileowner(VM::ActRec *ar);
TypedValue* fg_filegroup(VM::ActRec *ar);
TypedValue* fg_fileatime(VM::ActRec *ar);
TypedValue* fg_filemtime(VM::ActRec *ar);
TypedValue* fg_filectime(VM::ActRec *ar);
TypedValue* fg_filetype(VM::ActRec *ar);
TypedValue* fg_linkinfo(VM::ActRec *ar);
TypedValue* fg_is_writable(VM::ActRec *ar);
TypedValue* fg_is_writeable(VM::ActRec *ar);
TypedValue* fg_is_readable(VM::ActRec *ar);
TypedValue* fg_is_executable(VM::ActRec *ar);
TypedValue* fg_is_file(VM::ActRec *ar);
TypedValue* fg_is_dir(VM::ActRec *ar);
TypedValue* fg_is_link(VM::ActRec *ar);
TypedValue* fg_is_uploaded_file(VM::ActRec *ar);
TypedValue* fg_file_exists(VM::ActRec *ar);
TypedValue* fg_stat(VM::ActRec *ar);
TypedValue* fg_lstat(VM::ActRec *ar);
TypedValue* fg_clearstatcache(VM::ActRec *ar);
TypedValue* fg_readlink(VM::ActRec *ar);
TypedValue* fg_realpath(VM::ActRec *ar);
TypedValue* fg_pathinfo(VM::ActRec *ar);
TypedValue* fg_disk_free_space(VM::ActRec *ar);
TypedValue* fg_diskfreespace(VM::ActRec *ar);
TypedValue* fg_disk_total_space(VM::ActRec *ar);
TypedValue* fg_mkdir(VM::ActRec *ar);
TypedValue* fg_rmdir(VM::ActRec *ar);
TypedValue* fg_dirname(VM::ActRec *ar);
TypedValue* fg_getcwd(VM::ActRec *ar);
TypedValue* fg_chdir(VM::ActRec *ar);
TypedValue* fg_chroot(VM::ActRec *ar);
TypedValue* fg_dir(VM::ActRec *ar);
TypedValue* fg_opendir(VM::ActRec *ar);
TypedValue* fg_readdir(VM::ActRec *ar);
TypedValue* fg_rewinddir(VM::ActRec *ar);
TypedValue* fg_scandir(VM::ActRec *ar);
TypedValue* fg_closedir(VM::ActRec *ar);
TypedValue* fg_get_defined_functions(VM::ActRec *ar);
TypedValue* fg_function_exists(VM::ActRec *ar);
TypedValue* fg_is_callable(VM::ActRec *ar);
TypedValue* fg_call_user_func_array(VM::ActRec *ar);
TypedValue* fg_call_user_func(VM::ActRec *ar);
TypedValue* fg_call_user_func_array_async(VM::ActRec *ar);
TypedValue* fg_call_user_func_async(VM::ActRec *ar);
TypedValue* fg_check_user_func_async(VM::ActRec *ar);
TypedValue* fg_end_user_func_async(VM::ActRec *ar);
TypedValue* fg_call_user_func_serialized(VM::ActRec *ar);
TypedValue* fg_call_user_func_array_rpc(VM::ActRec *ar);
TypedValue* fg_call_user_func_rpc(VM::ActRec *ar);
TypedValue* fg_forward_static_call_array(VM::ActRec *ar);
TypedValue* fg_forward_static_call(VM::ActRec *ar);
TypedValue* fg_get_called_class(VM::ActRec *ar);
TypedValue* fg_create_function(VM::ActRec *ar);
TypedValue* fg_func_get_arg(VM::ActRec *ar);
TypedValue* fg_func_get_args(VM::ActRec *ar);
TypedValue* fg_func_num_args(VM::ActRec *ar);
TypedValue* fg_register_postsend_function(VM::ActRec *ar);
TypedValue* fg_register_shutdown_function(VM::ActRec *ar);
TypedValue* fg_register_cleanup_function(VM::ActRec *ar);
TypedValue* fg_register_tick_function(VM::ActRec *ar);
TypedValue* fg_unregister_tick_function(VM::ActRec *ar);
TypedValue* fg_hash(VM::ActRec *ar);
TypedValue* fg_hash_algos(VM::ActRec *ar);
TypedValue* fg_hash_init(VM::ActRec *ar);
TypedValue* fg_hash_file(VM::ActRec *ar);
TypedValue* fg_hash_final(VM::ActRec *ar);
TypedValue* fg_hash_hmac_file(VM::ActRec *ar);
TypedValue* fg_hash_hmac(VM::ActRec *ar);
TypedValue* fg_hash_update_file(VM::ActRec *ar);
TypedValue* fg_hash_update_stream(VM::ActRec *ar);
TypedValue* fg_hash_update(VM::ActRec *ar);
TypedValue* fg_furchash_hphp_ext(VM::ActRec *ar);
TypedValue* fg_furchash_hphp_ext_supported(VM::ActRec *ar);
TypedValue* fg_hphp_murmurhash(VM::ActRec *ar);
TypedValue* fg_iconv_mime_encode(VM::ActRec *ar);
TypedValue* fg_iconv_mime_decode(VM::ActRec *ar);
TypedValue* fg_iconv_mime_decode_headers(VM::ActRec *ar);
TypedValue* fg_iconv_get_encoding(VM::ActRec *ar);
TypedValue* fg_iconv_set_encoding(VM::ActRec *ar);
TypedValue* fg_iconv(VM::ActRec *ar);
TypedValue* fg_iconv_strlen(VM::ActRec *ar);
TypedValue* fg_iconv_strpos(VM::ActRec *ar);
TypedValue* fg_iconv_strrpos(VM::ActRec *ar);
TypedValue* fg_iconv_substr(VM::ActRec *ar);
TypedValue* fg_ob_iconv_handler(VM::ActRec *ar);
TypedValue* fg_icu_transliterate(VM::ActRec *ar);
TypedValue* fg_icu_tokenize(VM::ActRec *ar);
TypedValue* fg_gd_info(VM::ActRec *ar);
TypedValue* fg_getimagesize(VM::ActRec *ar);
TypedValue* fg_image_type_to_extension(VM::ActRec *ar);
TypedValue* fg_image_type_to_mime_type(VM::ActRec *ar);
TypedValue* fg_image2wbmp(VM::ActRec *ar);
TypedValue* fg_imagealphablending(VM::ActRec *ar);
TypedValue* fg_imageantialias(VM::ActRec *ar);
TypedValue* fg_imagearc(VM::ActRec *ar);
TypedValue* fg_imagechar(VM::ActRec *ar);
TypedValue* fg_imagecharup(VM::ActRec *ar);
TypedValue* fg_imagecolorallocate(VM::ActRec *ar);
TypedValue* fg_imagecolorallocatealpha(VM::ActRec *ar);
TypedValue* fg_imagecolorat(VM::ActRec *ar);
TypedValue* fg_imagecolorclosest(VM::ActRec *ar);
TypedValue* fg_imagecolorclosestalpha(VM::ActRec *ar);
TypedValue* fg_imagecolorclosesthwb(VM::ActRec *ar);
TypedValue* fg_imagecolordeallocate(VM::ActRec *ar);
TypedValue* fg_imagecolorexact(VM::ActRec *ar);
TypedValue* fg_imagecolorexactalpha(VM::ActRec *ar);
TypedValue* fg_imagecolormatch(VM::ActRec *ar);
TypedValue* fg_imagecolorresolve(VM::ActRec *ar);
TypedValue* fg_imagecolorresolvealpha(VM::ActRec *ar);
TypedValue* fg_imagecolorset(VM::ActRec *ar);
TypedValue* fg_imagecolorsforindex(VM::ActRec *ar);
TypedValue* fg_imagecolorstotal(VM::ActRec *ar);
TypedValue* fg_imagecolortransparent(VM::ActRec *ar);
TypedValue* fg_imageconvolution(VM::ActRec *ar);
TypedValue* fg_imagecopy(VM::ActRec *ar);
TypedValue* fg_imagecopymerge(VM::ActRec *ar);
TypedValue* fg_imagecopymergegray(VM::ActRec *ar);
TypedValue* fg_imagecopyresampled(VM::ActRec *ar);
TypedValue* fg_imagecopyresized(VM::ActRec *ar);
TypedValue* fg_imagecreate(VM::ActRec *ar);
TypedValue* fg_imagecreatefromgd2part(VM::ActRec *ar);
TypedValue* fg_imagecreatefromgd(VM::ActRec *ar);
TypedValue* fg_imagecreatefromgd2(VM::ActRec *ar);
TypedValue* fg_imagecreatefromgif(VM::ActRec *ar);
TypedValue* fg_imagecreatefromjpeg(VM::ActRec *ar);
TypedValue* fg_imagecreatefrompng(VM::ActRec *ar);
TypedValue* fg_imagecreatefromstring(VM::ActRec *ar);
TypedValue* fg_imagecreatefromwbmp(VM::ActRec *ar);
TypedValue* fg_imagecreatefromxbm(VM::ActRec *ar);
TypedValue* fg_imagecreatefromxpm(VM::ActRec *ar);
TypedValue* fg_imagecreatetruecolor(VM::ActRec *ar);
TypedValue* fg_imagedashedline(VM::ActRec *ar);
TypedValue* fg_imagedestroy(VM::ActRec *ar);
TypedValue* fg_imageellipse(VM::ActRec *ar);
TypedValue* fg_imagefill(VM::ActRec *ar);
TypedValue* fg_imagefilledarc(VM::ActRec *ar);
TypedValue* fg_imagefilledellipse(VM::ActRec *ar);
TypedValue* fg_imagefilledpolygon(VM::ActRec *ar);
TypedValue* fg_imagefilledrectangle(VM::ActRec *ar);
TypedValue* fg_imagefilltoborder(VM::ActRec *ar);
TypedValue* fg_imagefilter(VM::ActRec *ar);
TypedValue* fg_imagefontheight(VM::ActRec *ar);
TypedValue* fg_imagefontwidth(VM::ActRec *ar);
TypedValue* fg_imageftbbox(VM::ActRec *ar);
TypedValue* fg_imagefttext(VM::ActRec *ar);
TypedValue* fg_imagegammacorrect(VM::ActRec *ar);
TypedValue* fg_imagegd2(VM::ActRec *ar);
TypedValue* fg_imagegd(VM::ActRec *ar);
TypedValue* fg_imagegif(VM::ActRec *ar);
TypedValue* fg_imagegrabscreen(VM::ActRec *ar);
TypedValue* fg_imagegrabwindow(VM::ActRec *ar);
TypedValue* fg_imageinterlace(VM::ActRec *ar);
TypedValue* fg_imageistruecolor(VM::ActRec *ar);
TypedValue* fg_imagejpeg(VM::ActRec *ar);
TypedValue* fg_imagelayereffect(VM::ActRec *ar);
TypedValue* fg_imageline(VM::ActRec *ar);
TypedValue* fg_imageloadfont(VM::ActRec *ar);
TypedValue* fg_imagepalettecopy(VM::ActRec *ar);
TypedValue* fg_imagepng(VM::ActRec *ar);
TypedValue* fg_imagepolygon(VM::ActRec *ar);
TypedValue* fg_imagepsbbox(VM::ActRec *ar);
TypedValue* fg_imagepsencodefont(VM::ActRec *ar);
TypedValue* fg_imagepsextendfont(VM::ActRec *ar);
TypedValue* fg_imagepsfreefont(VM::ActRec *ar);
TypedValue* fg_imagepsloadfont(VM::ActRec *ar);
TypedValue* fg_imagepsslantfont(VM::ActRec *ar);
TypedValue* fg_imagepstext(VM::ActRec *ar);
TypedValue* fg_imagerectangle(VM::ActRec *ar);
TypedValue* fg_imagerotate(VM::ActRec *ar);
TypedValue* fg_imagesavealpha(VM::ActRec *ar);
TypedValue* fg_imagesetbrush(VM::ActRec *ar);
TypedValue* fg_imagesetpixel(VM::ActRec *ar);
TypedValue* fg_imagesetstyle(VM::ActRec *ar);
TypedValue* fg_imagesetthickness(VM::ActRec *ar);
TypedValue* fg_imagesettile(VM::ActRec *ar);
TypedValue* fg_imagestring(VM::ActRec *ar);
TypedValue* fg_imagestringup(VM::ActRec *ar);
TypedValue* fg_imagesx(VM::ActRec *ar);
TypedValue* fg_imagesy(VM::ActRec *ar);
TypedValue* fg_imagetruecolortopalette(VM::ActRec *ar);
TypedValue* fg_imagettfbbox(VM::ActRec *ar);
TypedValue* fg_imagettftext(VM::ActRec *ar);
TypedValue* fg_imagetypes(VM::ActRec *ar);
TypedValue* fg_imagewbmp(VM::ActRec *ar);
TypedValue* fg_imagexbm(VM::ActRec *ar);
TypedValue* fg_iptcembed(VM::ActRec *ar);
TypedValue* fg_iptcparse(VM::ActRec *ar);
TypedValue* fg_jpeg2wbmp(VM::ActRec *ar);
TypedValue* fg_png2wbmp(VM::ActRec *ar);
TypedValue* fg_exif_imagetype(VM::ActRec *ar);
TypedValue* fg_exif_read_data(VM::ActRec *ar);
TypedValue* fg_read_exif_data(VM::ActRec *ar);
TypedValue* fg_exif_tagname(VM::ActRec *ar);
TypedValue* fg_exif_thumbnail(VM::ActRec *ar);
TypedValue* fg_imap_8bit(VM::ActRec *ar);
TypedValue* fg_imap_alerts(VM::ActRec *ar);
TypedValue* fg_imap_append(VM::ActRec *ar);
TypedValue* fg_imap_base64(VM::ActRec *ar);
TypedValue* fg_imap_binary(VM::ActRec *ar);
TypedValue* fg_imap_body(VM::ActRec *ar);
TypedValue* fg_imap_bodystruct(VM::ActRec *ar);
TypedValue* fg_imap_check(VM::ActRec *ar);
TypedValue* fg_imap_clearflag_full(VM::ActRec *ar);
TypedValue* fg_imap_close(VM::ActRec *ar);
TypedValue* fg_imap_createmailbox(VM::ActRec *ar);
TypedValue* fg_imap_delete(VM::ActRec *ar);
TypedValue* fg_imap_deletemailbox(VM::ActRec *ar);
TypedValue* fg_imap_errors(VM::ActRec *ar);
TypedValue* fg_imap_expunge(VM::ActRec *ar);
TypedValue* fg_imap_fetch_overview(VM::ActRec *ar);
TypedValue* fg_imap_fetchbody(VM::ActRec *ar);
TypedValue* fg_imap_fetchheader(VM::ActRec *ar);
TypedValue* fg_imap_fetchstructure(VM::ActRec *ar);
TypedValue* fg_imap_gc(VM::ActRec *ar);
TypedValue* fg_imap_get_quota(VM::ActRec *ar);
TypedValue* fg_imap_get_quotaroot(VM::ActRec *ar);
TypedValue* fg_imap_getacl(VM::ActRec *ar);
TypedValue* fg_imap_getmailboxes(VM::ActRec *ar);
TypedValue* fg_imap_getsubscribed(VM::ActRec *ar);
TypedValue* fg_imap_header(VM::ActRec *ar);
TypedValue* fg_imap_headerinfo(VM::ActRec *ar);
TypedValue* fg_imap_headers(VM::ActRec *ar);
TypedValue* fg_imap_last_error(VM::ActRec *ar);
TypedValue* fg_imap_list(VM::ActRec *ar);
TypedValue* fg_imap_listmailbox(VM::ActRec *ar);
TypedValue* fg_imap_listscan(VM::ActRec *ar);
TypedValue* fg_imap_listsubscribed(VM::ActRec *ar);
TypedValue* fg_imap_lsub(VM::ActRec *ar);
TypedValue* fg_imap_mail_compose(VM::ActRec *ar);
TypedValue* fg_imap_mail_copy(VM::ActRec *ar);
TypedValue* fg_imap_mail_move(VM::ActRec *ar);
TypedValue* fg_imap_mail(VM::ActRec *ar);
TypedValue* fg_imap_mailboxmsginfo(VM::ActRec *ar);
TypedValue* fg_imap_mime_header_decode(VM::ActRec *ar);
TypedValue* fg_imap_msgno(VM::ActRec *ar);
TypedValue* fg_imap_num_msg(VM::ActRec *ar);
TypedValue* fg_imap_num_recent(VM::ActRec *ar);
TypedValue* fg_imap_open(VM::ActRec *ar);
TypedValue* fg_imap_ping(VM::ActRec *ar);
TypedValue* fg_imap_qprint(VM::ActRec *ar);
TypedValue* fg_imap_renamemailbox(VM::ActRec *ar);
TypedValue* fg_imap_reopen(VM::ActRec *ar);
TypedValue* fg_imap_rfc822_parse_adrlist(VM::ActRec *ar);
TypedValue* fg_imap_rfc822_parse_headers(VM::ActRec *ar);
TypedValue* fg_imap_rfc822_write_address(VM::ActRec *ar);
TypedValue* fg_imap_savebody(VM::ActRec *ar);
TypedValue* fg_imap_scanmailbox(VM::ActRec *ar);
TypedValue* fg_imap_search(VM::ActRec *ar);
TypedValue* fg_imap_set_quota(VM::ActRec *ar);
TypedValue* fg_imap_setacl(VM::ActRec *ar);
TypedValue* fg_imap_setflag_full(VM::ActRec *ar);
TypedValue* fg_imap_sort(VM::ActRec *ar);
TypedValue* fg_imap_status(VM::ActRec *ar);
TypedValue* fg_imap_subscribe(VM::ActRec *ar);
TypedValue* fg_imap_thread(VM::ActRec *ar);
TypedValue* fg_imap_timeout(VM::ActRec *ar);
TypedValue* fg_imap_uid(VM::ActRec *ar);
TypedValue* fg_imap_undelete(VM::ActRec *ar);
TypedValue* fg_imap_unsubscribe(VM::ActRec *ar);
TypedValue* fg_imap_utf7_decode(VM::ActRec *ar);
TypedValue* fg_imap_utf7_encode(VM::ActRec *ar);
TypedValue* fg_imap_utf8(VM::ActRec *ar);
TypedValue* fg_intl_get_error_code(VM::ActRec *ar);
TypedValue* fg_intl_get_error_message(VM::ActRec *ar);
TypedValue* fg_intl_error_name(VM::ActRec *ar);
TypedValue* fg_intl_is_failure(VM::ActRec *ar);
TypedValue* fg_collator_asort(VM::ActRec *ar);
TypedValue* fg_collator_compare(VM::ActRec *ar);
TypedValue* fg_collator_create(VM::ActRec *ar);
TypedValue* fg_collator_get_attribute(VM::ActRec *ar);
TypedValue* fg_collator_get_error_code(VM::ActRec *ar);
TypedValue* fg_collator_get_error_message(VM::ActRec *ar);
TypedValue* fg_collator_get_locale(VM::ActRec *ar);
TypedValue* fg_collator_get_strength(VM::ActRec *ar);
TypedValue* fg_collator_set_attribute(VM::ActRec *ar);
TypedValue* fg_collator_set_strength(VM::ActRec *ar);
TypedValue* fg_collator_sort_with_sort_keys(VM::ActRec *ar);
TypedValue* fg_collator_sort(VM::ActRec *ar);
TypedValue* fg_idn_to_ascii(VM::ActRec *ar);
TypedValue* fg_idn_to_unicode(VM::ActRec *ar);
TypedValue* fg_idn_to_utf8(VM::ActRec *ar);
TypedValue* fg_ftok(VM::ActRec *ar);
TypedValue* fg_msg_get_queue(VM::ActRec *ar);
TypedValue* fg_msg_queue_exists(VM::ActRec *ar);
TypedValue* fg_msg_send(VM::ActRec *ar);
TypedValue* fg_msg_receive(VM::ActRec *ar);
TypedValue* fg_msg_remove_queue(VM::ActRec *ar);
TypedValue* fg_msg_set_queue(VM::ActRec *ar);
TypedValue* fg_msg_stat_queue(VM::ActRec *ar);
TypedValue* fg_sem_acquire(VM::ActRec *ar);
TypedValue* fg_sem_get(VM::ActRec *ar);
TypedValue* fg_sem_release(VM::ActRec *ar);
TypedValue* fg_sem_remove(VM::ActRec *ar);
TypedValue* fg_shm_attach(VM::ActRec *ar);
TypedValue* fg_shm_detach(VM::ActRec *ar);
TypedValue* fg_shm_remove(VM::ActRec *ar);
TypedValue* fg_shm_get_var(VM::ActRec *ar);
TypedValue* fg_shm_has_var(VM::ActRec *ar);
TypedValue* fg_shm_put_var(VM::ActRec *ar);
TypedValue* fg_shm_remove_var(VM::ActRec *ar);
TypedValue* fg_hphp_recursiveiteratoriterator___construct(VM::ActRec *ar);
TypedValue* fg_hphp_recursiveiteratoriterator_getinneriterator(VM::ActRec *ar);
TypedValue* fg_hphp_recursiveiteratoriterator_current(VM::ActRec *ar);
TypedValue* fg_hphp_recursiveiteratoriterator_key(VM::ActRec *ar);
TypedValue* fg_hphp_recursiveiteratoriterator_next(VM::ActRec *ar);
TypedValue* fg_hphp_recursiveiteratoriterator_rewind(VM::ActRec *ar);
TypedValue* fg_hphp_recursiveiteratoriterator_valid(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator___construct(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator_key(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator_next(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator_rewind(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator_seek(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator_current(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator___tostring(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator_valid(VM::ActRec *ar);
TypedValue* fg_hphp_directoryiterator_isdot(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator___construct(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_key(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_next(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_rewind(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_seek(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_current(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator___tostring(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_valid(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_haschildren(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_getchildren(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_getsubpath(VM::ActRec *ar);
TypedValue* fg_hphp_recursivedirectoryiterator_getsubpathname(VM::ActRec *ar);
TypedValue* fg_json_encode(VM::ActRec *ar);
TypedValue* fg_json_decode(VM::ActRec *ar);
TypedValue* fg_ldap_connect(VM::ActRec *ar);
TypedValue* fg_ldap_explode_dn(VM::ActRec *ar);
TypedValue* fg_ldap_dn2ufn(VM::ActRec *ar);
TypedValue* fg_ldap_err2str(VM::ActRec *ar);
TypedValue* fg_ldap_add(VM::ActRec *ar);
TypedValue* fg_ldap_mod_add(VM::ActRec *ar);
TypedValue* fg_ldap_mod_del(VM::ActRec *ar);
TypedValue* fg_ldap_mod_replace(VM::ActRec *ar);
TypedValue* fg_ldap_modify(VM::ActRec *ar);
TypedValue* fg_ldap_bind(VM::ActRec *ar);
TypedValue* fg_ldap_set_rebind_proc(VM::ActRec *ar);
TypedValue* fg_ldap_sort(VM::ActRec *ar);
TypedValue* fg_ldap_start_tls(VM::ActRec *ar);
TypedValue* fg_ldap_unbind(VM::ActRec *ar);
TypedValue* fg_ldap_get_option(VM::ActRec *ar);
TypedValue* fg_ldap_set_option(VM::ActRec *ar);
TypedValue* fg_ldap_close(VM::ActRec *ar);
TypedValue* fg_ldap_list(VM::ActRec *ar);
TypedValue* fg_ldap_read(VM::ActRec *ar);
TypedValue* fg_ldap_search(VM::ActRec *ar);
TypedValue* fg_ldap_rename(VM::ActRec *ar);
TypedValue* fg_ldap_delete(VM::ActRec *ar);
TypedValue* fg_ldap_compare(VM::ActRec *ar);
TypedValue* fg_ldap_errno(VM::ActRec *ar);
TypedValue* fg_ldap_error(VM::ActRec *ar);
TypedValue* fg_ldap_get_dn(VM::ActRec *ar);
TypedValue* fg_ldap_count_entries(VM::ActRec *ar);
TypedValue* fg_ldap_get_entries(VM::ActRec *ar);
TypedValue* fg_ldap_first_entry(VM::ActRec *ar);
TypedValue* fg_ldap_next_entry(VM::ActRec *ar);
TypedValue* fg_ldap_get_attributes(VM::ActRec *ar);
TypedValue* fg_ldap_first_attribute(VM::ActRec *ar);
TypedValue* fg_ldap_next_attribute(VM::ActRec *ar);
TypedValue* fg_ldap_first_reference(VM::ActRec *ar);
TypedValue* fg_ldap_next_reference(VM::ActRec *ar);
TypedValue* fg_ldap_parse_reference(VM::ActRec *ar);
TypedValue* fg_ldap_parse_result(VM::ActRec *ar);
TypedValue* fg_ldap_free_result(VM::ActRec *ar);
TypedValue* fg_ldap_get_values_len(VM::ActRec *ar);
TypedValue* fg_ldap_get_values(VM::ActRec *ar);
TypedValue* fg_magickgetcopyright(VM::ActRec *ar);
TypedValue* fg_magickgethomeurl(VM::ActRec *ar);
TypedValue* fg_magickgetpackagename(VM::ActRec *ar);
TypedValue* fg_magickgetquantumdepth(VM::ActRec *ar);
TypedValue* fg_magickgetreleasedate(VM::ActRec *ar);
TypedValue* fg_magickgetresourcelimit(VM::ActRec *ar);
TypedValue* fg_magickgetversion(VM::ActRec *ar);
TypedValue* fg_magickgetversionnumber(VM::ActRec *ar);
TypedValue* fg_magickgetversionstring(VM::ActRec *ar);
TypedValue* fg_magickqueryconfigureoption(VM::ActRec *ar);
TypedValue* fg_magickqueryconfigureoptions(VM::ActRec *ar);
TypedValue* fg_magickqueryfonts(VM::ActRec *ar);
TypedValue* fg_magickqueryformats(VM::ActRec *ar);
TypedValue* fg_magicksetresourcelimit(VM::ActRec *ar);
TypedValue* fg_newdrawingwand(VM::ActRec *ar);
TypedValue* fg_newmagickwand(VM::ActRec *ar);
TypedValue* fg_newpixeliterator(VM::ActRec *ar);
TypedValue* fg_newpixelregioniterator(VM::ActRec *ar);
TypedValue* fg_newpixelwand(VM::ActRec *ar);
TypedValue* fg_newpixelwandarray(VM::ActRec *ar);
TypedValue* fg_newpixelwands(VM::ActRec *ar);
TypedValue* fg_destroydrawingwand(VM::ActRec *ar);
TypedValue* fg_destroymagickwand(VM::ActRec *ar);
TypedValue* fg_destroypixeliterator(VM::ActRec *ar);
TypedValue* fg_destroypixelwand(VM::ActRec *ar);
TypedValue* fg_destroypixelwandarray(VM::ActRec *ar);
TypedValue* fg_destroypixelwands(VM::ActRec *ar);
TypedValue* fg_isdrawingwand(VM::ActRec *ar);
TypedValue* fg_ismagickwand(VM::ActRec *ar);
TypedValue* fg_ispixeliterator(VM::ActRec *ar);
TypedValue* fg_ispixelwand(VM::ActRec *ar);
TypedValue* fg_cleardrawingwand(VM::ActRec *ar);
TypedValue* fg_clearmagickwand(VM::ActRec *ar);
TypedValue* fg_clearpixeliterator(VM::ActRec *ar);
TypedValue* fg_clearpixelwand(VM::ActRec *ar);
TypedValue* fg_clonedrawingwand(VM::ActRec *ar);
TypedValue* fg_clonemagickwand(VM::ActRec *ar);
TypedValue* fg_wandgetexception(VM::ActRec *ar);
TypedValue* fg_wandgetexceptionstring(VM::ActRec *ar);
TypedValue* fg_wandgetexceptiontype(VM::ActRec *ar);
TypedValue* fg_wandhasexception(VM::ActRec *ar);
TypedValue* fg_drawaffine(VM::ActRec *ar);
TypedValue* fg_drawannotation(VM::ActRec *ar);
TypedValue* fg_drawarc(VM::ActRec *ar);
TypedValue* fg_drawbezier(VM::ActRec *ar);
TypedValue* fg_drawcircle(VM::ActRec *ar);
TypedValue* fg_drawcolor(VM::ActRec *ar);
TypedValue* fg_drawcomment(VM::ActRec *ar);
TypedValue* fg_drawcomposite(VM::ActRec *ar);
TypedValue* fg_drawellipse(VM::ActRec *ar);
TypedValue* fg_drawgetclippath(VM::ActRec *ar);
TypedValue* fg_drawgetcliprule(VM::ActRec *ar);
TypedValue* fg_drawgetclipunits(VM::ActRec *ar);
TypedValue* fg_drawgetexception(VM::ActRec *ar);
TypedValue* fg_drawgetexceptionstring(VM::ActRec *ar);
TypedValue* fg_drawgetexceptiontype(VM::ActRec *ar);
TypedValue* fg_drawgetfillalpha(VM::ActRec *ar);
TypedValue* fg_drawgetfillcolor(VM::ActRec *ar);
TypedValue* fg_drawgetfillopacity(VM::ActRec *ar);
TypedValue* fg_drawgetfillrule(VM::ActRec *ar);
TypedValue* fg_drawgetfont(VM::ActRec *ar);
TypedValue* fg_drawgetfontfamily(VM::ActRec *ar);
TypedValue* fg_drawgetfontsize(VM::ActRec *ar);
TypedValue* fg_drawgetfontstretch(VM::ActRec *ar);
TypedValue* fg_drawgetfontstyle(VM::ActRec *ar);
TypedValue* fg_drawgetfontweight(VM::ActRec *ar);
TypedValue* fg_drawgetgravity(VM::ActRec *ar);
TypedValue* fg_drawgetstrokealpha(VM::ActRec *ar);
TypedValue* fg_drawgetstrokeantialias(VM::ActRec *ar);
TypedValue* fg_drawgetstrokecolor(VM::ActRec *ar);
TypedValue* fg_drawgetstrokedasharray(VM::ActRec *ar);
TypedValue* fg_drawgetstrokedashoffset(VM::ActRec *ar);
TypedValue* fg_drawgetstrokelinecap(VM::ActRec *ar);
TypedValue* fg_drawgetstrokelinejoin(VM::ActRec *ar);
TypedValue* fg_drawgetstrokemiterlimit(VM::ActRec *ar);
TypedValue* fg_drawgetstrokeopacity(VM::ActRec *ar);
TypedValue* fg_drawgetstrokewidth(VM::ActRec *ar);
TypedValue* fg_drawgettextalignment(VM::ActRec *ar);
TypedValue* fg_drawgettextantialias(VM::ActRec *ar);
TypedValue* fg_drawgettextdecoration(VM::ActRec *ar);
TypedValue* fg_drawgettextencoding(VM::ActRec *ar);
TypedValue* fg_drawgettextundercolor(VM::ActRec *ar);
TypedValue* fg_drawgetvectorgraphics(VM::ActRec *ar);
TypedValue* fg_drawline(VM::ActRec *ar);
TypedValue* fg_drawmatte(VM::ActRec *ar);
TypedValue* fg_drawpathclose(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetoabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetoquadraticbezierabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetoquadraticbezierrelative(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetoquadraticbeziersmoothabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetoquadraticbeziersmoothrelative(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetorelative(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetosmoothabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathcurvetosmoothrelative(VM::ActRec *ar);
TypedValue* fg_drawpathellipticarcabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathellipticarcrelative(VM::ActRec *ar);
TypedValue* fg_drawpathfinish(VM::ActRec *ar);
TypedValue* fg_drawpathlinetoabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathlinetohorizontalabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathlinetohorizontalrelative(VM::ActRec *ar);
TypedValue* fg_drawpathlinetorelative(VM::ActRec *ar);
TypedValue* fg_drawpathlinetoverticalabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathlinetoverticalrelative(VM::ActRec *ar);
TypedValue* fg_drawpathmovetoabsolute(VM::ActRec *ar);
TypedValue* fg_drawpathmovetorelative(VM::ActRec *ar);
TypedValue* fg_drawpathstart(VM::ActRec *ar);
TypedValue* fg_drawpoint(VM::ActRec *ar);
TypedValue* fg_drawpolygon(VM::ActRec *ar);
TypedValue* fg_drawpolyline(VM::ActRec *ar);
TypedValue* fg_drawrectangle(VM::ActRec *ar);
TypedValue* fg_drawrender(VM::ActRec *ar);
TypedValue* fg_drawrotate(VM::ActRec *ar);
TypedValue* fg_drawroundrectangle(VM::ActRec *ar);
TypedValue* fg_drawscale(VM::ActRec *ar);
TypedValue* fg_drawsetclippath(VM::ActRec *ar);
TypedValue* fg_drawsetcliprule(VM::ActRec *ar);
TypedValue* fg_drawsetclipunits(VM::ActRec *ar);
TypedValue* fg_drawsetfillalpha(VM::ActRec *ar);
TypedValue* fg_drawsetfillcolor(VM::ActRec *ar);
TypedValue* fg_drawsetfillopacity(VM::ActRec *ar);
TypedValue* fg_drawsetfillpatternurl(VM::ActRec *ar);
TypedValue* fg_drawsetfillrule(VM::ActRec *ar);
TypedValue* fg_drawsetfont(VM::ActRec *ar);
TypedValue* fg_drawsetfontfamily(VM::ActRec *ar);
TypedValue* fg_drawsetfontsize(VM::ActRec *ar);
TypedValue* fg_drawsetfontstretch(VM::ActRec *ar);
TypedValue* fg_drawsetfontstyle(VM::ActRec *ar);
TypedValue* fg_drawsetfontweight(VM::ActRec *ar);
TypedValue* fg_drawsetgravity(VM::ActRec *ar);
TypedValue* fg_drawsetstrokealpha(VM::ActRec *ar);
TypedValue* fg_drawsetstrokeantialias(VM::ActRec *ar);
TypedValue* fg_drawsetstrokecolor(VM::ActRec *ar);
TypedValue* fg_drawsetstrokedasharray(VM::ActRec *ar);
TypedValue* fg_drawsetstrokedashoffset(VM::ActRec *ar);
TypedValue* fg_drawsetstrokelinecap(VM::ActRec *ar);
TypedValue* fg_drawsetstrokelinejoin(VM::ActRec *ar);
TypedValue* fg_drawsetstrokemiterlimit(VM::ActRec *ar);
TypedValue* fg_drawsetstrokeopacity(VM::ActRec *ar);
TypedValue* fg_drawsetstrokepatternurl(VM::ActRec *ar);
TypedValue* fg_drawsetstrokewidth(VM::ActRec *ar);
TypedValue* fg_drawsettextalignment(VM::ActRec *ar);
TypedValue* fg_drawsettextantialias(VM::ActRec *ar);
TypedValue* fg_drawsettextdecoration(VM::ActRec *ar);
TypedValue* fg_drawsettextencoding(VM::ActRec *ar);
TypedValue* fg_drawsettextundercolor(VM::ActRec *ar);
TypedValue* fg_drawsetvectorgraphics(VM::ActRec *ar);
TypedValue* fg_drawsetviewbox(VM::ActRec *ar);
TypedValue* fg_drawskewx(VM::ActRec *ar);
TypedValue* fg_drawskewy(VM::ActRec *ar);
TypedValue* fg_drawtranslate(VM::ActRec *ar);
TypedValue* fg_pushdrawingwand(VM::ActRec *ar);
TypedValue* fg_drawpushclippath(VM::ActRec *ar);
TypedValue* fg_drawpushdefs(VM::ActRec *ar);
TypedValue* fg_drawpushpattern(VM::ActRec *ar);
TypedValue* fg_popdrawingwand(VM::ActRec *ar);
TypedValue* fg_drawpopclippath(VM::ActRec *ar);
TypedValue* fg_drawpopdefs(VM::ActRec *ar);
TypedValue* fg_drawpoppattern(VM::ActRec *ar);
TypedValue* fg_magickadaptivethresholdimage(VM::ActRec *ar);
TypedValue* fg_magickaddimage(VM::ActRec *ar);
TypedValue* fg_magickaddnoiseimage(VM::ActRec *ar);
TypedValue* fg_magickaffinetransformimage(VM::ActRec *ar);
TypedValue* fg_magickannotateimage(VM::ActRec *ar);
TypedValue* fg_magickappendimages(VM::ActRec *ar);
TypedValue* fg_magickaverageimages(VM::ActRec *ar);
TypedValue* fg_magickblackthresholdimage(VM::ActRec *ar);
TypedValue* fg_magickblurimage(VM::ActRec *ar);
TypedValue* fg_magickborderimage(VM::ActRec *ar);
TypedValue* fg_magickcharcoalimage(VM::ActRec *ar);
TypedValue* fg_magickchopimage(VM::ActRec *ar);
TypedValue* fg_magickclipimage(VM::ActRec *ar);
TypedValue* fg_magickclippathimage(VM::ActRec *ar);
TypedValue* fg_magickcoalesceimages(VM::ActRec *ar);
TypedValue* fg_magickcolorfloodfillimage(VM::ActRec *ar);
TypedValue* fg_magickcolorizeimage(VM::ActRec *ar);
TypedValue* fg_magickcombineimages(VM::ActRec *ar);
TypedValue* fg_magickcommentimage(VM::ActRec *ar);
TypedValue* fg_magickcompareimages(VM::ActRec *ar);
TypedValue* fg_magickcompositeimage(VM::ActRec *ar);
TypedValue* fg_magickconstituteimage(VM::ActRec *ar);
TypedValue* fg_magickcontrastimage(VM::ActRec *ar);
TypedValue* fg_magickconvolveimage(VM::ActRec *ar);
TypedValue* fg_magickcropimage(VM::ActRec *ar);
TypedValue* fg_magickcyclecolormapimage(VM::ActRec *ar);
TypedValue* fg_magickdeconstructimages(VM::ActRec *ar);
TypedValue* fg_magickdescribeimage(VM::ActRec *ar);
TypedValue* fg_magickdespeckleimage(VM::ActRec *ar);
TypedValue* fg_magickdrawimage(VM::ActRec *ar);
TypedValue* fg_magickechoimageblob(VM::ActRec *ar);
TypedValue* fg_magickechoimagesblob(VM::ActRec *ar);
TypedValue* fg_magickedgeimage(VM::ActRec *ar);
TypedValue* fg_magickembossimage(VM::ActRec *ar);
TypedValue* fg_magickenhanceimage(VM::ActRec *ar);
TypedValue* fg_magickequalizeimage(VM::ActRec *ar);
TypedValue* fg_magickevaluateimage(VM::ActRec *ar);
TypedValue* fg_magickflattenimages(VM::ActRec *ar);
TypedValue* fg_magickflipimage(VM::ActRec *ar);
TypedValue* fg_magickflopimage(VM::ActRec *ar);
TypedValue* fg_magickframeimage(VM::ActRec *ar);
TypedValue* fg_magickfximage(VM::ActRec *ar);
TypedValue* fg_magickgammaimage(VM::ActRec *ar);
TypedValue* fg_magickgaussianblurimage(VM::ActRec *ar);
TypedValue* fg_magickgetcharheight(VM::ActRec *ar);
TypedValue* fg_magickgetcharwidth(VM::ActRec *ar);
TypedValue* fg_magickgetexception(VM::ActRec *ar);
TypedValue* fg_magickgetexceptionstring(VM::ActRec *ar);
TypedValue* fg_magickgetexceptiontype(VM::ActRec *ar);
TypedValue* fg_magickgetfilename(VM::ActRec *ar);
TypedValue* fg_magickgetformat(VM::ActRec *ar);
TypedValue* fg_magickgetimage(VM::ActRec *ar);
TypedValue* fg_magickgetimagebackgroundcolor(VM::ActRec *ar);
TypedValue* fg_magickgetimageblob(VM::ActRec *ar);
TypedValue* fg_magickgetimageblueprimary(VM::ActRec *ar);
TypedValue* fg_magickgetimagebordercolor(VM::ActRec *ar);
TypedValue* fg_magickgetimagechannelmean(VM::ActRec *ar);
TypedValue* fg_magickgetimagecolormapcolor(VM::ActRec *ar);
TypedValue* fg_magickgetimagecolors(VM::ActRec *ar);
TypedValue* fg_magickgetimagecolorspace(VM::ActRec *ar);
TypedValue* fg_magickgetimagecompose(VM::ActRec *ar);
TypedValue* fg_magickgetimagecompression(VM::ActRec *ar);
TypedValue* fg_magickgetimagecompressionquality(VM::ActRec *ar);
TypedValue* fg_magickgetimagedelay(VM::ActRec *ar);
TypedValue* fg_magickgetimagedepth(VM::ActRec *ar);
TypedValue* fg_magickgetimagedispose(VM::ActRec *ar);
TypedValue* fg_magickgetimageextrema(VM::ActRec *ar);
TypedValue* fg_magickgetimagefilename(VM::ActRec *ar);
TypedValue* fg_magickgetimageformat(VM::ActRec *ar);
TypedValue* fg_magickgetimagegamma(VM::ActRec *ar);
TypedValue* fg_magickgetimagegreenprimary(VM::ActRec *ar);
TypedValue* fg_magickgetimageheight(VM::ActRec *ar);
TypedValue* fg_magickgetimagehistogram(VM::ActRec *ar);
TypedValue* fg_magickgetimageindex(VM::ActRec *ar);
TypedValue* fg_magickgetimageinterlacescheme(VM::ActRec *ar);
TypedValue* fg_magickgetimageiterations(VM::ActRec *ar);
TypedValue* fg_magickgetimagemattecolor(VM::ActRec *ar);
TypedValue* fg_magickgetimagemimetype(VM::ActRec *ar);
TypedValue* fg_magickgetimagepixels(VM::ActRec *ar);
TypedValue* fg_magickgetimageprofile(VM::ActRec *ar);
TypedValue* fg_magickgetimageredprimary(VM::ActRec *ar);
TypedValue* fg_magickgetimagerenderingintent(VM::ActRec *ar);
TypedValue* fg_magickgetimageresolution(VM::ActRec *ar);
TypedValue* fg_magickgetimagescene(VM::ActRec *ar);
TypedValue* fg_magickgetimagesignature(VM::ActRec *ar);
TypedValue* fg_magickgetimagesize(VM::ActRec *ar);
TypedValue* fg_magickgetimagetype(VM::ActRec *ar);
TypedValue* fg_magickgetimageunits(VM::ActRec *ar);
TypedValue* fg_magickgetimagevirtualpixelmethod(VM::ActRec *ar);
TypedValue* fg_magickgetimagewhitepoint(VM::ActRec *ar);
TypedValue* fg_magickgetimagewidth(VM::ActRec *ar);
TypedValue* fg_magickgetimagesblob(VM::ActRec *ar);
TypedValue* fg_magickgetinterlacescheme(VM::ActRec *ar);
TypedValue* fg_magickgetmaxtextadvance(VM::ActRec *ar);
TypedValue* fg_magickgetmimetype(VM::ActRec *ar);
TypedValue* fg_magickgetnumberimages(VM::ActRec *ar);
TypedValue* fg_magickgetsamplingfactors(VM::ActRec *ar);
TypedValue* fg_magickgetsize(VM::ActRec *ar);
TypedValue* fg_magickgetstringheight(VM::ActRec *ar);
TypedValue* fg_magickgetstringwidth(VM::ActRec *ar);
TypedValue* fg_magickgettextascent(VM::ActRec *ar);
TypedValue* fg_magickgettextdescent(VM::ActRec *ar);
TypedValue* fg_magickgetwandsize(VM::ActRec *ar);
TypedValue* fg_magickhasnextimage(VM::ActRec *ar);
TypedValue* fg_magickhaspreviousimage(VM::ActRec *ar);
TypedValue* fg_magickimplodeimage(VM::ActRec *ar);
TypedValue* fg_magicklabelimage(VM::ActRec *ar);
TypedValue* fg_magicklevelimage(VM::ActRec *ar);
TypedValue* fg_magickmagnifyimage(VM::ActRec *ar);
TypedValue* fg_magickmapimage(VM::ActRec *ar);
TypedValue* fg_magickmattefloodfillimage(VM::ActRec *ar);
TypedValue* fg_magickmedianfilterimage(VM::ActRec *ar);
TypedValue* fg_magickminifyimage(VM::ActRec *ar);
TypedValue* fg_magickmodulateimage(VM::ActRec *ar);
TypedValue* fg_magickmontageimage(VM::ActRec *ar);
TypedValue* fg_magickmorphimages(VM::ActRec *ar);
TypedValue* fg_magickmosaicimages(VM::ActRec *ar);
TypedValue* fg_magickmotionblurimage(VM::ActRec *ar);
TypedValue* fg_magicknegateimage(VM::ActRec *ar);
TypedValue* fg_magicknewimage(VM::ActRec *ar);
TypedValue* fg_magicknextimage(VM::ActRec *ar);
TypedValue* fg_magicknormalizeimage(VM::ActRec *ar);
TypedValue* fg_magickoilpaintimage(VM::ActRec *ar);
TypedValue* fg_magickpaintopaqueimage(VM::ActRec *ar);
TypedValue* fg_magickpainttransparentimage(VM::ActRec *ar);
TypedValue* fg_magickpingimage(VM::ActRec *ar);
TypedValue* fg_magickposterizeimage(VM::ActRec *ar);
TypedValue* fg_magickpreviewimages(VM::ActRec *ar);
TypedValue* fg_magickpreviousimage(VM::ActRec *ar);
TypedValue* fg_magickprofileimage(VM::ActRec *ar);
TypedValue* fg_magickquantizeimage(VM::ActRec *ar);
TypedValue* fg_magickquantizeimages(VM::ActRec *ar);
TypedValue* fg_magickqueryfontmetrics(VM::ActRec *ar);
TypedValue* fg_magickradialblurimage(VM::ActRec *ar);
TypedValue* fg_magickraiseimage(VM::ActRec *ar);
TypedValue* fg_magickreadimage(VM::ActRec *ar);
TypedValue* fg_magickreadimageblob(VM::ActRec *ar);
TypedValue* fg_magickreadimagefile(VM::ActRec *ar);
TypedValue* fg_magickreadimages(VM::ActRec *ar);
TypedValue* fg_magickreducenoiseimage(VM::ActRec *ar);
TypedValue* fg_magickremoveimage(VM::ActRec *ar);
TypedValue* fg_magickremoveimageprofile(VM::ActRec *ar);
TypedValue* fg_magickremoveimageprofiles(VM::ActRec *ar);
TypedValue* fg_magickresampleimage(VM::ActRec *ar);
TypedValue* fg_magickresetiterator(VM::ActRec *ar);
TypedValue* fg_magickresizeimage(VM::ActRec *ar);
TypedValue* fg_magickrollimage(VM::ActRec *ar);
TypedValue* fg_magickrotateimage(VM::ActRec *ar);
TypedValue* fg_magicksampleimage(VM::ActRec *ar);
TypedValue* fg_magickscaleimage(VM::ActRec *ar);
TypedValue* fg_magickseparateimagechannel(VM::ActRec *ar);
TypedValue* fg_magicksetcompressionquality(VM::ActRec *ar);
TypedValue* fg_magicksetfilename(VM::ActRec *ar);
TypedValue* fg_magicksetfirstiterator(VM::ActRec *ar);
TypedValue* fg_magicksetformat(VM::ActRec *ar);
TypedValue* fg_magicksetimage(VM::ActRec *ar);
TypedValue* fg_magicksetimagebackgroundcolor(VM::ActRec *ar);
TypedValue* fg_magicksetimagebias(VM::ActRec *ar);
TypedValue* fg_magicksetimageblueprimary(VM::ActRec *ar);
TypedValue* fg_magicksetimagebordercolor(VM::ActRec *ar);
TypedValue* fg_magicksetimagecolormapcolor(VM::ActRec *ar);
TypedValue* fg_magicksetimagecolorspace(VM::ActRec *ar);
TypedValue* fg_magicksetimagecompose(VM::ActRec *ar);
TypedValue* fg_magicksetimagecompression(VM::ActRec *ar);
TypedValue* fg_magicksetimagecompressionquality(VM::ActRec *ar);
TypedValue* fg_magicksetimagedelay(VM::ActRec *ar);
TypedValue* fg_magicksetimagedepth(VM::ActRec *ar);
TypedValue* fg_magicksetimagedispose(VM::ActRec *ar);
TypedValue* fg_magicksetimagefilename(VM::ActRec *ar);
TypedValue* fg_magicksetimageformat(VM::ActRec *ar);
TypedValue* fg_magicksetimagegamma(VM::ActRec *ar);
TypedValue* fg_magicksetimagegreenprimary(VM::ActRec *ar);
TypedValue* fg_magicksetimageindex(VM::ActRec *ar);
TypedValue* fg_magicksetimageinterlacescheme(VM::ActRec *ar);
TypedValue* fg_magicksetimageiterations(VM::ActRec *ar);
TypedValue* fg_magicksetimagemattecolor(VM::ActRec *ar);
TypedValue* fg_magicksetimageoption(VM::ActRec *ar);
TypedValue* fg_magicksetimagepixels(VM::ActRec *ar);
TypedValue* fg_magicksetimageprofile(VM::ActRec *ar);
TypedValue* fg_magicksetimageredprimary(VM::ActRec *ar);
TypedValue* fg_magicksetimagerenderingintent(VM::ActRec *ar);
TypedValue* fg_magicksetimageresolution(VM::ActRec *ar);
TypedValue* fg_magicksetimagescene(VM::ActRec *ar);
TypedValue* fg_magicksetimagetype(VM::ActRec *ar);
TypedValue* fg_magicksetimageunits(VM::ActRec *ar);
TypedValue* fg_magicksetimagevirtualpixelmethod(VM::ActRec *ar);
TypedValue* fg_magicksetimagewhitepoint(VM::ActRec *ar);
TypedValue* fg_magicksetinterlacescheme(VM::ActRec *ar);
TypedValue* fg_magicksetlastiterator(VM::ActRec *ar);
TypedValue* fg_magicksetpassphrase(VM::ActRec *ar);
TypedValue* fg_magicksetresolution(VM::ActRec *ar);
TypedValue* fg_magicksetsamplingfactors(VM::ActRec *ar);
TypedValue* fg_magicksetsize(VM::ActRec *ar);
TypedValue* fg_magicksetwandsize(VM::ActRec *ar);
TypedValue* fg_magicksharpenimage(VM::ActRec *ar);
TypedValue* fg_magickshaveimage(VM::ActRec *ar);
TypedValue* fg_magickshearimage(VM::ActRec *ar);
TypedValue* fg_magicksolarizeimage(VM::ActRec *ar);
TypedValue* fg_magickspliceimage(VM::ActRec *ar);
TypedValue* fg_magickspreadimage(VM::ActRec *ar);
TypedValue* fg_magicksteganoimage(VM::ActRec *ar);
TypedValue* fg_magickstereoimage(VM::ActRec *ar);
TypedValue* fg_magickstripimage(VM::ActRec *ar);
TypedValue* fg_magickswirlimage(VM::ActRec *ar);
TypedValue* fg_magicktextureimage(VM::ActRec *ar);
TypedValue* fg_magickthresholdimage(VM::ActRec *ar);
TypedValue* fg_magicktintimage(VM::ActRec *ar);
TypedValue* fg_magicktransformimage(VM::ActRec *ar);
TypedValue* fg_magicktrimimage(VM::ActRec *ar);
TypedValue* fg_magickunsharpmaskimage(VM::ActRec *ar);
TypedValue* fg_magickwaveimage(VM::ActRec *ar);
TypedValue* fg_magickwhitethresholdimage(VM::ActRec *ar);
TypedValue* fg_magickwriteimage(VM::ActRec *ar);
TypedValue* fg_magickwriteimagefile(VM::ActRec *ar);
TypedValue* fg_magickwriteimages(VM::ActRec *ar);
TypedValue* fg_magickwriteimagesfile(VM::ActRec *ar);
TypedValue* fg_pixelgetalpha(VM::ActRec *ar);
TypedValue* fg_pixelgetalphaquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetblack(VM::ActRec *ar);
TypedValue* fg_pixelgetblackquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetblue(VM::ActRec *ar);
TypedValue* fg_pixelgetbluequantum(VM::ActRec *ar);
TypedValue* fg_pixelgetcolorasstring(VM::ActRec *ar);
TypedValue* fg_pixelgetcolorcount(VM::ActRec *ar);
TypedValue* fg_pixelgetcyan(VM::ActRec *ar);
TypedValue* fg_pixelgetcyanquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetexception(VM::ActRec *ar);
TypedValue* fg_pixelgetexceptionstring(VM::ActRec *ar);
TypedValue* fg_pixelgetexceptiontype(VM::ActRec *ar);
TypedValue* fg_pixelgetgreen(VM::ActRec *ar);
TypedValue* fg_pixelgetgreenquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetindex(VM::ActRec *ar);
TypedValue* fg_pixelgetmagenta(VM::ActRec *ar);
TypedValue* fg_pixelgetmagentaquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetopacity(VM::ActRec *ar);
TypedValue* fg_pixelgetopacityquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetquantumcolor(VM::ActRec *ar);
TypedValue* fg_pixelgetred(VM::ActRec *ar);
TypedValue* fg_pixelgetredquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetyellow(VM::ActRec *ar);
TypedValue* fg_pixelgetyellowquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetalpha(VM::ActRec *ar);
TypedValue* fg_pixelsetalphaquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetblack(VM::ActRec *ar);
TypedValue* fg_pixelsetblackquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetblue(VM::ActRec *ar);
TypedValue* fg_pixelsetbluequantum(VM::ActRec *ar);
TypedValue* fg_pixelsetcolor(VM::ActRec *ar);
TypedValue* fg_pixelsetcolorcount(VM::ActRec *ar);
TypedValue* fg_pixelsetcyan(VM::ActRec *ar);
TypedValue* fg_pixelsetcyanquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetgreen(VM::ActRec *ar);
TypedValue* fg_pixelsetgreenquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetindex(VM::ActRec *ar);
TypedValue* fg_pixelsetmagenta(VM::ActRec *ar);
TypedValue* fg_pixelsetmagentaquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetopacity(VM::ActRec *ar);
TypedValue* fg_pixelsetopacityquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetquantumcolor(VM::ActRec *ar);
TypedValue* fg_pixelsetred(VM::ActRec *ar);
TypedValue* fg_pixelsetredquantum(VM::ActRec *ar);
TypedValue* fg_pixelsetyellow(VM::ActRec *ar);
TypedValue* fg_pixelsetyellowquantum(VM::ActRec *ar);
TypedValue* fg_pixelgetiteratorexception(VM::ActRec *ar);
TypedValue* fg_pixelgetiteratorexceptionstring(VM::ActRec *ar);
TypedValue* fg_pixelgetiteratorexceptiontype(VM::ActRec *ar);
TypedValue* fg_pixelgetnextiteratorrow(VM::ActRec *ar);
TypedValue* fg_pixelresetiterator(VM::ActRec *ar);
TypedValue* fg_pixelsetiteratorrow(VM::ActRec *ar);
TypedValue* fg_pixelsynciterator(VM::ActRec *ar);
TypedValue* fg_mail(VM::ActRec *ar);
TypedValue* fg_ezmlm_hash(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_create(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_free(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_parse_file(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_parse(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_extract_part_file(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_extract_whole_part_file(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_extract_part(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_get_part_data(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_get_part(VM::ActRec *ar);
TypedValue* fg_mailparse_msg_get_structure(VM::ActRec *ar);
TypedValue* fg_mailparse_rfc822_parse_addresses(VM::ActRec *ar);
TypedValue* fg_mailparse_stream_encode(VM::ActRec *ar);
TypedValue* fg_mailparse_uudecode_all(VM::ActRec *ar);
TypedValue* fg_mailparse_determine_best_xfer_encoding(VM::ActRec *ar);
TypedValue* fg_pi(VM::ActRec *ar);
TypedValue* fg_min(VM::ActRec *ar);
TypedValue* fg_max(VM::ActRec *ar);
TypedValue* fg_abs(VM::ActRec *ar);
TypedValue* fg_is_finite(VM::ActRec *ar);
TypedValue* fg_is_infinite(VM::ActRec *ar);
TypedValue* fg_is_nan(VM::ActRec *ar);
TypedValue* fg_ceil(VM::ActRec *ar);
TypedValue* fg_floor(VM::ActRec *ar);
TypedValue* fg_round(VM::ActRec *ar);
TypedValue* fg_deg2rad(VM::ActRec *ar);
TypedValue* fg_rad2deg(VM::ActRec *ar);
TypedValue* fg_decbin(VM::ActRec *ar);
TypedValue* fg_dechex(VM::ActRec *ar);
TypedValue* fg_decoct(VM::ActRec *ar);
TypedValue* fg_bindec(VM::ActRec *ar);
TypedValue* fg_hexdec(VM::ActRec *ar);
TypedValue* fg_octdec(VM::ActRec *ar);
TypedValue* fg_base_convert(VM::ActRec *ar);
TypedValue* fg_pow(VM::ActRec *ar);
TypedValue* fg_exp(VM::ActRec *ar);
TypedValue* fg_expm1(VM::ActRec *ar);
TypedValue* fg_log10(VM::ActRec *ar);
TypedValue* fg_log1p(VM::ActRec *ar);
TypedValue* fg_log(VM::ActRec *ar);
TypedValue* fg_cos(VM::ActRec *ar);
TypedValue* fg_cosh(VM::ActRec *ar);
TypedValue* fg_sin(VM::ActRec *ar);
TypedValue* fg_sinh(VM::ActRec *ar);
TypedValue* fg_tan(VM::ActRec *ar);
TypedValue* fg_tanh(VM::ActRec *ar);
TypedValue* fg_acos(VM::ActRec *ar);
TypedValue* fg_acosh(VM::ActRec *ar);
TypedValue* fg_asin(VM::ActRec *ar);
TypedValue* fg_asinh(VM::ActRec *ar);
TypedValue* fg_atan(VM::ActRec *ar);
TypedValue* fg_atanh(VM::ActRec *ar);
TypedValue* fg_atan2(VM::ActRec *ar);
TypedValue* fg_hypot(VM::ActRec *ar);
TypedValue* fg_fmod(VM::ActRec *ar);
TypedValue* fg_sqrt(VM::ActRec *ar);
TypedValue* fg_getrandmax(VM::ActRec *ar);
TypedValue* fg_srand(VM::ActRec *ar);
TypedValue* fg_rand(VM::ActRec *ar);
TypedValue* fg_mt_getrandmax(VM::ActRec *ar);
TypedValue* fg_mt_srand(VM::ActRec *ar);
TypedValue* fg_mt_rand(VM::ActRec *ar);
TypedValue* fg_lcg_value(VM::ActRec *ar);
TypedValue* fg_mb_list_encodings(VM::ActRec *ar);
TypedValue* fg_mb_list_encodings_alias_names(VM::ActRec *ar);
TypedValue* fg_mb_list_mime_names(VM::ActRec *ar);
TypedValue* fg_mb_check_encoding(VM::ActRec *ar);
TypedValue* fg_mb_convert_case(VM::ActRec *ar);
TypedValue* fg_mb_convert_encoding(VM::ActRec *ar);
TypedValue* fg_mb_convert_kana(VM::ActRec *ar);
TypedValue* fg_mb_convert_variables(VM::ActRec *ar);
TypedValue* fg_mb_decode_mimeheader(VM::ActRec *ar);
TypedValue* fg_mb_decode_numericentity(VM::ActRec *ar);
TypedValue* fg_mb_detect_encoding(VM::ActRec *ar);
TypedValue* fg_mb_detect_order(VM::ActRec *ar);
TypedValue* fg_mb_encode_mimeheader(VM::ActRec *ar);
TypedValue* fg_mb_encode_numericentity(VM::ActRec *ar);
TypedValue* fg_mb_ereg_match(VM::ActRec *ar);
TypedValue* fg_mb_ereg_replace(VM::ActRec *ar);
TypedValue* fg_mb_ereg_search_getpos(VM::ActRec *ar);
TypedValue* fg_mb_ereg_search_getregs(VM::ActRec *ar);
TypedValue* fg_mb_ereg_search_init(VM::ActRec *ar);
TypedValue* fg_mb_ereg_search_pos(VM::ActRec *ar);
TypedValue* fg_mb_ereg_search_regs(VM::ActRec *ar);
TypedValue* fg_mb_ereg_search_setpos(VM::ActRec *ar);
TypedValue* fg_mb_ereg_search(VM::ActRec *ar);
TypedValue* fg_mb_ereg(VM::ActRec *ar);
TypedValue* fg_mb_eregi_replace(VM::ActRec *ar);
TypedValue* fg_mb_eregi(VM::ActRec *ar);
TypedValue* fg_mb_get_info(VM::ActRec *ar);
TypedValue* fg_mb_http_input(VM::ActRec *ar);
TypedValue* fg_mb_http_output(VM::ActRec *ar);
TypedValue* fg_mb_internal_encoding(VM::ActRec *ar);
TypedValue* fg_mb_language(VM::ActRec *ar);
TypedValue* fg_mb_output_handler(VM::ActRec *ar);
TypedValue* fg_mb_parse_str(VM::ActRec *ar);
TypedValue* fg_mb_preferred_mime_name(VM::ActRec *ar);
TypedValue* fg_mb_regex_encoding(VM::ActRec *ar);
TypedValue* fg_mb_regex_set_options(VM::ActRec *ar);
TypedValue* fg_mb_send_mail(VM::ActRec *ar);
TypedValue* fg_mb_split(VM::ActRec *ar);
TypedValue* fg_mb_strcut(VM::ActRec *ar);
TypedValue* fg_mb_strimwidth(VM::ActRec *ar);
TypedValue* fg_mb_stripos(VM::ActRec *ar);
TypedValue* fg_mb_stristr(VM::ActRec *ar);
TypedValue* fg_mb_strlen(VM::ActRec *ar);
TypedValue* fg_mb_strpos(VM::ActRec *ar);
TypedValue* fg_mb_strrchr(VM::ActRec *ar);
TypedValue* fg_mb_strrichr(VM::ActRec *ar);
TypedValue* fg_mb_strripos(VM::ActRec *ar);
TypedValue* fg_mb_strrpos(VM::ActRec *ar);
TypedValue* fg_mb_strstr(VM::ActRec *ar);
TypedValue* fg_mb_strtolower(VM::ActRec *ar);
TypedValue* fg_mb_strtoupper(VM::ActRec *ar);
TypedValue* fg_mb_strwidth(VM::ActRec *ar);
TypedValue* fg_mb_substitute_character(VM::ActRec *ar);
TypedValue* fg_mb_substr_count(VM::ActRec *ar);
TypedValue* fg_mb_substr(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_open(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_close(VM::ActRec *ar);
TypedValue* fg_mcrypt_list_algorithms(VM::ActRec *ar);
TypedValue* fg_mcrypt_list_modes(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_get_algo_block_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_get_algo_key_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_get_supported_key_sizes(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_is_block_algorithm_mode(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_is_block_algorithm(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_is_block_mode(VM::ActRec *ar);
TypedValue* fg_mcrypt_module_self_test(VM::ActRec *ar);
TypedValue* fg_mcrypt_create_iv(VM::ActRec *ar);
TypedValue* fg_mcrypt_encrypt(VM::ActRec *ar);
TypedValue* fg_mcrypt_decrypt(VM::ActRec *ar);
TypedValue* fg_mcrypt_cbc(VM::ActRec *ar);
TypedValue* fg_mcrypt_cfb(VM::ActRec *ar);
TypedValue* fg_mcrypt_ecb(VM::ActRec *ar);
TypedValue* fg_mcrypt_ofb(VM::ActRec *ar);
TypedValue* fg_mcrypt_get_block_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_get_cipher_name(VM::ActRec *ar);
TypedValue* fg_mcrypt_get_iv_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_get_key_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_get_algorithms_name(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_get_block_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_get_iv_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_get_key_size(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_get_modes_name(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_get_supported_key_sizes(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_is_block_algorithm_mode(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_is_block_algorithm(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_is_block_mode(VM::ActRec *ar);
TypedValue* fg_mcrypt_enc_self_test(VM::ActRec *ar);
TypedValue* fg_mcrypt_generic(VM::ActRec *ar);
TypedValue* fg_mcrypt_generic_init(VM::ActRec *ar);
TypedValue* fg_mdecrypt_generic(VM::ActRec *ar);
TypedValue* fg_mcrypt_generic_deinit(VM::ActRec *ar);
TypedValue* fg_mcrypt_generic_end(VM::ActRec *ar);
TypedValue* fg_memcache_connect(VM::ActRec *ar);
TypedValue* fg_memcache_pconnect(VM::ActRec *ar);
TypedValue* fg_memcache_add(VM::ActRec *ar);
TypedValue* fg_memcache_set(VM::ActRec *ar);
TypedValue* fg_memcache_replace(VM::ActRec *ar);
TypedValue* fg_memcache_get(VM::ActRec *ar);
TypedValue* fg_memcache_delete(VM::ActRec *ar);
TypedValue* fg_memcache_increment(VM::ActRec *ar);
TypedValue* fg_memcache_decrement(VM::ActRec *ar);
TypedValue* fg_memcache_close(VM::ActRec *ar);
TypedValue* fg_memcache_debug(VM::ActRec *ar);
TypedValue* fg_memcache_get_version(VM::ActRec *ar);
TypedValue* fg_memcache_flush(VM::ActRec *ar);
TypedValue* fg_memcache_setoptimeout(VM::ActRec *ar);
TypedValue* fg_memcache_get_server_status(VM::ActRec *ar);
TypedValue* fg_memcache_set_compress_threshold(VM::ActRec *ar);
TypedValue* fg_memcache_get_stats(VM::ActRec *ar);
TypedValue* fg_memcache_get_extended_stats(VM::ActRec *ar);
TypedValue* fg_memcache_set_server_params(VM::ActRec *ar);
TypedValue* fg_memcache_add_server(VM::ActRec *ar);
TypedValue* fg_connection_aborted(VM::ActRec *ar);
TypedValue* fg_connection_status(VM::ActRec *ar);
TypedValue* fg_connection_timeout(VM::ActRec *ar);
TypedValue* fg_constant(VM::ActRec *ar);
TypedValue* fg_define(VM::ActRec *ar);
TypedValue* fg_defined(VM::ActRec *ar);
TypedValue* fg_die(VM::ActRec *ar);
TypedValue* fg_exit(VM::ActRec *ar);
TypedValue* fg_eval(VM::ActRec *ar);
TypedValue* fg_get_browser(VM::ActRec *ar);
TypedValue* fg___halt_compiler(VM::ActRec *ar);
TypedValue* fg_highlight_file(VM::ActRec *ar);
TypedValue* fg_show_source(VM::ActRec *ar);
TypedValue* fg_highlight_string(VM::ActRec *ar);
TypedValue* fg_ignore_user_abort(VM::ActRec *ar);
TypedValue* fg_pack(VM::ActRec *ar);
TypedValue* fg_php_check_syntax(VM::ActRec *ar);
TypedValue* fg_php_strip_whitespace(VM::ActRec *ar);
TypedValue* fg_sleep(VM::ActRec *ar);
TypedValue* fg_usleep(VM::ActRec *ar);
TypedValue* fg_time_nanosleep(VM::ActRec *ar);
TypedValue* fg_time_sleep_until(VM::ActRec *ar);
TypedValue* fg_uniqid(VM::ActRec *ar);
TypedValue* fg_unpack(VM::ActRec *ar);
TypedValue* fg_sys_getloadavg(VM::ActRec *ar);
TypedValue* fg_token_get_all(VM::ActRec *ar);
TypedValue* fg_token_name(VM::ActRec *ar);
TypedValue* fg_hphp_process_abort(VM::ActRec *ar);
TypedValue* fg_hphp_to_string(VM::ActRec *ar);
TypedValue* fg_mysql_connect(VM::ActRec *ar);
TypedValue* fg_mysql_pconnect(VM::ActRec *ar);
TypedValue* fg_mysql_connect_with_db(VM::ActRec *ar);
TypedValue* fg_mysql_pconnect_with_db(VM::ActRec *ar);
TypedValue* fg_mysql_set_charset(VM::ActRec *ar);
TypedValue* fg_mysql_ping(VM::ActRec *ar);
TypedValue* fg_mysql_escape_string(VM::ActRec *ar);
TypedValue* fg_mysql_real_escape_string(VM::ActRec *ar);
TypedValue* fg_mysql_client_encoding(VM::ActRec *ar);
TypedValue* fg_mysql_close(VM::ActRec *ar);
TypedValue* fg_mysql_errno(VM::ActRec *ar);
TypedValue* fg_mysql_error(VM::ActRec *ar);
TypedValue* fg_mysql_warning_count(VM::ActRec *ar);
TypedValue* fg_mysql_get_client_info(VM::ActRec *ar);
TypedValue* fg_mysql_get_host_info(VM::ActRec *ar);
TypedValue* fg_mysql_get_proto_info(VM::ActRec *ar);
TypedValue* fg_mysql_get_server_info(VM::ActRec *ar);
TypedValue* fg_mysql_info(VM::ActRec *ar);
TypedValue* fg_mysql_insert_id(VM::ActRec *ar);
TypedValue* fg_mysql_stat(VM::ActRec *ar);
TypedValue* fg_mysql_thread_id(VM::ActRec *ar);
TypedValue* fg_mysql_create_db(VM::ActRec *ar);
TypedValue* fg_mysql_select_db(VM::ActRec *ar);
TypedValue* fg_mysql_drop_db(VM::ActRec *ar);
TypedValue* fg_mysql_affected_rows(VM::ActRec *ar);
TypedValue* fg_mysql_set_timeout(VM::ActRec *ar);
TypedValue* fg_mysql_query(VM::ActRec *ar);
TypedValue* fg_mysql_multi_query(VM::ActRec *ar);
TypedValue* fg_mysql_next_result(VM::ActRec *ar);
TypedValue* fg_mysql_more_results(VM::ActRec *ar);
TypedValue* fg_mysql_fetch_result(VM::ActRec *ar);
TypedValue* fg_mysql_unbuffered_query(VM::ActRec *ar);
TypedValue* fg_mysql_db_query(VM::ActRec *ar);
TypedValue* fg_mysql_list_dbs(VM::ActRec *ar);
TypedValue* fg_mysql_list_tables(VM::ActRec *ar);
TypedValue* fg_mysql_list_fields(VM::ActRec *ar);
TypedValue* fg_mysql_list_processes(VM::ActRec *ar);
TypedValue* fg_mysql_db_name(VM::ActRec *ar);
TypedValue* fg_mysql_tablename(VM::ActRec *ar);
TypedValue* fg_mysql_num_fields(VM::ActRec *ar);
TypedValue* fg_mysql_num_rows(VM::ActRec *ar);
TypedValue* fg_mysql_free_result(VM::ActRec *ar);
TypedValue* fg_mysql_data_seek(VM::ActRec *ar);
TypedValue* fg_mysql_fetch_row(VM::ActRec *ar);
TypedValue* fg_mysql_fetch_assoc(VM::ActRec *ar);
TypedValue* fg_mysql_fetch_array(VM::ActRec *ar);
TypedValue* fg_mysql_fetch_lengths(VM::ActRec *ar);
TypedValue* fg_mysql_fetch_object(VM::ActRec *ar);
TypedValue* fg_mysql_result(VM::ActRec *ar);
TypedValue* fg_mysql_fetch_field(VM::ActRec *ar);
TypedValue* fg_mysql_field_seek(VM::ActRec *ar);
TypedValue* fg_mysql_field_name(VM::ActRec *ar);
TypedValue* fg_mysql_field_table(VM::ActRec *ar);
TypedValue* fg_mysql_field_len(VM::ActRec *ar);
TypedValue* fg_mysql_field_type(VM::ActRec *ar);
TypedValue* fg_mysql_field_flags(VM::ActRec *ar);
TypedValue* fg_gethostname(VM::ActRec *ar);
TypedValue* fg_gethostbyaddr(VM::ActRec *ar);
TypedValue* fg_gethostbyname(VM::ActRec *ar);
TypedValue* fg_gethostbynamel(VM::ActRec *ar);
TypedValue* fg_getprotobyname(VM::ActRec *ar);
TypedValue* fg_getprotobynumber(VM::ActRec *ar);
TypedValue* fg_getservbyname(VM::ActRec *ar);
TypedValue* fg_getservbyport(VM::ActRec *ar);
TypedValue* fg_inet_ntop(VM::ActRec *ar);
TypedValue* fg_inet_pton(VM::ActRec *ar);
TypedValue* fg_ip2long(VM::ActRec *ar);
TypedValue* fg_long2ip(VM::ActRec *ar);
TypedValue* fg_dns_check_record(VM::ActRec *ar);
TypedValue* fg_checkdnsrr(VM::ActRec *ar);
TypedValue* fg_dns_get_record(VM::ActRec *ar);
TypedValue* fg_dns_get_mx(VM::ActRec *ar);
TypedValue* fg_getmxrr(VM::ActRec *ar);
TypedValue* fg_fsockopen(VM::ActRec *ar);
TypedValue* fg_pfsockopen(VM::ActRec *ar);
TypedValue* fg_socket_get_status(VM::ActRec *ar);
TypedValue* fg_socket_set_blocking(VM::ActRec *ar);
TypedValue* fg_socket_set_timeout(VM::ActRec *ar);
TypedValue* fg_header(VM::ActRec *ar);
TypedValue* fg_headers_list(VM::ActRec *ar);
TypedValue* fg_headers_sent(VM::ActRec *ar);
TypedValue* fg_header_register_callback(VM::ActRec *ar);
TypedValue* fg_header_remove(VM::ActRec *ar);
TypedValue* fg_setcookie(VM::ActRec *ar);
TypedValue* fg_setrawcookie(VM::ActRec *ar);
TypedValue* fg_define_syslog_variables(VM::ActRec *ar);
TypedValue* fg_openlog(VM::ActRec *ar);
TypedValue* fg_closelog(VM::ActRec *ar);
TypedValue* fg_syslog(VM::ActRec *ar);
TypedValue* fg_openssl_csr_export_to_file(VM::ActRec *ar);
TypedValue* fg_openssl_csr_export(VM::ActRec *ar);
TypedValue* fg_openssl_csr_get_public_key(VM::ActRec *ar);
TypedValue* fg_openssl_csr_get_subject(VM::ActRec *ar);
TypedValue* fg_openssl_csr_new(VM::ActRec *ar);
TypedValue* fg_openssl_csr_sign(VM::ActRec *ar);
TypedValue* fg_openssl_error_string(VM::ActRec *ar);
TypedValue* fg_openssl_open(VM::ActRec *ar);
TypedValue* fg_openssl_pkcs12_export_to_file(VM::ActRec *ar);
TypedValue* fg_openssl_pkcs12_export(VM::ActRec *ar);
TypedValue* fg_openssl_pkcs12_read(VM::ActRec *ar);
TypedValue* fg_openssl_pkcs7_decrypt(VM::ActRec *ar);
TypedValue* fg_openssl_pkcs7_encrypt(VM::ActRec *ar);
TypedValue* fg_openssl_pkcs7_sign(VM::ActRec *ar);
TypedValue* fg_openssl_pkcs7_verify(VM::ActRec *ar);
TypedValue* fg_openssl_pkey_export_to_file(VM::ActRec *ar);
TypedValue* fg_openssl_pkey_export(VM::ActRec *ar);
TypedValue* fg_openssl_pkey_free(VM::ActRec *ar);
TypedValue* fg_openssl_free_key(VM::ActRec *ar);
TypedValue* fg_openssl_pkey_get_details(VM::ActRec *ar);
TypedValue* fg_openssl_pkey_get_private(VM::ActRec *ar);
TypedValue* fg_openssl_get_privatekey(VM::ActRec *ar);
TypedValue* fg_openssl_pkey_get_public(VM::ActRec *ar);
TypedValue* fg_openssl_get_publickey(VM::ActRec *ar);
TypedValue* fg_openssl_pkey_new(VM::ActRec *ar);
TypedValue* fg_openssl_private_decrypt(VM::ActRec *ar);
TypedValue* fg_openssl_private_encrypt(VM::ActRec *ar);
TypedValue* fg_openssl_public_decrypt(VM::ActRec *ar);
TypedValue* fg_openssl_public_encrypt(VM::ActRec *ar);
TypedValue* fg_openssl_seal(VM::ActRec *ar);
TypedValue* fg_openssl_sign(VM::ActRec *ar);
TypedValue* fg_openssl_verify(VM::ActRec *ar);
TypedValue* fg_openssl_x509_check_private_key(VM::ActRec *ar);
TypedValue* fg_openssl_x509_checkpurpose(VM::ActRec *ar);
TypedValue* fg_openssl_x509_export_to_file(VM::ActRec *ar);
TypedValue* fg_openssl_x509_export(VM::ActRec *ar);
TypedValue* fg_openssl_x509_free(VM::ActRec *ar);
TypedValue* fg_openssl_x509_parse(VM::ActRec *ar);
TypedValue* fg_openssl_x509_read(VM::ActRec *ar);
TypedValue* fg_openssl_random_pseudo_bytes(VM::ActRec *ar);
TypedValue* fg_openssl_cipher_iv_length(VM::ActRec *ar);
TypedValue* fg_openssl_encrypt(VM::ActRec *ar);
TypedValue* fg_openssl_decrypt(VM::ActRec *ar);
TypedValue* fg_openssl_digest(VM::ActRec *ar);
TypedValue* fg_openssl_get_cipher_methods(VM::ActRec *ar);
TypedValue* fg_openssl_get_md_methods(VM::ActRec *ar);
TypedValue* fg_assert_options(VM::ActRec *ar);
TypedValue* fg_assert(VM::ActRec *ar);
TypedValue* fg_dl(VM::ActRec *ar);
TypedValue* fg_extension_loaded(VM::ActRec *ar);
TypedValue* fg_get_loaded_extensions(VM::ActRec *ar);
TypedValue* fg_get_extension_funcs(VM::ActRec *ar);
TypedValue* fg_get_cfg_var(VM::ActRec *ar);
TypedValue* fg_get_current_user(VM::ActRec *ar);
TypedValue* fg_get_defined_constants(VM::ActRec *ar);
TypedValue* fg_get_include_path(VM::ActRec *ar);
TypedValue* fg_restore_include_path(VM::ActRec *ar);
TypedValue* fg_set_include_path(VM::ActRec *ar);
TypedValue* fg_get_included_files(VM::ActRec *ar);
TypedValue* fg_inclued_get_data(VM::ActRec *ar);
TypedValue* fg_get_magic_quotes_gpc(VM::ActRec *ar);
TypedValue* fg_get_magic_quotes_runtime(VM::ActRec *ar);
TypedValue* fg_get_required_files(VM::ActRec *ar);
TypedValue* fg_getenv(VM::ActRec *ar);
TypedValue* fg_getlastmod(VM::ActRec *ar);
TypedValue* fg_getmygid(VM::ActRec *ar);
TypedValue* fg_getmyinode(VM::ActRec *ar);
TypedValue* fg_getmypid(VM::ActRec *ar);
TypedValue* fg_getmyuid(VM::ActRec *ar);
TypedValue* fg_getopt(VM::ActRec *ar);
TypedValue* fg_getrusage(VM::ActRec *ar);
TypedValue* fg_clock_getres(VM::ActRec *ar);
TypedValue* fg_clock_gettime(VM::ActRec *ar);
TypedValue* fg_clock_settime(VM::ActRec *ar);
TypedValue* fg_cpu_get_count(VM::ActRec *ar);
TypedValue* fg_cpu_get_model(VM::ActRec *ar);
TypedValue* fg_ini_alter(VM::ActRec *ar);
TypedValue* fg_ini_get_all(VM::ActRec *ar);
TypedValue* fg_ini_get(VM::ActRec *ar);
TypedValue* fg_ini_restore(VM::ActRec *ar);
TypedValue* fg_ini_set(VM::ActRec *ar);
TypedValue* fg_memory_get_allocation(VM::ActRec *ar);
TypedValue* fg_memory_get_peak_usage(VM::ActRec *ar);
TypedValue* fg_memory_get_usage(VM::ActRec *ar);
TypedValue* fg_php_ini_scanned_files(VM::ActRec *ar);
TypedValue* fg_php_logo_guid(VM::ActRec *ar);
TypedValue* fg_php_sapi_name(VM::ActRec *ar);
TypedValue* fg_php_uname(VM::ActRec *ar);
TypedValue* fg_phpcredits(VM::ActRec *ar);
TypedValue* fg_phpinfo(VM::ActRec *ar);
TypedValue* fg_phpversion(VM::ActRec *ar);
TypedValue* fg_putenv(VM::ActRec *ar);
TypedValue* fg_set_magic_quotes_runtime(VM::ActRec *ar);
TypedValue* fg_set_time_limit(VM::ActRec *ar);
TypedValue* fg_sys_get_temp_dir(VM::ActRec *ar);
TypedValue* fg_version_compare(VM::ActRec *ar);
TypedValue* fg_gc_enabled(VM::ActRec *ar);
TypedValue* fg_gc_enable(VM::ActRec *ar);
TypedValue* fg_gc_disable(VM::ActRec *ar);
TypedValue* fg_gc_collect_cycles(VM::ActRec *ar);
TypedValue* fg_zend_logo_guid(VM::ActRec *ar);
TypedValue* fg_zend_thread_id(VM::ActRec *ar);
TypedValue* fg_zend_version(VM::ActRec *ar);
TypedValue* fg_ob_start(VM::ActRec *ar);
TypedValue* fg_ob_clean(VM::ActRec *ar);
TypedValue* fg_ob_flush(VM::ActRec *ar);
TypedValue* fg_ob_end_clean(VM::ActRec *ar);
TypedValue* fg_ob_end_flush(VM::ActRec *ar);
TypedValue* fg_flush(VM::ActRec *ar);
TypedValue* fg_ob_get_clean(VM::ActRec *ar);
TypedValue* fg_ob_get_contents(VM::ActRec *ar);
TypedValue* fg_ob_get_flush(VM::ActRec *ar);
TypedValue* fg_ob_get_length(VM::ActRec *ar);
TypedValue* fg_ob_get_level(VM::ActRec *ar);
TypedValue* fg_ob_get_status(VM::ActRec *ar);
TypedValue* fg_ob_gzhandler(VM::ActRec *ar);
TypedValue* fg_ob_implicit_flush(VM::ActRec *ar);
TypedValue* fg_ob_list_handlers(VM::ActRec *ar);
TypedValue* fg_output_add_rewrite_var(VM::ActRec *ar);
TypedValue* fg_output_reset_rewrite_vars(VM::ActRec *ar);
TypedValue* fg_hphp_crash_log(VM::ActRec *ar);
TypedValue* fg_hphp_stats(VM::ActRec *ar);
TypedValue* fg_hphp_get_stats(VM::ActRec *ar);
TypedValue* fg_hphp_get_status(VM::ActRec *ar);
TypedValue* fg_hphp_get_iostatus(VM::ActRec *ar);
TypedValue* fg_hphp_set_iostatus_address(VM::ActRec *ar);
TypedValue* fg_hphp_get_timers(VM::ActRec *ar);
TypedValue* fg_hphp_output_global_state(VM::ActRec *ar);
TypedValue* fg_hphp_instruction_counter(VM::ActRec *ar);
TypedValue* fg_hphp_get_hardware_counters(VM::ActRec *ar);
TypedValue* fg_hphp_set_hardware_events(VM::ActRec *ar);
TypedValue* fg_hphp_clear_hardware_events(VM::ActRec *ar);
TypedValue* fg_pdo_drivers(VM::ActRec *ar);
TypedValue* fg_posix_access(VM::ActRec *ar);
TypedValue* fg_posix_ctermid(VM::ActRec *ar);
TypedValue* fg_posix_get_last_error(VM::ActRec *ar);
TypedValue* fg_posix_getcwd(VM::ActRec *ar);
TypedValue* fg_posix_getegid(VM::ActRec *ar);
TypedValue* fg_posix_geteuid(VM::ActRec *ar);
TypedValue* fg_posix_getgid(VM::ActRec *ar);
TypedValue* fg_posix_getgrgid(VM::ActRec *ar);
TypedValue* fg_posix_getgrnam(VM::ActRec *ar);
TypedValue* fg_posix_getgroups(VM::ActRec *ar);
TypedValue* fg_posix_getlogin(VM::ActRec *ar);
TypedValue* fg_posix_getpgid(VM::ActRec *ar);
TypedValue* fg_posix_getpgrp(VM::ActRec *ar);
TypedValue* fg_posix_getpid(VM::ActRec *ar);
TypedValue* fg_posix_getppid(VM::ActRec *ar);
TypedValue* fg_posix_getpwnam(VM::ActRec *ar);
TypedValue* fg_posix_getpwuid(VM::ActRec *ar);
TypedValue* fg_posix_getrlimit(VM::ActRec *ar);
TypedValue* fg_posix_getsid(VM::ActRec *ar);
TypedValue* fg_posix_getuid(VM::ActRec *ar);
TypedValue* fg_posix_initgroups(VM::ActRec *ar);
TypedValue* fg_posix_isatty(VM::ActRec *ar);
TypedValue* fg_posix_kill(VM::ActRec *ar);
TypedValue* fg_posix_mkfifo(VM::ActRec *ar);
TypedValue* fg_posix_mknod(VM::ActRec *ar);
TypedValue* fg_posix_setegid(VM::ActRec *ar);
TypedValue* fg_posix_seteuid(VM::ActRec *ar);
TypedValue* fg_posix_setgid(VM::ActRec *ar);
TypedValue* fg_posix_setpgid(VM::ActRec *ar);
TypedValue* fg_posix_setsid(VM::ActRec *ar);
TypedValue* fg_posix_setuid(VM::ActRec *ar);
TypedValue* fg_posix_strerror(VM::ActRec *ar);
TypedValue* fg_posix_times(VM::ActRec *ar);
TypedValue* fg_posix_ttyname(VM::ActRec *ar);
TypedValue* fg_posix_uname(VM::ActRec *ar);
TypedValue* fg_preg_grep(VM::ActRec *ar);
TypedValue* fg_preg_match(VM::ActRec *ar);
TypedValue* fg_preg_match_all(VM::ActRec *ar);
TypedValue* fg_preg_replace(VM::ActRec *ar);
TypedValue* fg_preg_replace_callback(VM::ActRec *ar);
TypedValue* fg_preg_split(VM::ActRec *ar);
TypedValue* fg_preg_quote(VM::ActRec *ar);
TypedValue* fg_preg_last_error(VM::ActRec *ar);
TypedValue* fg_ereg_replace(VM::ActRec *ar);
TypedValue* fg_eregi_replace(VM::ActRec *ar);
TypedValue* fg_ereg(VM::ActRec *ar);
TypedValue* fg_eregi(VM::ActRec *ar);
TypedValue* fg_split(VM::ActRec *ar);
TypedValue* fg_spliti(VM::ActRec *ar);
TypedValue* fg_sql_regcase(VM::ActRec *ar);
TypedValue* fg_pcntl_alarm(VM::ActRec *ar);
TypedValue* fg_pcntl_exec(VM::ActRec *ar);
TypedValue* fg_pcntl_fork(VM::ActRec *ar);
TypedValue* fg_pcntl_getpriority(VM::ActRec *ar);
TypedValue* fg_pcntl_setpriority(VM::ActRec *ar);
TypedValue* fg_pcntl_signal(VM::ActRec *ar);
TypedValue* fg_pcntl_wait(VM::ActRec *ar);
TypedValue* fg_pcntl_waitpid(VM::ActRec *ar);
TypedValue* fg_pcntl_wexitstatus(VM::ActRec *ar);
TypedValue* fg_pcntl_wifexited(VM::ActRec *ar);
TypedValue* fg_pcntl_wifsignaled(VM::ActRec *ar);
TypedValue* fg_pcntl_wifstopped(VM::ActRec *ar);
TypedValue* fg_pcntl_wstopsig(VM::ActRec *ar);
TypedValue* fg_pcntl_wtermsig(VM::ActRec *ar);
TypedValue* fg_pcntl_signal_dispatch(VM::ActRec *ar);
TypedValue* fg_shell_exec(VM::ActRec *ar);
TypedValue* fg_exec(VM::ActRec *ar);
TypedValue* fg_passthru(VM::ActRec *ar);
TypedValue* fg_system(VM::ActRec *ar);
TypedValue* fg_proc_open(VM::ActRec *ar);
TypedValue* fg_proc_terminate(VM::ActRec *ar);
TypedValue* fg_proc_close(VM::ActRec *ar);
TypedValue* fg_proc_get_status(VM::ActRec *ar);
TypedValue* fg_proc_nice(VM::ActRec *ar);
TypedValue* fg_escapeshellarg(VM::ActRec *ar);
TypedValue* fg_escapeshellcmd(VM::ActRec *ar);
TypedValue* fg_hphp_get_extension_info(VM::ActRec *ar);
TypedValue* fg_hphp_get_method_info(VM::ActRec *ar);
TypedValue* fg_hphp_get_closure_info(VM::ActRec *ar);
TypedValue* fg_hphp_get_class_constant(VM::ActRec *ar);
TypedValue* fg_hphp_get_class_info(VM::ActRec *ar);
TypedValue* fg_hphp_get_function_info(VM::ActRec *ar);
TypedValue* fg_hphp_invoke(VM::ActRec *ar);
TypedValue* fg_hphp_invoke_method(VM::ActRec *ar);
TypedValue* fg_hphp_instanceof(VM::ActRec *ar);
TypedValue* fg_hphp_create_object(VM::ActRec *ar);
TypedValue* fg_hphp_get_property(VM::ActRec *ar);
TypedValue* fg_hphp_set_property(VM::ActRec *ar);
TypedValue* fg_hphp_get_static_property(VM::ActRec *ar);
TypedValue* fg_hphp_set_static_property(VM::ActRec *ar);
TypedValue* fg_hphp_get_original_class_name(VM::ActRec *ar);
TypedValue* fg_hphp_scalar_typehints_enabled(VM::ActRec *ar);
TypedValue* fg_dangling_server_proxy_old_request(VM::ActRec *ar);
TypedValue* fg_dangling_server_proxy_new_request(VM::ActRec *ar);
TypedValue* fg_pagelet_server_is_enabled(VM::ActRec *ar);
TypedValue* fg_pagelet_server_task_start(VM::ActRec *ar);
TypedValue* fg_pagelet_server_task_status(VM::ActRec *ar);
TypedValue* fg_pagelet_server_task_result(VM::ActRec *ar);
TypedValue* fg_pagelet_server_flush(VM::ActRec *ar);
TypedValue* fg_xbox_send_message(VM::ActRec *ar);
TypedValue* fg_xbox_post_message(VM::ActRec *ar);
TypedValue* fg_xbox_task_start(VM::ActRec *ar);
TypedValue* fg_xbox_task_status(VM::ActRec *ar);
TypedValue* fg_xbox_task_result(VM::ActRec *ar);
TypedValue* fg_xbox_process_call_message(VM::ActRec *ar);
TypedValue* fg_xbox_get_thread_timeout(VM::ActRec *ar);
TypedValue* fg_xbox_set_thread_timeout(VM::ActRec *ar);
TypedValue* fg_xbox_schedule_thread_reset(VM::ActRec *ar);
TypedValue* fg_xbox_get_thread_time(VM::ActRec *ar);
TypedValue* fg_session_set_cookie_params(VM::ActRec *ar);
TypedValue* fg_session_get_cookie_params(VM::ActRec *ar);
TypedValue* fg_session_name(VM::ActRec *ar);
TypedValue* fg_session_module_name(VM::ActRec *ar);
TypedValue* fg_session_set_save_handler(VM::ActRec *ar);
TypedValue* fg_session_save_path(VM::ActRec *ar);
TypedValue* fg_session_id(VM::ActRec *ar);
TypedValue* fg_session_regenerate_id(VM::ActRec *ar);
TypedValue* fg_session_cache_limiter(VM::ActRec *ar);
TypedValue* fg_session_cache_expire(VM::ActRec *ar);
TypedValue* fg_session_encode(VM::ActRec *ar);
TypedValue* fg_session_decode(VM::ActRec *ar);
TypedValue* fg_session_start(VM::ActRec *ar);
TypedValue* fg_session_destroy(VM::ActRec *ar);
TypedValue* fg_session_unset(VM::ActRec *ar);
TypedValue* fg_session_commit(VM::ActRec *ar);
TypedValue* fg_session_write_close(VM::ActRec *ar);
TypedValue* fg_session_register(VM::ActRec *ar);
TypedValue* fg_session_unregister(VM::ActRec *ar);
TypedValue* fg_session_is_registered(VM::ActRec *ar);
TypedValue* fg_simplexml_load_string(VM::ActRec *ar);
TypedValue* fg_simplexml_load_file(VM::ActRec *ar);
TypedValue* fg_libxml_get_errors(VM::ActRec *ar);
TypedValue* fg_libxml_get_last_error(VM::ActRec *ar);
TypedValue* fg_libxml_clear_errors(VM::ActRec *ar);
TypedValue* fg_libxml_use_internal_errors(VM::ActRec *ar);
TypedValue* fg_libxml_set_streams_context(VM::ActRec *ar);
TypedValue* fg_libxml_disable_entity_loader(VM::ActRec *ar);
TypedValue* fg_use_soap_error_handler(VM::ActRec *ar);
TypedValue* fg_is_soap_fault(VM::ActRec *ar);
TypedValue* fg__soap_active_version(VM::ActRec *ar);
TypedValue* fg_socket_create(VM::ActRec *ar);
TypedValue* fg_socket_create_listen(VM::ActRec *ar);
TypedValue* fg_socket_create_pair(VM::ActRec *ar);
TypedValue* fg_socket_get_option(VM::ActRec *ar);
TypedValue* fg_socket_getpeername(VM::ActRec *ar);
TypedValue* fg_socket_getsockname(VM::ActRec *ar);
TypedValue* fg_socket_set_block(VM::ActRec *ar);
TypedValue* fg_socket_set_nonblock(VM::ActRec *ar);
TypedValue* fg_socket_set_option(VM::ActRec *ar);
TypedValue* fg_socket_connect(VM::ActRec *ar);
TypedValue* fg_socket_bind(VM::ActRec *ar);
TypedValue* fg_socket_listen(VM::ActRec *ar);
TypedValue* fg_socket_select(VM::ActRec *ar);
TypedValue* fg_socket_server(VM::ActRec *ar);
TypedValue* fg_socket_accept(VM::ActRec *ar);
TypedValue* fg_socket_read(VM::ActRec *ar);
TypedValue* fg_socket_write(VM::ActRec *ar);
TypedValue* fg_socket_send(VM::ActRec *ar);
TypedValue* fg_socket_sendto(VM::ActRec *ar);
TypedValue* fg_socket_recv(VM::ActRec *ar);
TypedValue* fg_socket_recvfrom(VM::ActRec *ar);
TypedValue* fg_socket_shutdown(VM::ActRec *ar);
TypedValue* fg_socket_close(VM::ActRec *ar);
TypedValue* fg_socket_strerror(VM::ActRec *ar);
TypedValue* fg_socket_last_error(VM::ActRec *ar);
TypedValue* fg_socket_clear_error(VM::ActRec *ar);
TypedValue* fg_getaddrinfo(VM::ActRec *ar);
TypedValue* fg_spl_classes(VM::ActRec *ar);
TypedValue* fg_spl_object_hash(VM::ActRec *ar);
TypedValue* fg_hphp_object_pointer(VM::ActRec *ar);
TypedValue* fg_hphp_get_this(VM::ActRec *ar);
TypedValue* fg_class_implements(VM::ActRec *ar);
TypedValue* fg_class_parents(VM::ActRec *ar);
TypedValue* fg_class_uses(VM::ActRec *ar);
TypedValue* fg_iterator_apply(VM::ActRec *ar);
TypedValue* fg_iterator_count(VM::ActRec *ar);
TypedValue* fg_iterator_to_array(VM::ActRec *ar);
TypedValue* fg_spl_autoload_call(VM::ActRec *ar);
TypedValue* fg_spl_autoload_extensions(VM::ActRec *ar);
TypedValue* fg_spl_autoload_functions(VM::ActRec *ar);
TypedValue* fg_spl_autoload_register(VM::ActRec *ar);
TypedValue* fg_spl_autoload_unregister(VM::ActRec *ar);
TypedValue* fg_spl_autoload(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo___construct(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getatime(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getbasename(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getctime(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getfileinfo(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getfilename(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getgroup(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getinode(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getlinktarget(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getmtime(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getowner(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getpath(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getpathinfo(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getpathname(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getperms(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getrealpath(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_getsize(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_gettype(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_isdir(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_isexecutable(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_isfile(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_islink(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_isreadable(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_iswritable(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_openfile(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_setfileclass(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo_setinfoclass(VM::ActRec *ar);
TypedValue* fg_hphp_splfileinfo___tostring(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject___construct(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_current(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_eof(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fflush(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fgetc(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fgetcsv(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fgets(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fgetss(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_flock(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fpassthru(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fscanf(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fseek(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fstat(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_ftell(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_ftruncate(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_fwrite(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_getcvscontrol(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_getflags(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_getmaxlinelen(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_key(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_next(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_rewind(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_valid(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_seek(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_setcsvcontrol(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_setflags(VM::ActRec *ar);
TypedValue* fg_hphp_splfileobject_setmaxlinelen(VM::ActRec *ar);
TypedValue* fg_stream_context_create(VM::ActRec *ar);
TypedValue* fg_stream_context_get_default(VM::ActRec *ar);
TypedValue* fg_stream_context_get_options(VM::ActRec *ar);
TypedValue* fg_stream_context_set_option(VM::ActRec *ar);
TypedValue* fg_stream_context_set_param(VM::ActRec *ar);
TypedValue* fg_stream_copy_to_stream(VM::ActRec *ar);
TypedValue* fg_stream_encoding(VM::ActRec *ar);
TypedValue* fg_stream_bucket_append(VM::ActRec *ar);
TypedValue* fg_stream_bucket_prepend(VM::ActRec *ar);
TypedValue* fg_stream_bucket_make_writeable(VM::ActRec *ar);
TypedValue* fg_stream_bucket_new(VM::ActRec *ar);
TypedValue* fg_stream_filter_register(VM::ActRec *ar);
TypedValue* fg_stream_filter_remove(VM::ActRec *ar);
TypedValue* fg_stream_filter_append(VM::ActRec *ar);
TypedValue* fg_stream_filter_prepend(VM::ActRec *ar);
TypedValue* fg_stream_get_contents(VM::ActRec *ar);
TypedValue* fg_stream_get_filters(VM::ActRec *ar);
TypedValue* fg_stream_get_line(VM::ActRec *ar);
TypedValue* fg_stream_get_meta_data(VM::ActRec *ar);
TypedValue* fg_stream_get_transports(VM::ActRec *ar);
TypedValue* fg_stream_get_wrappers(VM::ActRec *ar);
TypedValue* fg_stream_register_wrapper(VM::ActRec *ar);
TypedValue* fg_stream_wrapper_register(VM::ActRec *ar);
TypedValue* fg_stream_wrapper_restore(VM::ActRec *ar);
TypedValue* fg_stream_wrapper_unregister(VM::ActRec *ar);
TypedValue* fg_stream_resolve_include_path(VM::ActRec *ar);
TypedValue* fg_stream_select(VM::ActRec *ar);
TypedValue* fg_stream_set_blocking(VM::ActRec *ar);
TypedValue* fg_stream_set_timeout(VM::ActRec *ar);
TypedValue* fg_stream_set_write_buffer(VM::ActRec *ar);
TypedValue* fg_set_file_buffer(VM::ActRec *ar);
TypedValue* fg_stream_socket_accept(VM::ActRec *ar);
TypedValue* fg_stream_socket_server(VM::ActRec *ar);
TypedValue* fg_stream_socket_client(VM::ActRec *ar);
TypedValue* fg_stream_socket_enable_crypto(VM::ActRec *ar);
TypedValue* fg_stream_socket_get_name(VM::ActRec *ar);
TypedValue* fg_stream_socket_pair(VM::ActRec *ar);
TypedValue* fg_stream_socket_recvfrom(VM::ActRec *ar);
TypedValue* fg_stream_socket_sendto(VM::ActRec *ar);
TypedValue* fg_stream_socket_shutdown(VM::ActRec *ar);
TypedValue* fg_addcslashes(VM::ActRec *ar);
TypedValue* fg_stripcslashes(VM::ActRec *ar);
TypedValue* fg_addslashes(VM::ActRec *ar);
TypedValue* fg_stripslashes(VM::ActRec *ar);
TypedValue* fg_bin2hex(VM::ActRec *ar);
TypedValue* fg_hex2bin(VM::ActRec *ar);
TypedValue* fg_nl2br(VM::ActRec *ar);
TypedValue* fg_quotemeta(VM::ActRec *ar);
TypedValue* fg_str_shuffle(VM::ActRec *ar);
TypedValue* fg_strrev(VM::ActRec *ar);
TypedValue* fg_strtolower(VM::ActRec *ar);
TypedValue* fg_strtoupper(VM::ActRec *ar);
TypedValue* fg_ucfirst(VM::ActRec *ar);
TypedValue* fg_ucwords(VM::ActRec *ar);
TypedValue* fg_strip_tags(VM::ActRec *ar);
TypedValue* fg_trim(VM::ActRec *ar);
TypedValue* fg_ltrim(VM::ActRec *ar);
TypedValue* fg_rtrim(VM::ActRec *ar);
TypedValue* fg_chop(VM::ActRec *ar);
TypedValue* fg_explode(VM::ActRec *ar);
TypedValue* fg_implode(VM::ActRec *ar);
TypedValue* fg_join(VM::ActRec *ar);
TypedValue* fg_str_split(VM::ActRec *ar);
TypedValue* fg_chunk_split(VM::ActRec *ar);
TypedValue* fg_strtok(VM::ActRec *ar);
TypedValue* fg_str_replace(VM::ActRec *ar);
TypedValue* fg_str_ireplace(VM::ActRec *ar);
TypedValue* fg_substr_replace(VM::ActRec *ar);
TypedValue* fg_substr(VM::ActRec *ar);
TypedValue* fg_str_pad(VM::ActRec *ar);
TypedValue* fg_str_repeat(VM::ActRec *ar);
TypedValue* fg_wordwrap(VM::ActRec *ar);
TypedValue* fg_html_entity_decode(VM::ActRec *ar);
TypedValue* fg_htmlentities(VM::ActRec *ar);
TypedValue* fg_htmlspecialchars_decode(VM::ActRec *ar);
TypedValue* fg_htmlspecialchars(VM::ActRec *ar);
TypedValue* fg_fb_htmlspecialchars(VM::ActRec *ar);
TypedValue* fg_quoted_printable_encode(VM::ActRec *ar);
TypedValue* fg_quoted_printable_decode(VM::ActRec *ar);
TypedValue* fg_convert_uudecode(VM::ActRec *ar);
TypedValue* fg_convert_uuencode(VM::ActRec *ar);
TypedValue* fg_str_rot13(VM::ActRec *ar);
TypedValue* fg_crc32(VM::ActRec *ar);
TypedValue* fg_crypt(VM::ActRec *ar);
TypedValue* fg_md5(VM::ActRec *ar);
TypedValue* fg_sha1(VM::ActRec *ar);
TypedValue* fg_strtr(VM::ActRec *ar);
TypedValue* fg_convert_cyr_string(VM::ActRec *ar);
TypedValue* fg_get_html_translation_table(VM::ActRec *ar);
TypedValue* fg_hebrev(VM::ActRec *ar);
TypedValue* fg_hebrevc(VM::ActRec *ar);
TypedValue* fg_setlocale(VM::ActRec *ar);
TypedValue* fg_localeconv(VM::ActRec *ar);
TypedValue* fg_nl_langinfo(VM::ActRec *ar);
TypedValue* fg_printf(VM::ActRec *ar);
TypedValue* fg_vprintf(VM::ActRec *ar);
TypedValue* fg_sprintf(VM::ActRec *ar);
TypedValue* fg_vsprintf(VM::ActRec *ar);
TypedValue* fg_sscanf(VM::ActRec *ar);
TypedValue* fg_chr(VM::ActRec *ar);
TypedValue* fg_ord(VM::ActRec *ar);
TypedValue* fg_money_format(VM::ActRec *ar);
TypedValue* fg_number_format(VM::ActRec *ar);
TypedValue* fg_strcmp(VM::ActRec *ar);
TypedValue* fg_strncmp(VM::ActRec *ar);
TypedValue* fg_strnatcmp(VM::ActRec *ar);
TypedValue* fg_strcasecmp(VM::ActRec *ar);
TypedValue* fg_strncasecmp(VM::ActRec *ar);
TypedValue* fg_strnatcasecmp(VM::ActRec *ar);
TypedValue* fg_strcoll(VM::ActRec *ar);
TypedValue* fg_substr_compare(VM::ActRec *ar);
TypedValue* fg_strchr(VM::ActRec *ar);
TypedValue* fg_strrchr(VM::ActRec *ar);
TypedValue* fg_strstr(VM::ActRec *ar);
TypedValue* fg_stristr(VM::ActRec *ar);
TypedValue* fg_strpbrk(VM::ActRec *ar);
TypedValue* fg_strpos(VM::ActRec *ar);
TypedValue* fg_stripos(VM::ActRec *ar);
TypedValue* fg_strrpos(VM::ActRec *ar);
TypedValue* fg_strripos(VM::ActRec *ar);
TypedValue* fg_substr_count(VM::ActRec *ar);
TypedValue* fg_strspn(VM::ActRec *ar);
TypedValue* fg_strcspn(VM::ActRec *ar);
TypedValue* fg_strlen(VM::ActRec *ar);
TypedValue* fg_count_chars(VM::ActRec *ar);
TypedValue* fg_str_word_count(VM::ActRec *ar);
TypedValue* fg_levenshtein(VM::ActRec *ar);
TypedValue* fg_similar_text(VM::ActRec *ar);
TypedValue* fg_soundex(VM::ActRec *ar);
TypedValue* fg_metaphone(VM::ActRec *ar);
TypedValue* fg_parse_str(VM::ActRec *ar);
TypedValue* fg_hphp_is_service_thread(VM::ActRec *ar);
TypedValue* fg_hphp_service_thread_started(VM::ActRec *ar);
TypedValue* fg_hphp_service_thread_stopped(VM::ActRec *ar);
TypedValue* fg_hphp_thread_is_warmup_enabled(VM::ActRec *ar);
TypedValue* fg_hphp_thread_set_warmup_enabled(VM::ActRec *ar);
TypedValue* fg_hphp_get_thread_id(VM::ActRec *ar);
TypedValue* fg_hphp_gettid(VM::ActRec *ar);
TypedValue* fg_thrift_protocol_write_binary(VM::ActRec *ar);
TypedValue* fg_thrift_protocol_read_binary(VM::ActRec *ar);
TypedValue* fg_thrift_protocol_set_compact_version(VM::ActRec *ar);
TypedValue* fg_thrift_protocol_write_compact(VM::ActRec *ar);
TypedValue* fg_thrift_protocol_read_compact(VM::ActRec *ar);
TypedValue* fg_base64_decode(VM::ActRec *ar);
TypedValue* fg_base64_encode(VM::ActRec *ar);
TypedValue* fg_get_headers(VM::ActRec *ar);
TypedValue* fg_get_meta_tags(VM::ActRec *ar);
TypedValue* fg_http_build_query(VM::ActRec *ar);
TypedValue* fg_parse_url(VM::ActRec *ar);
TypedValue* fg_rawurldecode(VM::ActRec *ar);
TypedValue* fg_rawurlencode(VM::ActRec *ar);
TypedValue* fg_urldecode(VM::ActRec *ar);
TypedValue* fg_urlencode(VM::ActRec *ar);
TypedValue* fg_is_bool(VM::ActRec *ar);
TypedValue* fg_is_int(VM::ActRec *ar);
TypedValue* fg_is_integer(VM::ActRec *ar);
TypedValue* fg_is_long(VM::ActRec *ar);
TypedValue* fg_is_double(VM::ActRec *ar);
TypedValue* fg_is_float(VM::ActRec *ar);
TypedValue* fg_is_numeric(VM::ActRec *ar);
TypedValue* fg_is_real(VM::ActRec *ar);
TypedValue* fg_is_string(VM::ActRec *ar);
TypedValue* fg_is_scalar(VM::ActRec *ar);
TypedValue* fg_is_array(VM::ActRec *ar);
TypedValue* fg_is_object(VM::ActRec *ar);
TypedValue* fg_is_resource(VM::ActRec *ar);
TypedValue* fg_is_null(VM::ActRec *ar);
TypedValue* fg_gettype(VM::ActRec *ar);
TypedValue* fg_get_resource_type(VM::ActRec *ar);
TypedValue* fg_intval(VM::ActRec *ar);
TypedValue* fg_doubleval(VM::ActRec *ar);
TypedValue* fg_floatval(VM::ActRec *ar);
TypedValue* fg_strval(VM::ActRec *ar);
TypedValue* fg_settype(VM::ActRec *ar);
TypedValue* fg_print_r(VM::ActRec *ar);
TypedValue* fg_var_export(VM::ActRec *ar);
TypedValue* fg_var_dump(VM::ActRec *ar);
TypedValue* fg_debug_zval_dump(VM::ActRec *ar);
TypedValue* fg_serialize(VM::ActRec *ar);
TypedValue* fg_unserialize(VM::ActRec *ar);
TypedValue* fg_get_defined_vars(VM::ActRec *ar);
TypedValue* fg_import_request_variables(VM::ActRec *ar);
TypedValue* fg_extract(VM::ActRec *ar);
TypedValue* fg_xml_parser_create(VM::ActRec *ar);
TypedValue* fg_xml_parser_free(VM::ActRec *ar);
TypedValue* fg_xml_parse(VM::ActRec *ar);
TypedValue* fg_xml_parse_into_struct(VM::ActRec *ar);
TypedValue* fg_xml_parser_create_ns(VM::ActRec *ar);
TypedValue* fg_xml_parser_get_option(VM::ActRec *ar);
TypedValue* fg_xml_parser_set_option(VM::ActRec *ar);
TypedValue* fg_xml_set_character_data_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_default_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_element_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_processing_instruction_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_start_namespace_decl_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_end_namespace_decl_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_unparsed_entity_decl_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_external_entity_ref_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_notation_decl_handler(VM::ActRec *ar);
TypedValue* fg_xml_set_object(VM::ActRec *ar);
TypedValue* fg_xml_get_current_byte_index(VM::ActRec *ar);
TypedValue* fg_xml_get_current_column_number(VM::ActRec *ar);
TypedValue* fg_xml_get_current_line_number(VM::ActRec *ar);
TypedValue* fg_xml_get_error_code(VM::ActRec *ar);
TypedValue* fg_xml_error_string(VM::ActRec *ar);
TypedValue* fg_utf8_decode(VM::ActRec *ar);
TypedValue* fg_utf8_encode(VM::ActRec *ar);
TypedValue* fg_xmlwriter_open_memory(VM::ActRec *ar);
TypedValue* fg_xmlwriter_open_uri(VM::ActRec *ar);
TypedValue* fg_xmlwriter_set_indent_string(VM::ActRec *ar);
TypedValue* fg_xmlwriter_set_indent(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_document(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_element(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_element_ns(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_element_ns(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_element(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_element(VM::ActRec *ar);
TypedValue* fg_xmlwriter_full_end_element(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_attribute_ns(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_attribute(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_attribute_ns(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_attribute(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_attribute(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_cdata(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_cdata(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_cdata(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_comment(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_comment(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_comment(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_document(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_pi(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_pi(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_pi(VM::ActRec *ar);
TypedValue* fg_xmlwriter_text(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_raw(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_dtd(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_dtd(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_dtd_element(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_dtd_element(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_dtd_element(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_dtd_attlist(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_dtd_attlist(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_dtd_attlist(VM::ActRec *ar);
TypedValue* fg_xmlwriter_start_dtd_entity(VM::ActRec *ar);
TypedValue* fg_xmlwriter_write_dtd_entity(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_dtd_entity(VM::ActRec *ar);
TypedValue* fg_xmlwriter_end_dtd(VM::ActRec *ar);
TypedValue* fg_xmlwriter_flush(VM::ActRec *ar);
TypedValue* fg_xmlwriter_output_memory(VM::ActRec *ar);
TypedValue* fg_readgzfile(VM::ActRec *ar);
TypedValue* fg_gzfile(VM::ActRec *ar);
TypedValue* fg_gzcompress(VM::ActRec *ar);
TypedValue* fg_gzuncompress(VM::ActRec *ar);
TypedValue* fg_gzdeflate(VM::ActRec *ar);
TypedValue* fg_gzinflate(VM::ActRec *ar);
TypedValue* fg_gzencode(VM::ActRec *ar);
TypedValue* fg_gzdecode(VM::ActRec *ar);
TypedValue* fg_zlib_get_coding_type(VM::ActRec *ar);
TypedValue* fg_gzopen(VM::ActRec *ar);
TypedValue* fg_gzclose(VM::ActRec *ar);
TypedValue* fg_gzrewind(VM::ActRec *ar);
TypedValue* fg_gzeof(VM::ActRec *ar);
TypedValue* fg_gzgetc(VM::ActRec *ar);
TypedValue* fg_gzgets(VM::ActRec *ar);
TypedValue* fg_gzgetss(VM::ActRec *ar);
TypedValue* fg_gzread(VM::ActRec *ar);
TypedValue* fg_gzpassthru(VM::ActRec *ar);
TypedValue* fg_gzseek(VM::ActRec *ar);
TypedValue* fg_gztell(VM::ActRec *ar);
TypedValue* fg_gzwrite(VM::ActRec *ar);
TypedValue* fg_gzputs(VM::ActRec *ar);
TypedValue* fg_qlzcompress(VM::ActRec *ar);
TypedValue* fg_qlzuncompress(VM::ActRec *ar);
TypedValue* fg_sncompress(VM::ActRec *ar);
TypedValue* fg_snuncompress(VM::ActRec *ar);
TypedValue* fg_nzcompress(VM::ActRec *ar);
TypedValue* fg_nzuncompress(VM::ActRec *ar);
VM::Instance* new_DummyClosure_Instance(VM::Class*);
TypedValue* tg_12DummyClosure___construct(VM::ActRec *ar);
VM::Instance* new_Vector_Instance(VM::Class*);
TypedValue* tg_6Vector___construct(VM::ActRec *ar);
TypedValue* tg_6Vector_isEmpty(VM::ActRec *ar);
TypedValue* tg_6Vector_count(VM::ActRec *ar);
TypedValue* tg_6Vector_at(VM::ActRec *ar);
TypedValue* tg_6Vector_get(VM::ActRec *ar);
TypedValue* tg_6Vector_put(VM::ActRec *ar);
TypedValue* tg_6Vector_clear(VM::ActRec *ar);
TypedValue* tg_6Vector_contains(VM::ActRec *ar);
TypedValue* tg_6Vector_append(VM::ActRec *ar);
TypedValue* tg_6Vector_add(VM::ActRec *ar);
TypedValue* tg_6Vector_pop(VM::ActRec *ar);
TypedValue* tg_6Vector_resize(VM::ActRec *ar);
TypedValue* tg_6Vector_toArray(VM::ActRec *ar);
TypedValue* tg_6Vector_getIterator(VM::ActRec *ar);
TypedValue* tg_6Vector_sort(VM::ActRec *ar);
TypedValue* tg_6Vector_reverse(VM::ActRec *ar);
TypedValue* tg_6Vector_splice(VM::ActRec *ar);
TypedValue* tg_6Vector_linearSearch(VM::ActRec *ar);
TypedValue* tg_6Vector_shuffle(VM::ActRec *ar);
TypedValue* tg_6Vector___toString(VM::ActRec *ar);
TypedValue* tg_6Vector___get(VM::ActRec *ar);
TypedValue* tg_6Vector___set(VM::ActRec *ar);
TypedValue* tg_6Vector___isset(VM::ActRec *ar);
TypedValue* tg_6Vector___unset(VM::ActRec *ar);
TypedValue* tg_6Vector_fromArray(VM::ActRec *ar);
TypedValue* tg_6Vector_fromVector(VM::ActRec *ar);
TypedValue* tg_6Vector_slice(VM::ActRec *ar);
VM::Instance* new_VectorIterator_Instance(VM::Class*);
TypedValue* tg_14VectorIterator___construct(VM::ActRec *ar);
TypedValue* tg_14VectorIterator_current(VM::ActRec *ar);
TypedValue* tg_14VectorIterator_key(VM::ActRec *ar);
TypedValue* tg_14VectorIterator_valid(VM::ActRec *ar);
TypedValue* tg_14VectorIterator_next(VM::ActRec *ar);
TypedValue* tg_14VectorIterator_rewind(VM::ActRec *ar);
VM::Instance* new_Map_Instance(VM::Class*);
TypedValue* tg_3Map___construct(VM::ActRec *ar);
TypedValue* tg_3Map_isEmpty(VM::ActRec *ar);
TypedValue* tg_3Map_count(VM::ActRec *ar);
TypedValue* tg_3Map_at(VM::ActRec *ar);
TypedValue* tg_3Map_get(VM::ActRec *ar);
TypedValue* tg_3Map_put(VM::ActRec *ar);
TypedValue* tg_3Map_clear(VM::ActRec *ar);
TypedValue* tg_3Map_contains(VM::ActRec *ar);
TypedValue* tg_3Map_remove(VM::ActRec *ar);
TypedValue* tg_3Map_discard(VM::ActRec *ar);
TypedValue* tg_3Map_toArray(VM::ActRec *ar);
TypedValue* tg_3Map_copyAsArray(VM::ActRec *ar);
TypedValue* tg_3Map_toKeysArray(VM::ActRec *ar);
TypedValue* tg_3Map_values(VM::ActRec *ar);
TypedValue* tg_3Map_toValuesArray(VM::ActRec *ar);
TypedValue* tg_3Map_updateFromArray(VM::ActRec *ar);
TypedValue* tg_3Map_updateFromIterable(VM::ActRec *ar);
TypedValue* tg_3Map_differenceByKey(VM::ActRec *ar);
TypedValue* tg_3Map_getIterator(VM::ActRec *ar);
TypedValue* tg_3Map___toString(VM::ActRec *ar);
TypedValue* tg_3Map___get(VM::ActRec *ar);
TypedValue* tg_3Map___set(VM::ActRec *ar);
TypedValue* tg_3Map___isset(VM::ActRec *ar);
TypedValue* tg_3Map___unset(VM::ActRec *ar);
TypedValue* tg_3Map_fromArray(VM::ActRec *ar);
TypedValue* tg_3Map_fromIterable(VM::ActRec *ar);
VM::Instance* new_MapIterator_Instance(VM::Class*);
TypedValue* tg_11MapIterator___construct(VM::ActRec *ar);
TypedValue* tg_11MapIterator_current(VM::ActRec *ar);
TypedValue* tg_11MapIterator_key(VM::ActRec *ar);
TypedValue* tg_11MapIterator_valid(VM::ActRec *ar);
TypedValue* tg_11MapIterator_next(VM::ActRec *ar);
TypedValue* tg_11MapIterator_rewind(VM::ActRec *ar);
VM::Instance* new_StableMap_Instance(VM::Class*);
TypedValue* tg_9StableMap___construct(VM::ActRec *ar);
TypedValue* tg_9StableMap_isEmpty(VM::ActRec *ar);
TypedValue* tg_9StableMap_count(VM::ActRec *ar);
TypedValue* tg_9StableMap_at(VM::ActRec *ar);
TypedValue* tg_9StableMap_get(VM::ActRec *ar);
TypedValue* tg_9StableMap_put(VM::ActRec *ar);
TypedValue* tg_9StableMap_clear(VM::ActRec *ar);
TypedValue* tg_9StableMap_contains(VM::ActRec *ar);
TypedValue* tg_9StableMap_remove(VM::ActRec *ar);
TypedValue* tg_9StableMap_discard(VM::ActRec *ar);
TypedValue* tg_9StableMap_toArray(VM::ActRec *ar);
TypedValue* tg_9StableMap_copyAsArray(VM::ActRec *ar);
TypedValue* tg_9StableMap_toKeysArray(VM::ActRec *ar);
TypedValue* tg_9StableMap_values(VM::ActRec *ar);
TypedValue* tg_9StableMap_toValuesArray(VM::ActRec *ar);
TypedValue* tg_9StableMap_updateFromArray(VM::ActRec *ar);
TypedValue* tg_9StableMap_updateFromIterable(VM::ActRec *ar);
TypedValue* tg_9StableMap_differenceByKey(VM::ActRec *ar);
TypedValue* tg_9StableMap_getIterator(VM::ActRec *ar);
TypedValue* tg_9StableMap___get(VM::ActRec *ar);
TypedValue* tg_9StableMap___set(VM::ActRec *ar);
TypedValue* tg_9StableMap___isset(VM::ActRec *ar);
TypedValue* tg_9StableMap___unset(VM::ActRec *ar);
TypedValue* tg_9StableMap___toString(VM::ActRec *ar);
TypedValue* tg_9StableMap_fromArray(VM::ActRec *ar);
TypedValue* tg_9StableMap_fromIterable(VM::ActRec *ar);
VM::Instance* new_StableMapIterator_Instance(VM::Class*);
TypedValue* tg_17StableMapIterator___construct(VM::ActRec *ar);
TypedValue* tg_17StableMapIterator_current(VM::ActRec *ar);
TypedValue* tg_17StableMapIterator_key(VM::ActRec *ar);
TypedValue* tg_17StableMapIterator_valid(VM::ActRec *ar);
TypedValue* tg_17StableMapIterator_next(VM::ActRec *ar);
TypedValue* tg_17StableMapIterator_rewind(VM::ActRec *ar);
VM::Instance* new_Continuation_Instance(VM::Class*);
TypedValue* tg_12Continuation___construct(VM::ActRec *ar);
TypedValue* tg_12Continuation_update(VM::ActRec *ar);
TypedValue* tg_12Continuation_done(VM::ActRec *ar);
TypedValue* tg_12Continuation_getLabel(VM::ActRec *ar);
TypedValue* tg_12Continuation_num_args(VM::ActRec *ar);
TypedValue* tg_12Continuation_get_args(VM::ActRec *ar);
TypedValue* tg_12Continuation_get_arg(VM::ActRec *ar);
TypedValue* tg_12Continuation_current(VM::ActRec *ar);
TypedValue* tg_12Continuation_key(VM::ActRec *ar);
TypedValue* tg_12Continuation_next(VM::ActRec *ar);
TypedValue* tg_12Continuation_rewind(VM::ActRec *ar);
TypedValue* tg_12Continuation_valid(VM::ActRec *ar);
TypedValue* tg_12Continuation_send(VM::ActRec *ar);
TypedValue* tg_12Continuation_raise(VM::ActRec *ar);
TypedValue* tg_12Continuation_raised(VM::ActRec *ar);
TypedValue* tg_12Continuation_receive(VM::ActRec *ar);
TypedValue* tg_12Continuation_getOrigFuncName(VM::ActRec *ar);
TypedValue* tg_12Continuation___clone(VM::ActRec *ar);
VM::Instance* new_DummyContinuation_Instance(VM::Class*);
TypedValue* tg_17DummyContinuation___construct(VM::ActRec *ar);
TypedValue* tg_17DummyContinuation_current(VM::ActRec *ar);
TypedValue* tg_17DummyContinuation_key(VM::ActRec *ar);
TypedValue* tg_17DummyContinuation_next(VM::ActRec *ar);
TypedValue* tg_17DummyContinuation_rewind(VM::ActRec *ar);
TypedValue* tg_17DummyContinuation_valid(VM::ActRec *ar);
VM::Instance* new_DateTime_Instance(VM::Class*);
TypedValue* tg_8DateTime_add(VM::ActRec *ar);
TypedValue* tg_8DateTime___construct(VM::ActRec *ar);
TypedValue* tg_8DateTime_createFromFormat(VM::ActRec *ar);
TypedValue* tg_8DateTime_diff(VM::ActRec *ar);
TypedValue* tg_8DateTime_format(VM::ActRec *ar);
TypedValue* tg_8DateTime_getLastErrors(VM::ActRec *ar);
TypedValue* tg_8DateTime_getOffset(VM::ActRec *ar);
TypedValue* tg_8DateTime_getTimestamp(VM::ActRec *ar);
TypedValue* tg_8DateTime_getTimezone(VM::ActRec *ar);
TypedValue* tg_8DateTime_modify(VM::ActRec *ar);
TypedValue* tg_8DateTime_setDate(VM::ActRec *ar);
TypedValue* tg_8DateTime_setISODate(VM::ActRec *ar);
TypedValue* tg_8DateTime_setTime(VM::ActRec *ar);
TypedValue* tg_8DateTime_setTimestamp(VM::ActRec *ar);
TypedValue* tg_8DateTime_setTimezone(VM::ActRec *ar);
TypedValue* tg_8DateTime_sub(VM::ActRec *ar);
VM::Instance* new_DateTimeZone_Instance(VM::Class*);
TypedValue* tg_12DateTimeZone___construct(VM::ActRec *ar);
TypedValue* tg_12DateTimeZone_getLocation(VM::ActRec *ar);
TypedValue* tg_12DateTimeZone_getName(VM::ActRec *ar);
TypedValue* tg_12DateTimeZone_getOffset(VM::ActRec *ar);
TypedValue* tg_12DateTimeZone_getTransitions(VM::ActRec *ar);
TypedValue* tg_12DateTimeZone_listAbbreviations(VM::ActRec *ar);
TypedValue* tg_12DateTimeZone_listIdentifiers(VM::ActRec *ar);
VM::Instance* new_DateInterval_Instance(VM::Class*);
TypedValue* tg_12DateInterval___construct(VM::ActRec *ar);
TypedValue* tg_12DateInterval___get(VM::ActRec *ar);
TypedValue* tg_12DateInterval___set(VM::ActRec *ar);
TypedValue* tg_12DateInterval_createFromDateString(VM::ActRec *ar);
TypedValue* tg_12DateInterval_format(VM::ActRec *ar);
VM::Instance* new_DebuggerProxyCmdUser_Instance(VM::Class*);
TypedValue* tg_20DebuggerProxyCmdUser___construct(VM::ActRec *ar);
TypedValue* tg_20DebuggerProxyCmdUser_isLocal(VM::ActRec *ar);
TypedValue* tg_20DebuggerProxyCmdUser_send(VM::ActRec *ar);
VM::Instance* new_DebuggerClientCmdUser_Instance(VM::Class*);
TypedValue* tg_21DebuggerClientCmdUser___construct(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_quit(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_print(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_help(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_info(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_output(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_error(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_code(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_ask(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_wrap(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_helpTitle(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_helpCmds(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_helpBody(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_helpSection(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_tutorial(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_getCode(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_getCommand(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_arg(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_argCount(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_argValue(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_lineRest(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_args(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_send(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_xend(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_getCurrentLocation(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_getStackTrace(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_getFrame(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_printFrame(VM::ActRec *ar);
TypedValue* tg_21DebuggerClientCmdUser_addCompletion(VM::ActRec *ar);
VM::Instance* new_DebuggerClient_Instance(VM::Class*);
TypedValue* tg_14DebuggerClient___construct(VM::ActRec *ar);
TypedValue* tg_14DebuggerClient_getState(VM::ActRec *ar);
TypedValue* tg_14DebuggerClient_init(VM::ActRec *ar);
TypedValue* tg_14DebuggerClient_processCmd(VM::ActRec *ar);
VM::Instance* new_DOMNode_Instance(VM::Class*);
TypedValue* tg_7DOMNode___construct(VM::ActRec *ar);
TypedValue* tg_7DOMNode_appendChild(VM::ActRec *ar);
TypedValue* tg_7DOMNode_cloneNode(VM::ActRec *ar);
TypedValue* tg_7DOMNode_getLineNo(VM::ActRec *ar);
TypedValue* tg_7DOMNode_hasAttributes(VM::ActRec *ar);
TypedValue* tg_7DOMNode_hasChildNodes(VM::ActRec *ar);
TypedValue* tg_7DOMNode_insertBefore(VM::ActRec *ar);
TypedValue* tg_7DOMNode_isDefaultNamespace(VM::ActRec *ar);
TypedValue* tg_7DOMNode_isSameNode(VM::ActRec *ar);
TypedValue* tg_7DOMNode_isSupported(VM::ActRec *ar);
TypedValue* tg_7DOMNode_lookupNamespaceUri(VM::ActRec *ar);
TypedValue* tg_7DOMNode_lookupPrefix(VM::ActRec *ar);
TypedValue* tg_7DOMNode_normalize(VM::ActRec *ar);
TypedValue* tg_7DOMNode_removeChild(VM::ActRec *ar);
TypedValue* tg_7DOMNode_replaceChild(VM::ActRec *ar);
TypedValue* tg_7DOMNode_c14n(VM::ActRec *ar);
TypedValue* tg_7DOMNode_c14nfile(VM::ActRec *ar);
TypedValue* tg_7DOMNode_getNodePath(VM::ActRec *ar);
TypedValue* tg_7DOMNode___get(VM::ActRec *ar);
TypedValue* tg_7DOMNode___set(VM::ActRec *ar);
TypedValue* tg_7DOMNode___isset(VM::ActRec *ar);
VM::Instance* new_DOMAttr_Instance(VM::Class*);
TypedValue* tg_7DOMAttr___construct(VM::ActRec *ar);
TypedValue* tg_7DOMAttr_isId(VM::ActRec *ar);
TypedValue* tg_7DOMAttr___get(VM::ActRec *ar);
TypedValue* tg_7DOMAttr___set(VM::ActRec *ar);
TypedValue* tg_7DOMAttr___isset(VM::ActRec *ar);
VM::Instance* new_DOMCharacterData_Instance(VM::Class*);
TypedValue* tg_16DOMCharacterData___construct(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData_appendData(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData_deleteData(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData_insertData(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData_replaceData(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData_substringData(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData___get(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData___set(VM::ActRec *ar);
TypedValue* tg_16DOMCharacterData___isset(VM::ActRec *ar);
VM::Instance* new_DOMComment_Instance(VM::Class*);
TypedValue* tg_10DOMComment___construct(VM::ActRec *ar);
VM::Instance* new_DOMText_Instance(VM::Class*);
TypedValue* tg_7DOMText___construct(VM::ActRec *ar);
TypedValue* tg_7DOMText_isWhitespaceInElementContent(VM::ActRec *ar);
TypedValue* tg_7DOMText_splitText(VM::ActRec *ar);
TypedValue* tg_7DOMText___get(VM::ActRec *ar);
TypedValue* tg_7DOMText___set(VM::ActRec *ar);
TypedValue* tg_7DOMText___isset(VM::ActRec *ar);
VM::Instance* new_DOMCDATASection_Instance(VM::Class*);
TypedValue* tg_15DOMCDATASection___construct(VM::ActRec *ar);
VM::Instance* new_DOMDocument_Instance(VM::Class*);
TypedValue* tg_11DOMDocument___construct(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createAttribute(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createAttributens(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createCDATASection(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createComment(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createDocumentFragment(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createElement(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createElementNS(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createEntityReference(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createProcessingInstruction(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_createTextNode(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_getElementById(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_getElementsByTagName(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_getElementsByTagNameNS(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_importNode(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_load(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_loadHTML(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_loadHTMLFile(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_loadXML(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_normalizeDocument(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_registerNodeClass(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_relaxNGValidate(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_relaxNGValidateSource(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_save(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_saveHTML(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_saveHTMLFile(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_saveXML(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_schemaValidate(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_schemaValidateSource(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_validate(VM::ActRec *ar);
TypedValue* tg_11DOMDocument_xinclude(VM::ActRec *ar);
TypedValue* tg_11DOMDocument___get(VM::ActRec *ar);
TypedValue* tg_11DOMDocument___set(VM::ActRec *ar);
TypedValue* tg_11DOMDocument___isset(VM::ActRec *ar);
VM::Instance* new_DOMDocumentFragment_Instance(VM::Class*);
TypedValue* tg_19DOMDocumentFragment___construct(VM::ActRec *ar);
TypedValue* tg_19DOMDocumentFragment_appendXML(VM::ActRec *ar);
VM::Instance* new_DOMDocumentType_Instance(VM::Class*);
TypedValue* tg_15DOMDocumentType___construct(VM::ActRec *ar);
TypedValue* tg_15DOMDocumentType___get(VM::ActRec *ar);
TypedValue* tg_15DOMDocumentType___set(VM::ActRec *ar);
TypedValue* tg_15DOMDocumentType___isset(VM::ActRec *ar);
VM::Instance* new_DOMElement_Instance(VM::Class*);
TypedValue* tg_10DOMElement___construct(VM::ActRec *ar);
TypedValue* tg_10DOMElement_getAttribute(VM::ActRec *ar);
TypedValue* tg_10DOMElement_getAttributeNode(VM::ActRec *ar);
TypedValue* tg_10DOMElement_getAttributeNodeNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement_getAttributeNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement_getElementsByTagName(VM::ActRec *ar);
TypedValue* tg_10DOMElement_getElementsByTagNameNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement_hasAttribute(VM::ActRec *ar);
TypedValue* tg_10DOMElement_hasAttributeNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement_removeAttribute(VM::ActRec *ar);
TypedValue* tg_10DOMElement_removeAttributeNode(VM::ActRec *ar);
TypedValue* tg_10DOMElement_removeAttributeNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement_setAttribute(VM::ActRec *ar);
TypedValue* tg_10DOMElement_setAttributeNode(VM::ActRec *ar);
TypedValue* tg_10DOMElement_setAttributeNodeNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement_setAttributeNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement_setIDAttribute(VM::ActRec *ar);
TypedValue* tg_10DOMElement_setIDAttributeNode(VM::ActRec *ar);
TypedValue* tg_10DOMElement_setIDAttributeNS(VM::ActRec *ar);
TypedValue* tg_10DOMElement___get(VM::ActRec *ar);
TypedValue* tg_10DOMElement___set(VM::ActRec *ar);
TypedValue* tg_10DOMElement___isset(VM::ActRec *ar);
VM::Instance* new_DOMEntity_Instance(VM::Class*);
TypedValue* tg_9DOMEntity___construct(VM::ActRec *ar);
TypedValue* tg_9DOMEntity___get(VM::ActRec *ar);
TypedValue* tg_9DOMEntity___set(VM::ActRec *ar);
TypedValue* tg_9DOMEntity___isset(VM::ActRec *ar);
VM::Instance* new_DOMEntityReference_Instance(VM::Class*);
TypedValue* tg_18DOMEntityReference___construct(VM::ActRec *ar);
VM::Instance* new_DOMNotation_Instance(VM::Class*);
TypedValue* tg_11DOMNotation___construct(VM::ActRec *ar);
TypedValue* tg_11DOMNotation___get(VM::ActRec *ar);
TypedValue* tg_11DOMNotation___set(VM::ActRec *ar);
TypedValue* tg_11DOMNotation___isset(VM::ActRec *ar);
VM::Instance* new_DOMProcessingInstruction_Instance(VM::Class*);
TypedValue* tg_24DOMProcessingInstruction___construct(VM::ActRec *ar);
TypedValue* tg_24DOMProcessingInstruction___get(VM::ActRec *ar);
TypedValue* tg_24DOMProcessingInstruction___set(VM::ActRec *ar);
TypedValue* tg_24DOMProcessingInstruction___isset(VM::ActRec *ar);
VM::Instance* new_DOMNodeIterator_Instance(VM::Class*);
TypedValue* tg_15DOMNodeIterator___construct(VM::ActRec *ar);
TypedValue* tg_15DOMNodeIterator_current(VM::ActRec *ar);
TypedValue* tg_15DOMNodeIterator_key(VM::ActRec *ar);
TypedValue* tg_15DOMNodeIterator_next(VM::ActRec *ar);
TypedValue* tg_15DOMNodeIterator_rewind(VM::ActRec *ar);
TypedValue* tg_15DOMNodeIterator_valid(VM::ActRec *ar);
VM::Instance* new_DOMNamedNodeMap_Instance(VM::Class*);
TypedValue* tg_15DOMNamedNodeMap___construct(VM::ActRec *ar);
TypedValue* tg_15DOMNamedNodeMap_getNamedItem(VM::ActRec *ar);
TypedValue* tg_15DOMNamedNodeMap_getNamedItemNS(VM::ActRec *ar);
TypedValue* tg_15DOMNamedNodeMap_item(VM::ActRec *ar);
TypedValue* tg_15DOMNamedNodeMap___get(VM::ActRec *ar);
TypedValue* tg_15DOMNamedNodeMap___set(VM::ActRec *ar);
TypedValue* tg_15DOMNamedNodeMap___isset(VM::ActRec *ar);
TypedValue* tg_15DOMNamedNodeMap_getIterator(VM::ActRec *ar);
VM::Instance* new_DOMNodeList_Instance(VM::Class*);
TypedValue* tg_11DOMNodeList___construct(VM::ActRec *ar);
TypedValue* tg_11DOMNodeList_item(VM::ActRec *ar);
TypedValue* tg_11DOMNodeList___get(VM::ActRec *ar);
TypedValue* tg_11DOMNodeList___set(VM::ActRec *ar);
TypedValue* tg_11DOMNodeList___isset(VM::ActRec *ar);
TypedValue* tg_11DOMNodeList_getIterator(VM::ActRec *ar);
VM::Instance* new_DOMImplementation_Instance(VM::Class*);
TypedValue* tg_17DOMImplementation___construct(VM::ActRec *ar);
TypedValue* tg_17DOMImplementation_createDocument(VM::ActRec *ar);
TypedValue* tg_17DOMImplementation_createDocumentType(VM::ActRec *ar);
TypedValue* tg_17DOMImplementation_hasFeature(VM::ActRec *ar);
VM::Instance* new_DOMXPath_Instance(VM::Class*);
TypedValue* tg_8DOMXPath___construct(VM::ActRec *ar);
TypedValue* tg_8DOMXPath_evaluate(VM::ActRec *ar);
TypedValue* tg_8DOMXPath_query(VM::ActRec *ar);
TypedValue* tg_8DOMXPath_registerNamespace(VM::ActRec *ar);
TypedValue* tg_8DOMXPath_registerPHPFunctions(VM::ActRec *ar);
TypedValue* tg_8DOMXPath___get(VM::ActRec *ar);
TypedValue* tg_8DOMXPath___set(VM::ActRec *ar);
TypedValue* tg_8DOMXPath___isset(VM::ActRec *ar);
VM::Instance* new_EncodingDetector_Instance(VM::Class*);
TypedValue* tg_16EncodingDetector___construct(VM::ActRec *ar);
TypedValue* tg_16EncodingDetector_setText(VM::ActRec *ar);
TypedValue* tg_16EncodingDetector_setDeclaredEncoding(VM::ActRec *ar);
TypedValue* tg_16EncodingDetector_detect(VM::ActRec *ar);
TypedValue* tg_16EncodingDetector_detectAll(VM::ActRec *ar);
VM::Instance* new_EncodingMatch_Instance(VM::Class*);
TypedValue* tg_13EncodingMatch___construct(VM::ActRec *ar);
TypedValue* tg_13EncodingMatch_isValid(VM::ActRec *ar);
TypedValue* tg_13EncodingMatch_getEncoding(VM::ActRec *ar);
TypedValue* tg_13EncodingMatch_getConfidence(VM::ActRec *ar);
TypedValue* tg_13EncodingMatch_getLanguage(VM::ActRec *ar);
TypedValue* tg_13EncodingMatch_getUTF8(VM::ActRec *ar);
VM::Instance* new_SpoofChecker_Instance(VM::Class*);
TypedValue* tg_12SpoofChecker___construct(VM::ActRec *ar);
TypedValue* tg_12SpoofChecker_isSuspicious(VM::ActRec *ar);
TypedValue* tg_12SpoofChecker_areConfusable(VM::ActRec *ar);
TypedValue* tg_12SpoofChecker_setAllowedLocales(VM::ActRec *ar);
TypedValue* tg_12SpoofChecker_setChecks(VM::ActRec *ar);
VM::Instance* new_ImageSprite_Instance(VM::Class*);
TypedValue* tg_11ImageSprite___construct(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_addFile(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_addString(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_addUrl(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_clear(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_loadDims(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_loadImages(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_output(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_css(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_getErrors(VM::ActRec *ar);
TypedValue* tg_11ImageSprite_mapping(VM::ActRec *ar);
TypedValue* tg_11ImageSprite___destruct(VM::ActRec *ar);
VM::Instance* new_Collator_Instance(VM::Class*);
TypedValue* tg_8Collator___construct(VM::ActRec *ar);
TypedValue* tg_8Collator_asort(VM::ActRec *ar);
TypedValue* tg_8Collator_compare(VM::ActRec *ar);
TypedValue* tg_8Collator_create(VM::ActRec *ar);
TypedValue* tg_8Collator_getattribute(VM::ActRec *ar);
TypedValue* tg_8Collator_geterrorcode(VM::ActRec *ar);
TypedValue* tg_8Collator_geterrormessage(VM::ActRec *ar);
TypedValue* tg_8Collator_getlocale(VM::ActRec *ar);
TypedValue* tg_8Collator_getstrength(VM::ActRec *ar);
TypedValue* tg_8Collator_setattribute(VM::ActRec *ar);
TypedValue* tg_8Collator_setstrength(VM::ActRec *ar);
TypedValue* tg_8Collator_sortwithsortkeys(VM::ActRec *ar);
TypedValue* tg_8Collator_sort(VM::ActRec *ar);
VM::Instance* new_Locale_Instance(VM::Class*);
TypedValue* tg_6Locale___construct(VM::ActRec *ar);
VM::Instance* new_Normalizer_Instance(VM::Class*);
TypedValue* tg_10Normalizer___construct(VM::ActRec *ar);
TypedValue* tg_10Normalizer_isnormalized(VM::ActRec *ar);
TypedValue* tg_10Normalizer_normalize(VM::ActRec *ar);
VM::Instance* new_MutableArrayIterator_Instance(VM::Class*);
TypedValue* tg_20MutableArrayIterator___construct(VM::ActRec *ar);
TypedValue* tg_20MutableArrayIterator_currentRef(VM::ActRec *ar);
TypedValue* tg_20MutableArrayIterator_current(VM::ActRec *ar);
TypedValue* tg_20MutableArrayIterator_key(VM::ActRec *ar);
TypedValue* tg_20MutableArrayIterator_next(VM::ActRec *ar);
TypedValue* tg_20MutableArrayIterator_valid(VM::ActRec *ar);
VM::Instance* new_Memcache_Instance(VM::Class*);
TypedValue* tg_8Memcache___construct(VM::ActRec *ar);
TypedValue* tg_8Memcache_connect(VM::ActRec *ar);
TypedValue* tg_8Memcache_pconnect(VM::ActRec *ar);
TypedValue* tg_8Memcache_add(VM::ActRec *ar);
TypedValue* tg_8Memcache_set(VM::ActRec *ar);
TypedValue* tg_8Memcache_replace(VM::ActRec *ar);
TypedValue* tg_8Memcache_get(VM::ActRec *ar);
TypedValue* tg_8Memcache_delete(VM::ActRec *ar);
TypedValue* tg_8Memcache_increment(VM::ActRec *ar);
TypedValue* tg_8Memcache_decrement(VM::ActRec *ar);
TypedValue* tg_8Memcache_getversion(VM::ActRec *ar);
TypedValue* tg_8Memcache_flush(VM::ActRec *ar);
TypedValue* tg_8Memcache_setoptimeout(VM::ActRec *ar);
TypedValue* tg_8Memcache_close(VM::ActRec *ar);
TypedValue* tg_8Memcache_getserverstatus(VM::ActRec *ar);
TypedValue* tg_8Memcache_setcompressthreshold(VM::ActRec *ar);
TypedValue* tg_8Memcache_getstats(VM::ActRec *ar);
TypedValue* tg_8Memcache_getextendedstats(VM::ActRec *ar);
TypedValue* tg_8Memcache_setserverparams(VM::ActRec *ar);
TypedValue* tg_8Memcache_addserver(VM::ActRec *ar);
TypedValue* tg_8Memcache___destruct(VM::ActRec *ar);
VM::Instance* new_Memcached_Instance(VM::Class*);
TypedValue* tg_9Memcached___construct(VM::ActRec *ar);
TypedValue* tg_9Memcached_add(VM::ActRec *ar);
TypedValue* tg_9Memcached_addByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_addServer(VM::ActRec *ar);
TypedValue* tg_9Memcached_addServers(VM::ActRec *ar);
TypedValue* tg_9Memcached_append(VM::ActRec *ar);
TypedValue* tg_9Memcached_appendByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_cas(VM::ActRec *ar);
TypedValue* tg_9Memcached_casByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_decrement(VM::ActRec *ar);
TypedValue* tg_9Memcached_delete(VM::ActRec *ar);
TypedValue* tg_9Memcached_deleteByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_fetch(VM::ActRec *ar);
TypedValue* tg_9Memcached_fetchAll(VM::ActRec *ar);
TypedValue* tg_9Memcached_flush(VM::ActRec *ar);
TypedValue* tg_9Memcached_get(VM::ActRec *ar);
TypedValue* tg_9Memcached_getByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_getDelayed(VM::ActRec *ar);
TypedValue* tg_9Memcached_getDelayedByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_getMulti(VM::ActRec *ar);
TypedValue* tg_9Memcached_getMultiByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_getOption(VM::ActRec *ar);
TypedValue* tg_9Memcached_getResultCode(VM::ActRec *ar);
TypedValue* tg_9Memcached_getResultMessage(VM::ActRec *ar);
TypedValue* tg_9Memcached_getServerByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_getServerList(VM::ActRec *ar);
TypedValue* tg_9Memcached_getStats(VM::ActRec *ar);
TypedValue* tg_9Memcached_getVersion(VM::ActRec *ar);
TypedValue* tg_9Memcached_increment(VM::ActRec *ar);
TypedValue* tg_9Memcached_prepend(VM::ActRec *ar);
TypedValue* tg_9Memcached_prependByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_replace(VM::ActRec *ar);
TypedValue* tg_9Memcached_replaceByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_set(VM::ActRec *ar);
TypedValue* tg_9Memcached_setByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_setMulti(VM::ActRec *ar);
TypedValue* tg_9Memcached_setMultiByKey(VM::ActRec *ar);
TypedValue* tg_9Memcached_setOption(VM::ActRec *ar);
VM::Instance* new_PDO_Instance(VM::Class*);
TypedValue* tg_3PDO___construct(VM::ActRec *ar);
TypedValue* tg_3PDO_prepare(VM::ActRec *ar);
TypedValue* tg_3PDO_begintransaction(VM::ActRec *ar);
TypedValue* tg_3PDO_commit(VM::ActRec *ar);
TypedValue* tg_3PDO_rollback(VM::ActRec *ar);
TypedValue* tg_3PDO_setattribute(VM::ActRec *ar);
TypedValue* tg_3PDO_getattribute(VM::ActRec *ar);
TypedValue* tg_3PDO_exec(VM::ActRec *ar);
TypedValue* tg_3PDO_lastinsertid(VM::ActRec *ar);
TypedValue* tg_3PDO_errorcode(VM::ActRec *ar);
TypedValue* tg_3PDO_errorinfo(VM::ActRec *ar);
TypedValue* tg_3PDO_query(VM::ActRec *ar);
TypedValue* tg_3PDO_quote(VM::ActRec *ar);
TypedValue* tg_3PDO___wakeup(VM::ActRec *ar);
TypedValue* tg_3PDO___sleep(VM::ActRec *ar);
TypedValue* tg_3PDO_getavailabledrivers(VM::ActRec *ar);
VM::Instance* new_PDOStatement_Instance(VM::Class*);
TypedValue* tg_12PDOStatement___construct(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_execute(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_fetch(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_fetchobject(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_fetchcolumn(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_fetchall(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_bindvalue(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_bindparam(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_bindcolumn(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_rowcount(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_errorcode(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_errorinfo(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_setattribute(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_getattribute(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_columncount(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_getcolumnmeta(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_setfetchmode(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_nextrowset(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_closecursor(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_debugdumpparams(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_current(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_key(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_next(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_rewind(VM::ActRec *ar);
TypedValue* tg_12PDOStatement_valid(VM::ActRec *ar);
TypedValue* tg_12PDOStatement___wakeup(VM::ActRec *ar);
TypedValue* tg_12PDOStatement___sleep(VM::ActRec *ar);
VM::Instance* new_SimpleXMLElement_Instance(VM::Class*);
TypedValue* tg_16SimpleXMLElement___construct(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_offsetExists(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_offsetGet(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_offsetSet(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_offsetUnset(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_getIterator(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_count(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_xpath(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_registerXPathNamespace(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_asXML(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_getNamespaces(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_getDocNamespaces(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_children(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_getName(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_attributes(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_addChild(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement_addAttribute(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement___toString(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement___get(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement___set(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement___isset(VM::ActRec *ar);
TypedValue* tg_16SimpleXMLElement___unset(VM::ActRec *ar);
VM::Instance* new_LibXMLError_Instance(VM::Class*);
TypedValue* tg_11LibXMLError___construct(VM::ActRec *ar);
VM::Instance* new_SimpleXMLElementIterator_Instance(VM::Class*);
TypedValue* tg_24SimpleXMLElementIterator___construct(VM::ActRec *ar);
TypedValue* tg_24SimpleXMLElementIterator_current(VM::ActRec *ar);
TypedValue* tg_24SimpleXMLElementIterator_key(VM::ActRec *ar);
TypedValue* tg_24SimpleXMLElementIterator_next(VM::ActRec *ar);
TypedValue* tg_24SimpleXMLElementIterator_rewind(VM::ActRec *ar);
TypedValue* tg_24SimpleXMLElementIterator_valid(VM::ActRec *ar);
VM::Instance* new_SoapServer_Instance(VM::Class*);
TypedValue* tg_10SoapServer___construct(VM::ActRec *ar);
TypedValue* tg_10SoapServer_setclass(VM::ActRec *ar);
TypedValue* tg_10SoapServer_setobject(VM::ActRec *ar);
TypedValue* tg_10SoapServer_addfunction(VM::ActRec *ar);
TypedValue* tg_10SoapServer_getfunctions(VM::ActRec *ar);
TypedValue* tg_10SoapServer_handle(VM::ActRec *ar);
TypedValue* tg_10SoapServer_setpersistence(VM::ActRec *ar);
TypedValue* tg_10SoapServer_fault(VM::ActRec *ar);
TypedValue* tg_10SoapServer_addsoapheader(VM::ActRec *ar);
VM::Instance* new_SoapClient_Instance(VM::Class*);
TypedValue* tg_10SoapClient___construct(VM::ActRec *ar);
TypedValue* tg_10SoapClient___call(VM::ActRec *ar);
TypedValue* tg_10SoapClient___soapcall(VM::ActRec *ar);
TypedValue* tg_10SoapClient___getlastrequest(VM::ActRec *ar);
TypedValue* tg_10SoapClient___getlastresponse(VM::ActRec *ar);
TypedValue* tg_10SoapClient___getlastrequestheaders(VM::ActRec *ar);
TypedValue* tg_10SoapClient___getlastresponseheaders(VM::ActRec *ar);
TypedValue* tg_10SoapClient___getfunctions(VM::ActRec *ar);
TypedValue* tg_10SoapClient___gettypes(VM::ActRec *ar);
TypedValue* tg_10SoapClient___dorequest(VM::ActRec *ar);
TypedValue* tg_10SoapClient___setcookie(VM::ActRec *ar);
TypedValue* tg_10SoapClient___setlocation(VM::ActRec *ar);
TypedValue* tg_10SoapClient___setsoapheaders(VM::ActRec *ar);
VM::Instance* new_SoapVar_Instance(VM::Class*);
TypedValue* tg_7SoapVar___construct(VM::ActRec *ar);
VM::Instance* new_SoapParam_Instance(VM::Class*);
TypedValue* tg_9SoapParam___construct(VM::ActRec *ar);
VM::Instance* new_SoapHeader_Instance(VM::Class*);
TypedValue* tg_10SoapHeader___construct(VM::ActRec *ar);
VM::Instance* new_SQLite3_Instance(VM::Class*);
TypedValue* tg_7SQLite3___construct(VM::ActRec *ar);
TypedValue* tg_7SQLite3_open(VM::ActRec *ar);
TypedValue* tg_7SQLite3_busytimeout(VM::ActRec *ar);
TypedValue* tg_7SQLite3_close(VM::ActRec *ar);
TypedValue* tg_7SQLite3_exec(VM::ActRec *ar);
TypedValue* tg_7SQLite3_version(VM::ActRec *ar);
TypedValue* tg_7SQLite3_lastinsertrowid(VM::ActRec *ar);
TypedValue* tg_7SQLite3_lasterrorcode(VM::ActRec *ar);
TypedValue* tg_7SQLite3_lasterrormsg(VM::ActRec *ar);
TypedValue* tg_7SQLite3_loadextension(VM::ActRec *ar);
TypedValue* tg_7SQLite3_changes(VM::ActRec *ar);
TypedValue* tg_7SQLite3_escapestring(VM::ActRec *ar);
TypedValue* tg_7SQLite3_prepare(VM::ActRec *ar);
TypedValue* tg_7SQLite3_query(VM::ActRec *ar);
TypedValue* tg_7SQLite3_querysingle(VM::ActRec *ar);
TypedValue* tg_7SQLite3_createfunction(VM::ActRec *ar);
TypedValue* tg_7SQLite3_createaggregate(VM::ActRec *ar);
TypedValue* tg_7SQLite3_openblob(VM::ActRec *ar);
VM::Instance* new_SQLite3Stmt_Instance(VM::Class*);
TypedValue* tg_11SQLite3Stmt___construct(VM::ActRec *ar);
TypedValue* tg_11SQLite3Stmt_paramcount(VM::ActRec *ar);
TypedValue* tg_11SQLite3Stmt_close(VM::ActRec *ar);
TypedValue* tg_11SQLite3Stmt_reset(VM::ActRec *ar);
TypedValue* tg_11SQLite3Stmt_clear(VM::ActRec *ar);
TypedValue* tg_11SQLite3Stmt_bindparam(VM::ActRec *ar);
TypedValue* tg_11SQLite3Stmt_bindvalue(VM::ActRec *ar);
TypedValue* tg_11SQLite3Stmt_execute(VM::ActRec *ar);
VM::Instance* new_SQLite3Result_Instance(VM::Class*);
TypedValue* tg_13SQLite3Result___construct(VM::ActRec *ar);
TypedValue* tg_13SQLite3Result_numcolumns(VM::ActRec *ar);
TypedValue* tg_13SQLite3Result_columnname(VM::ActRec *ar);
TypedValue* tg_13SQLite3Result_columntype(VM::ActRec *ar);
TypedValue* tg_13SQLite3Result_fetcharray(VM::ActRec *ar);
TypedValue* tg_13SQLite3Result_reset(VM::ActRec *ar);
TypedValue* tg_13SQLite3Result_finalize(VM::ActRec *ar);
VM::Instance* new_XMLReader_Instance(VM::Class*);
TypedValue* tg_9XMLReader___construct(VM::ActRec *ar);
TypedValue* tg_9XMLReader_open(VM::ActRec *ar);
TypedValue* tg_9XMLReader_XML(VM::ActRec *ar);
TypedValue* tg_9XMLReader_close(VM::ActRec *ar);
TypedValue* tg_9XMLReader_read(VM::ActRec *ar);
TypedValue* tg_9XMLReader_next(VM::ActRec *ar);
TypedValue* tg_9XMLReader_readString(VM::ActRec *ar);
TypedValue* tg_9XMLReader_readInnerXML(VM::ActRec *ar);
TypedValue* tg_9XMLReader_readOuterXML(VM::ActRec *ar);
TypedValue* tg_9XMLReader_moveToNextAttribute(VM::ActRec *ar);
TypedValue* tg_9XMLReader_getAttribute(VM::ActRec *ar);
TypedValue* tg_9XMLReader_getAttributeNo(VM::ActRec *ar);
TypedValue* tg_9XMLReader_getAttributeNs(VM::ActRec *ar);
TypedValue* tg_9XMLReader_moveToAttribute(VM::ActRec *ar);
TypedValue* tg_9XMLReader_moveToAttributeNo(VM::ActRec *ar);
TypedValue* tg_9XMLReader_moveToAttributeNs(VM::ActRec *ar);
TypedValue* tg_9XMLReader_moveToElement(VM::ActRec *ar);
TypedValue* tg_9XMLReader_moveToFirstAttribute(VM::ActRec *ar);
TypedValue* tg_9XMLReader_isValid(VM::ActRec *ar);
TypedValue* tg_9XMLReader_expand(VM::ActRec *ar);
TypedValue* tg_9XMLReader___get(VM::ActRec *ar);
TypedValue* tg_9XMLReader_getParserProperty(VM::ActRec *ar);
TypedValue* tg_9XMLReader_lookupNamespace(VM::ActRec *ar);
TypedValue* tg_9XMLReader_setSchema(VM::ActRec *ar);
TypedValue* tg_9XMLReader_setParserProperty(VM::ActRec *ar);
TypedValue* tg_9XMLReader_setRelaxNGSchema(VM::ActRec *ar);
TypedValue* tg_9XMLReader_setRelaxNGSchemaSource(VM::ActRec *ar);
VM::Instance* new_XMLWriter_Instance(VM::Class*);
TypedValue* tg_9XMLWriter___construct(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_openMemory(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_openURI(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_setIndentString(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_setIndent(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startDocument(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startElement(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startElementNS(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeElementNS(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeElement(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endElement(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_fullEndElement(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startAttributens(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startAttribute(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeAttributeNS(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeAttribute(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endAttribute(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startCData(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeCData(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endCData(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startComment(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeComment(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endComment(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endDocument(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startPI(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writePI(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endPI(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_text(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeRaw(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startDTD(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeDTD(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startDTDElement(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeDTDElement(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endDTDElement(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startDTDAttlist(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeDTDAttlist(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endDTDAttlist(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_startDTDEntity(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_writeDTDEntity(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endDTDEntity(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_endDTD(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_flush(VM::ActRec *ar);
TypedValue* tg_9XMLWriter_outputMemory(VM::ActRec *ar);

const long long hhbc_ext_funcs_count = 2186;
const HhbcExtFuncInfo hhbc_ext_funcs[] = {
  { "apache_note", fg_apache_note },
  { "apache_request_headers", fg_apache_request_headers },
  { "apache_response_headers", fg_apache_response_headers },
  { "apache_setenv", fg_apache_setenv },
  { "getallheaders", fg_getallheaders },
  { "virtual", fg_virtual },
  { "apache_get_config", fg_apache_get_config },
  { "apache_get_scoreboard", fg_apache_get_scoreboard },
  { "apache_get_rewrite_rules", fg_apache_get_rewrite_rules },
  { "apc_add", fg_apc_add },
  { "apc_store", fg_apc_store },
  { "apc_fetch", fg_apc_fetch },
  { "apc_delete", fg_apc_delete },
  { "apc_compile_file", fg_apc_compile_file },
  { "apc_cache_info", fg_apc_cache_info },
  { "apc_clear_cache", fg_apc_clear_cache },
  { "apc_define_constants", fg_apc_define_constants },
  { "apc_load_constants", fg_apc_load_constants },
  { "apc_sma_info", fg_apc_sma_info },
  { "apc_filehits", fg_apc_filehits },
  { "apc_delete_file", fg_apc_delete_file },
  { "apc_inc", fg_apc_inc },
  { "apc_dec", fg_apc_dec },
  { "apc_cas", fg_apc_cas },
  { "apc_exists", fg_apc_exists },
  { "apc_bin_dump", fg_apc_bin_dump },
  { "apc_bin_load", fg_apc_bin_load },
  { "apc_bin_dumpfile", fg_apc_bin_dumpfile },
  { "apc_bin_loadfile", fg_apc_bin_loadfile },
  { "override_function", fg_override_function },
  { "rename_function", fg_rename_function },
  { "apd_set_browser_trace", fg_apd_set_browser_trace },
  { "apd_set_pprof_trace", fg_apd_set_pprof_trace },
  { "apd_set_session_trace_socket", fg_apd_set_session_trace_socket },
  { "apd_stop_trace", fg_apd_stop_trace },
  { "apd_breakpoint", fg_apd_breakpoint },
  { "apd_continue", fg_apd_continue },
  { "apd_echo", fg_apd_echo },
  { "array_change_key_case", fg_array_change_key_case },
  { "array_chunk", fg_array_chunk },
  { "array_combine", fg_array_combine },
  { "array_count_values", fg_array_count_values },
  { "array_fill_keys", fg_array_fill_keys },
  { "array_fill", fg_array_fill },
  { "array_filter", fg_array_filter },
  { "array_flip", fg_array_flip },
  { "array_key_exists", fg_array_key_exists },
  { "key_exists", fg_key_exists },
  { "array_keys", fg_array_keys },
  { "array_map", fg_array_map },
  { "array_merge_recursive", fg_array_merge_recursive },
  { "array_merge", fg_array_merge },
  { "array_replace_recursive", fg_array_replace_recursive },
  { "array_replace", fg_array_replace },
  { "array_multisort", fg_array_multisort },
  { "array_pad", fg_array_pad },
  { "array_pop", fg_array_pop },
  { "array_product", fg_array_product },
  { "array_push", fg_array_push },
  { "array_rand", fg_array_rand },
  { "array_reduce", fg_array_reduce },
  { "array_reverse", fg_array_reverse },
  { "array_search", fg_array_search },
  { "array_shift", fg_array_shift },
  { "array_slice", fg_array_slice },
  { "array_splice", fg_array_splice },
  { "array_sum", fg_array_sum },
  { "array_unique", fg_array_unique },
  { "array_unshift", fg_array_unshift },
  { "array_values", fg_array_values },
  { "array_walk_recursive", fg_array_walk_recursive },
  { "array_walk", fg_array_walk },
  { "compact", fg_compact },
  { "shuffle", fg_shuffle },
  { "count", fg_count },
  { "sizeof", fg_sizeof },
  { "each", fg_each },
  { "current", fg_current },
  { "hphp_current_ref", fg_hphp_current_ref },
  { "next", fg_next },
  { "pos", fg_pos },
  { "prev", fg_prev },
  { "reset", fg_reset },
  { "end", fg_end },
  { "key", fg_key },
  { "hphp_get_iterator", fg_hphp_get_iterator },
  { "hphp_get_mutable_iterator", fg_hphp_get_mutable_iterator },
  { "in_array", fg_in_array },
  { "range", fg_range },
  { "array_diff", fg_array_diff },
  { "array_udiff", fg_array_udiff },
  { "array_diff_assoc", fg_array_diff_assoc },
  { "array_diff_uassoc", fg_array_diff_uassoc },
  { "array_udiff_assoc", fg_array_udiff_assoc },
  { "array_udiff_uassoc", fg_array_udiff_uassoc },
  { "array_diff_key", fg_array_diff_key },
  { "array_diff_ukey", fg_array_diff_ukey },
  { "array_intersect", fg_array_intersect },
  { "array_uintersect", fg_array_uintersect },
  { "array_intersect_assoc", fg_array_intersect_assoc },
  { "array_intersect_uassoc", fg_array_intersect_uassoc },
  { "array_uintersect_assoc", fg_array_uintersect_assoc },
  { "array_uintersect_uassoc", fg_array_uintersect_uassoc },
  { "array_intersect_key", fg_array_intersect_key },
  { "array_intersect_ukey", fg_array_intersect_ukey },
  { "sort", fg_sort },
  { "rsort", fg_rsort },
  { "asort", fg_asort },
  { "arsort", fg_arsort },
  { "ksort", fg_ksort },
  { "krsort", fg_krsort },
  { "usort", fg_usort },
  { "uasort", fg_uasort },
  { "uksort", fg_uksort },
  { "natsort", fg_natsort },
  { "natcasesort", fg_natcasesort },
  { "i18n_loc_get_default", fg_i18n_loc_get_default },
  { "i18n_loc_set_default", fg_i18n_loc_set_default },
  { "i18n_loc_set_attribute", fg_i18n_loc_set_attribute },
  { "i18n_loc_set_strength", fg_i18n_loc_set_strength },
  { "i18n_loc_get_error_code", fg_i18n_loc_get_error_code },
  { "bcscale", fg_bcscale },
  { "bcadd", fg_bcadd },
  { "bcsub", fg_bcsub },
  { "bccomp", fg_bccomp },
  { "bcmul", fg_bcmul },
  { "bcdiv", fg_bcdiv },
  { "bcmod", fg_bcmod },
  { "bcpow", fg_bcpow },
  { "bcpowmod", fg_bcpowmod },
  { "bcsqrt", fg_bcsqrt },
  { "bzclose", fg_bzclose },
  { "bzopen", fg_bzopen },
  { "bzread", fg_bzread },
  { "bzwrite", fg_bzwrite },
  { "bzflush", fg_bzflush },
  { "bzerrstr", fg_bzerrstr },
  { "bzerror", fg_bzerror },
  { "bzerrno", fg_bzerrno },
  { "bzcompress", fg_bzcompress },
  { "bzdecompress", fg_bzdecompress },
  { "get_declared_classes", fg_get_declared_classes },
  { "get_declared_interfaces", fg_get_declared_interfaces },
  { "get_declared_traits", fg_get_declared_traits },
  { "class_exists", fg_class_exists },
  { "interface_exists", fg_interface_exists },
  { "trait_exists", fg_trait_exists },
  { "get_class_methods", fg_get_class_methods },
  { "get_class_vars", fg_get_class_vars },
  { "get_class_constants", fg_get_class_constants },
  { "get_class", fg_get_class },
  { "get_parent_class", fg_get_parent_class },
  { "is_a", fg_is_a },
  { "is_subclass_of", fg_is_subclass_of },
  { "method_exists", fg_method_exists },
  { "property_exists", fg_property_exists },
  { "get_object_vars", fg_get_object_vars },
  { "call_user_method_array", fg_call_user_method_array },
  { "call_user_method", fg_call_user_method },
  { "hphp_create_continuation", fg_hphp_create_continuation },
  { "hphp_pack_continuation", fg_hphp_pack_continuation },
  { "hphp_unpack_continuation", fg_hphp_unpack_continuation },
  { "ctype_alnum", fg_ctype_alnum },
  { "ctype_alpha", fg_ctype_alpha },
  { "ctype_cntrl", fg_ctype_cntrl },
  { "ctype_digit", fg_ctype_digit },
  { "ctype_graph", fg_ctype_graph },
  { "ctype_lower", fg_ctype_lower },
  { "ctype_print", fg_ctype_print },
  { "ctype_punct", fg_ctype_punct },
  { "ctype_space", fg_ctype_space },
  { "ctype_upper", fg_ctype_upper },
  { "ctype_xdigit", fg_ctype_xdigit },
  { "curl_init", fg_curl_init },
  { "curl_copy_handle", fg_curl_copy_handle },
  { "curl_version", fg_curl_version },
  { "curl_setopt", fg_curl_setopt },
  { "curl_setopt_array", fg_curl_setopt_array },
  { "fb_curl_getopt", fg_fb_curl_getopt },
  { "curl_exec", fg_curl_exec },
  { "curl_getinfo", fg_curl_getinfo },
  { "curl_errno", fg_curl_errno },
  { "curl_error", fg_curl_error },
  { "curl_close", fg_curl_close },
  { "curl_multi_init", fg_curl_multi_init },
  { "curl_multi_add_handle", fg_curl_multi_add_handle },
  { "curl_multi_remove_handle", fg_curl_multi_remove_handle },
  { "curl_multi_exec", fg_curl_multi_exec },
  { "curl_multi_select", fg_curl_multi_select },
  { "fb_curl_multi_fdset", fg_fb_curl_multi_fdset },
  { "curl_multi_getcontent", fg_curl_multi_getcontent },
  { "curl_multi_info_read", fg_curl_multi_info_read },
  { "curl_multi_close", fg_curl_multi_close },
  { "evhttp_set_cache", fg_evhttp_set_cache },
  { "evhttp_get", fg_evhttp_get },
  { "evhttp_post", fg_evhttp_post },
  { "evhttp_async_get", fg_evhttp_async_get },
  { "evhttp_async_post", fg_evhttp_async_post },
  { "evhttp_recv", fg_evhttp_recv },
  { "checkdate", fg_checkdate },
  { "date_add", fg_date_add },
  { "date_create_from_format", fg_date_create_from_format },
  { "date_create", fg_date_create },
  { "date_date_set", fg_date_date_set },
  { "date_default_timezone_get", fg_date_default_timezone_get },
  { "date_default_timezone_set", fg_date_default_timezone_set },
  { "date_diff", fg_date_diff },
  { "date_format", fg_date_format },
  { "date_get_last_errors", fg_date_get_last_errors },
  { "date_interval_create_from_date_string", fg_date_interval_create_from_date_string },
  { "date_interval_format", fg_date_interval_format },
  { "date_isodate_set", fg_date_isodate_set },
  { "date_modify", fg_date_modify },
  { "date_offset_get", fg_date_offset_get },
  { "date_parse", fg_date_parse },
  { "date_sub", fg_date_sub },
  { "date_sun_info", fg_date_sun_info },
  { "date_sunrise", fg_date_sunrise },
  { "date_sunset", fg_date_sunset },
  { "date_time_set", fg_date_time_set },
  { "date_timestamp_get", fg_date_timestamp_get },
  { "date_timestamp_set", fg_date_timestamp_set },
  { "date_timezone_get", fg_date_timezone_get },
  { "date_timezone_set", fg_date_timezone_set },
  { "date", fg_date },
  { "getdate", fg_getdate },
  { "gettimeofday", fg_gettimeofday },
  { "gmdate", fg_gmdate },
  { "gmmktime", fg_gmmktime },
  { "gmstrftime", fg_gmstrftime },
  { "idate", fg_idate },
  { "localtime", fg_localtime },
  { "microtime", fg_microtime },
  { "mktime", fg_mktime },
  { "strftime", fg_strftime },
  { "strptime", fg_strptime },
  { "strtotime", fg_strtotime },
  { "time", fg_time },
  { "timezone_abbreviations_list", fg_timezone_abbreviations_list },
  { "timezone_identifiers_list", fg_timezone_identifiers_list },
  { "timezone_location_get", fg_timezone_location_get },
  { "timezone_name_from_abbr", fg_timezone_name_from_abbr },
  { "timezone_name_get", fg_timezone_name_get },
  { "timezone_offset_get", fg_timezone_offset_get },
  { "timezone_open", fg_timezone_open },
  { "timezone_transitions_get", fg_timezone_transitions_get },
  { "timezone_version_get", fg_timezone_version_get },
  { "hphpd_install_user_command", fg_hphpd_install_user_command },
  { "hphpd_get_user_commands", fg_hphpd_get_user_commands },
  { "hphpd_break", fg_hphpd_break },
  { "hphpd_get_client", fg_hphpd_get_client },
  { "hphpd_client_ctrl", fg_hphpd_client_ctrl },
  { "dom_document_create_element", fg_dom_document_create_element },
  { "dom_document_create_document_fragment", fg_dom_document_create_document_fragment },
  { "dom_document_create_text_node", fg_dom_document_create_text_node },
  { "dom_document_create_comment", fg_dom_document_create_comment },
  { "dom_document_create_cdatasection", fg_dom_document_create_cdatasection },
  { "dom_document_create_processing_instruction", fg_dom_document_create_processing_instruction },
  { "dom_document_create_attribute", fg_dom_document_create_attribute },
  { "dom_document_create_entity_reference", fg_dom_document_create_entity_reference },
  { "dom_document_get_elements_by_tag_name", fg_dom_document_get_elements_by_tag_name },
  { "dom_document_import_node", fg_dom_document_import_node },
  { "dom_document_create_element_ns", fg_dom_document_create_element_ns },
  { "dom_document_create_attribute_ns", fg_dom_document_create_attribute_ns },
  { "dom_document_get_elements_by_tag_name_ns", fg_dom_document_get_elements_by_tag_name_ns },
  { "dom_document_get_element_by_id", fg_dom_document_get_element_by_id },
  { "dom_document_normalize_document", fg_dom_document_normalize_document },
  { "dom_document_save", fg_dom_document_save },
  { "dom_document_savexml", fg_dom_document_savexml },
  { "dom_document_validate", fg_dom_document_validate },
  { "dom_document_xinclude", fg_dom_document_xinclude },
  { "dom_document_save_html", fg_dom_document_save_html },
  { "dom_document_save_html_file", fg_dom_document_save_html_file },
  { "dom_document_schema_validate_file", fg_dom_document_schema_validate_file },
  { "dom_document_schema_validate_xml", fg_dom_document_schema_validate_xml },
  { "dom_document_relaxng_validate_file", fg_dom_document_relaxng_validate_file },
  { "dom_document_relaxng_validate_xml", fg_dom_document_relaxng_validate_xml },
  { "dom_node_insert_before", fg_dom_node_insert_before },
  { "dom_node_replace_child", fg_dom_node_replace_child },
  { "dom_node_remove_child", fg_dom_node_remove_child },
  { "dom_node_append_child", fg_dom_node_append_child },
  { "dom_node_has_child_nodes", fg_dom_node_has_child_nodes },
  { "dom_node_clone_node", fg_dom_node_clone_node },
  { "dom_node_normalize", fg_dom_node_normalize },
  { "dom_node_is_supported", fg_dom_node_is_supported },
  { "dom_node_has_attributes", fg_dom_node_has_attributes },
  { "dom_node_is_same_node", fg_dom_node_is_same_node },
  { "dom_node_lookup_prefix", fg_dom_node_lookup_prefix },
  { "dom_node_is_default_namespace", fg_dom_node_is_default_namespace },
  { "dom_node_lookup_namespace_uri", fg_dom_node_lookup_namespace_uri },
  { "dom_nodelist_item", fg_dom_nodelist_item },
  { "dom_namednodemap_get_named_item", fg_dom_namednodemap_get_named_item },
  { "dom_namednodemap_item", fg_dom_namednodemap_item },
  { "dom_namednodemap_get_named_item_ns", fg_dom_namednodemap_get_named_item_ns },
  { "dom_characterdata_substring_data", fg_dom_characterdata_substring_data },
  { "dom_characterdata_append_data", fg_dom_characterdata_append_data },
  { "dom_characterdata_insert_data", fg_dom_characterdata_insert_data },
  { "dom_characterdata_delete_data", fg_dom_characterdata_delete_data },
  { "dom_characterdata_replace_data", fg_dom_characterdata_replace_data },
  { "dom_attr_is_id", fg_dom_attr_is_id },
  { "dom_element_get_attribute", fg_dom_element_get_attribute },
  { "dom_element_set_attribute", fg_dom_element_set_attribute },
  { "dom_element_remove_attribute", fg_dom_element_remove_attribute },
  { "dom_element_get_attribute_node", fg_dom_element_get_attribute_node },
  { "dom_element_set_attribute_node", fg_dom_element_set_attribute_node },
  { "dom_element_remove_attribute_node", fg_dom_element_remove_attribute_node },
  { "dom_element_get_elements_by_tag_name", fg_dom_element_get_elements_by_tag_name },
  { "dom_element_get_attribute_ns", fg_dom_element_get_attribute_ns },
  { "dom_element_set_attribute_ns", fg_dom_element_set_attribute_ns },
  { "dom_element_remove_attribute_ns", fg_dom_element_remove_attribute_ns },
  { "dom_element_get_attribute_node_ns", fg_dom_element_get_attribute_node_ns },
  { "dom_element_set_attribute_node_ns", fg_dom_element_set_attribute_node_ns },
  { "dom_element_get_elements_by_tag_name_ns", fg_dom_element_get_elements_by_tag_name_ns },
  { "dom_element_has_attribute", fg_dom_element_has_attribute },
  { "dom_element_has_attribute_ns", fg_dom_element_has_attribute_ns },
  { "dom_element_set_id_attribute", fg_dom_element_set_id_attribute },
  { "dom_element_set_id_attribute_ns", fg_dom_element_set_id_attribute_ns },
  { "dom_element_set_id_attribute_node", fg_dom_element_set_id_attribute_node },
  { "dom_text_split_text", fg_dom_text_split_text },
  { "dom_text_is_whitespace_in_element_content", fg_dom_text_is_whitespace_in_element_content },
  { "dom_xpath_register_ns", fg_dom_xpath_register_ns },
  { "dom_xpath_query", fg_dom_xpath_query },
  { "dom_xpath_evaluate", fg_dom_xpath_evaluate },
  { "dom_xpath_register_php_functions", fg_dom_xpath_register_php_functions },
  { "debug_backtrace", fg_debug_backtrace },
  { "debug_print_backtrace", fg_debug_print_backtrace },
  { "error_get_last", fg_error_get_last },
  { "error_log", fg_error_log },
  { "error_reporting", fg_error_reporting },
  { "restore_error_handler", fg_restore_error_handler },
  { "restore_exception_handler", fg_restore_exception_handler },
  { "set_error_handler", fg_set_error_handler },
  { "set_exception_handler", fg_set_exception_handler },
  { "hphp_set_error_page", fg_hphp_set_error_page },
  { "hphp_throw_fatal_error", fg_hphp_throw_fatal_error },
  { "hphp_clear_unflushed", fg_hphp_clear_unflushed },
  { "hphp_debug_caller_info", fg_hphp_debug_caller_info },
  { "trigger_error", fg_trigger_error },
  { "user_error", fg_user_error },
  { "fb_thrift_serialize", fg_fb_thrift_serialize },
  { "fb_thrift_unserialize", fg_fb_thrift_unserialize },
  { "fb_serialize", fg_fb_serialize },
  { "fb_unserialize", fg_fb_unserialize },
  { "fb_compact_serialize", fg_fb_compact_serialize },
  { "fb_compact_unserialize", fg_fb_compact_unserialize },
  { "fb_could_include", fg_fb_could_include },
  { "fb_intercept", fg_fb_intercept },
  { "fb_stubout_intercept_handler", fg_fb_stubout_intercept_handler },
  { "fb_rpc_intercept_handler", fg_fb_rpc_intercept_handler },
  { "fb_renamed_functions", fg_fb_renamed_functions },
  { "fb_rename_function", fg_fb_rename_function },
  { "fb_utf8ize", fg_fb_utf8ize },
  { "fb_utf8_strlen_deprecated", fg_fb_utf8_strlen_deprecated },
  { "fb_utf8_strlen", fg_fb_utf8_strlen },
  { "fb_utf8_substr", fg_fb_utf8_substr },
  { "fb_call_user_func_safe", fg_fb_call_user_func_safe },
  { "fb_call_user_func_safe_return", fg_fb_call_user_func_safe_return },
  { "fb_call_user_func_array_safe", fg_fb_call_user_func_array_safe },
  { "fb_get_code_coverage", fg_fb_get_code_coverage },
  { "fb_enable_code_coverage", fg_fb_enable_code_coverage },
  { "fb_disable_code_coverage", fg_fb_disable_code_coverage },
  { "xhprof_enable", fg_xhprof_enable },
  { "xhprof_disable", fg_xhprof_disable },
  { "xhprof_network_enable", fg_xhprof_network_enable },
  { "xhprof_network_disable", fg_xhprof_network_disable },
  { "xhprof_frame_begin", fg_xhprof_frame_begin },
  { "xhprof_frame_end", fg_xhprof_frame_end },
  { "xhprof_run_trace", fg_xhprof_run_trace },
  { "xhprof_sample_enable", fg_xhprof_sample_enable },
  { "xhprof_sample_disable", fg_xhprof_sample_disable },
  { "fb_load_local_databases", fg_fb_load_local_databases },
  { "fb_parallel_query", fg_fb_parallel_query },
  { "fb_crossall_query", fg_fb_crossall_query },
  { "fb_set_taint", fg_fb_set_taint },
  { "fb_unset_taint", fg_fb_unset_taint },
  { "fb_get_taint", fg_fb_get_taint },
  { "fb_get_taint_warning_counts", fg_fb_get_taint_warning_counts },
  { "fb_enable_html_taint_trace", fg_fb_enable_html_taint_trace },
  { "fb_const_fetch", fg_fb_const_fetch },
  { "fb_output_compression", fg_fb_output_compression },
  { "fb_set_exit_callback", fg_fb_set_exit_callback },
  { "fb_get_flush_stat", fg_fb_get_flush_stat },
  { "fb_get_last_flush_size", fg_fb_get_last_flush_size },
  { "fb_lazy_stat", fg_fb_lazy_stat },
  { "fb_lazy_lstat", fg_fb_lazy_lstat },
  { "fb_lazy_realpath", fg_fb_lazy_realpath },
  { "fb_setprofile", fg_fb_setprofile },
  { "fb_gc_collect_cycles", fg_fb_gc_collect_cycles },
  { "fb_gc_detect_cycles", fg_fb_gc_detect_cycles },
  { "fopen", fg_fopen },
  { "popen", fg_popen },
  { "fclose", fg_fclose },
  { "pclose", fg_pclose },
  { "fseek", fg_fseek },
  { "rewind", fg_rewind },
  { "ftell", fg_ftell },
  { "feof", fg_feof },
  { "fstat", fg_fstat },
  { "fread", fg_fread },
  { "fgetc", fg_fgetc },
  { "fgets", fg_fgets },
  { "fgetss", fg_fgetss },
  { "fscanf", fg_fscanf },
  { "fpassthru", fg_fpassthru },
  { "fwrite", fg_fwrite },
  { "fputs", fg_fputs },
  { "fprintf", fg_fprintf },
  { "vfprintf", fg_vfprintf },
  { "fflush", fg_fflush },
  { "ftruncate", fg_ftruncate },
  { "flock", fg_flock },
  { "fputcsv", fg_fputcsv },
  { "fgetcsv", fg_fgetcsv },
  { "file_get_contents", fg_file_get_contents },
  { "file_put_contents", fg_file_put_contents },
  { "file", fg_file },
  { "readfile", fg_readfile },
  { "move_uploaded_file", fg_move_uploaded_file },
  { "parse_ini_file", fg_parse_ini_file },
  { "parse_ini_string", fg_parse_ini_string },
  { "parse_hdf_file", fg_parse_hdf_file },
  { "parse_hdf_string", fg_parse_hdf_string },
  { "write_hdf_file", fg_write_hdf_file },
  { "write_hdf_string", fg_write_hdf_string },
  { "md5_file", fg_md5_file },
  { "sha1_file", fg_sha1_file },
  { "chmod", fg_chmod },
  { "chown", fg_chown },
  { "lchown", fg_lchown },
  { "chgrp", fg_chgrp },
  { "lchgrp", fg_lchgrp },
  { "touch", fg_touch },
  { "copy", fg_copy },
  { "rename", fg_rename },
  { "umask", fg_umask },
  { "unlink", fg_unlink },
  { "link", fg_link },
  { "symlink", fg_symlink },
  { "basename", fg_basename },
  { "fnmatch", fg_fnmatch },
  { "glob", fg_glob },
  { "tempnam", fg_tempnam },
  { "tmpfile", fg_tmpfile },
  { "fileperms", fg_fileperms },
  { "fileinode", fg_fileinode },
  { "filesize", fg_filesize },
  { "fileowner", fg_fileowner },
  { "filegroup", fg_filegroup },
  { "fileatime", fg_fileatime },
  { "filemtime", fg_filemtime },
  { "filectime", fg_filectime },
  { "filetype", fg_filetype },
  { "linkinfo", fg_linkinfo },
  { "is_writable", fg_is_writable },
  { "is_writeable", fg_is_writeable },
  { "is_readable", fg_is_readable },
  { "is_executable", fg_is_executable },
  { "is_file", fg_is_file },
  { "is_dir", fg_is_dir },
  { "is_link", fg_is_link },
  { "is_uploaded_file", fg_is_uploaded_file },
  { "file_exists", fg_file_exists },
  { "stat", fg_stat },
  { "lstat", fg_lstat },
  { "clearstatcache", fg_clearstatcache },
  { "readlink", fg_readlink },
  { "realpath", fg_realpath },
  { "pathinfo", fg_pathinfo },
  { "disk_free_space", fg_disk_free_space },
  { "diskfreespace", fg_diskfreespace },
  { "disk_total_space", fg_disk_total_space },
  { "mkdir", fg_mkdir },
  { "rmdir", fg_rmdir },
  { "dirname", fg_dirname },
  { "getcwd", fg_getcwd },
  { "chdir", fg_chdir },
  { "chroot", fg_chroot },
  { "dir", fg_dir },
  { "opendir", fg_opendir },
  { "readdir", fg_readdir },
  { "rewinddir", fg_rewinddir },
  { "scandir", fg_scandir },
  { "closedir", fg_closedir },
  { "get_defined_functions", fg_get_defined_functions },
  { "function_exists", fg_function_exists },
  { "is_callable", fg_is_callable },
  { "call_user_func_array", fg_call_user_func_array },
  { "call_user_func", fg_call_user_func },
  { "call_user_func_array_async", fg_call_user_func_array_async },
  { "call_user_func_async", fg_call_user_func_async },
  { "check_user_func_async", fg_check_user_func_async },
  { "end_user_func_async", fg_end_user_func_async },
  { "call_user_func_serialized", fg_call_user_func_serialized },
  { "call_user_func_array_rpc", fg_call_user_func_array_rpc },
  { "call_user_func_rpc", fg_call_user_func_rpc },
  { "forward_static_call_array", fg_forward_static_call_array },
  { "forward_static_call", fg_forward_static_call },
  { "get_called_class", fg_get_called_class },
  { "create_function", fg_create_function },
  { "func_get_arg", fg_func_get_arg },
  { "func_get_args", fg_func_get_args },
  { "func_num_args", fg_func_num_args },
  { "register_postsend_function", fg_register_postsend_function },
  { "register_shutdown_function", fg_register_shutdown_function },
  { "register_cleanup_function", fg_register_cleanup_function },
  { "register_tick_function", fg_register_tick_function },
  { "unregister_tick_function", fg_unregister_tick_function },
  { "hash", fg_hash },
  { "hash_algos", fg_hash_algos },
  { "hash_init", fg_hash_init },
  { "hash_file", fg_hash_file },
  { "hash_final", fg_hash_final },
  { "hash_hmac_file", fg_hash_hmac_file },
  { "hash_hmac", fg_hash_hmac },
  { "hash_update_file", fg_hash_update_file },
  { "hash_update_stream", fg_hash_update_stream },
  { "hash_update", fg_hash_update },
  { "furchash_hphp_ext", fg_furchash_hphp_ext },
  { "furchash_hphp_ext_supported", fg_furchash_hphp_ext_supported },
  { "hphp_murmurhash", fg_hphp_murmurhash },
  { "iconv_mime_encode", fg_iconv_mime_encode },
  { "iconv_mime_decode", fg_iconv_mime_decode },
  { "iconv_mime_decode_headers", fg_iconv_mime_decode_headers },
  { "iconv_get_encoding", fg_iconv_get_encoding },
  { "iconv_set_encoding", fg_iconv_set_encoding },
  { "iconv", fg_iconv },
  { "iconv_strlen", fg_iconv_strlen },
  { "iconv_strpos", fg_iconv_strpos },
  { "iconv_strrpos", fg_iconv_strrpos },
  { "iconv_substr", fg_iconv_substr },
  { "ob_iconv_handler", fg_ob_iconv_handler },
  { "icu_transliterate", fg_icu_transliterate },
  { "icu_tokenize", fg_icu_tokenize },
  { "gd_info", fg_gd_info },
  { "getimagesize", fg_getimagesize },
  { "image_type_to_extension", fg_image_type_to_extension },
  { "image_type_to_mime_type", fg_image_type_to_mime_type },
  { "image2wbmp", fg_image2wbmp },
  { "imagealphablending", fg_imagealphablending },
  { "imageantialias", fg_imageantialias },
  { "imagearc", fg_imagearc },
  { "imagechar", fg_imagechar },
  { "imagecharup", fg_imagecharup },
  { "imagecolorallocate", fg_imagecolorallocate },
  { "imagecolorallocatealpha", fg_imagecolorallocatealpha },
  { "imagecolorat", fg_imagecolorat },
  { "imagecolorclosest", fg_imagecolorclosest },
  { "imagecolorclosestalpha", fg_imagecolorclosestalpha },
  { "imagecolorclosesthwb", fg_imagecolorclosesthwb },
  { "imagecolordeallocate", fg_imagecolordeallocate },
  { "imagecolorexact", fg_imagecolorexact },
  { "imagecolorexactalpha", fg_imagecolorexactalpha },
  { "imagecolormatch", fg_imagecolormatch },
  { "imagecolorresolve", fg_imagecolorresolve },
  { "imagecolorresolvealpha", fg_imagecolorresolvealpha },
  { "imagecolorset", fg_imagecolorset },
  { "imagecolorsforindex", fg_imagecolorsforindex },
  { "imagecolorstotal", fg_imagecolorstotal },
  { "imagecolortransparent", fg_imagecolortransparent },
  { "imageconvolution", fg_imageconvolution },
  { "imagecopy", fg_imagecopy },
  { "imagecopymerge", fg_imagecopymerge },
  { "imagecopymergegray", fg_imagecopymergegray },
  { "imagecopyresampled", fg_imagecopyresampled },
  { "imagecopyresized", fg_imagecopyresized },
  { "imagecreate", fg_imagecreate },
  { "imagecreatefromgd2part", fg_imagecreatefromgd2part },
  { "imagecreatefromgd", fg_imagecreatefromgd },
  { "imagecreatefromgd2", fg_imagecreatefromgd2 },
  { "imagecreatefromgif", fg_imagecreatefromgif },
  { "imagecreatefromjpeg", fg_imagecreatefromjpeg },
  { "imagecreatefrompng", fg_imagecreatefrompng },
  { "imagecreatefromstring", fg_imagecreatefromstring },
  { "imagecreatefromwbmp", fg_imagecreatefromwbmp },
  { "imagecreatefromxbm", fg_imagecreatefromxbm },
  { "imagecreatefromxpm", fg_imagecreatefromxpm },
  { "imagecreatetruecolor", fg_imagecreatetruecolor },
  { "imagedashedline", fg_imagedashedline },
  { "imagedestroy", fg_imagedestroy },
  { "imageellipse", fg_imageellipse },
  { "imagefill", fg_imagefill },
  { "imagefilledarc", fg_imagefilledarc },
  { "imagefilledellipse", fg_imagefilledellipse },
  { "imagefilledpolygon", fg_imagefilledpolygon },
  { "imagefilledrectangle", fg_imagefilledrectangle },
  { "imagefilltoborder", fg_imagefilltoborder },
  { "imagefilter", fg_imagefilter },
  { "imagefontheight", fg_imagefontheight },
  { "imagefontwidth", fg_imagefontwidth },
  { "imageftbbox", fg_imageftbbox },
  { "imagefttext", fg_imagefttext },
  { "imagegammacorrect", fg_imagegammacorrect },
  { "imagegd2", fg_imagegd2 },
  { "imagegd", fg_imagegd },
  { "imagegif", fg_imagegif },
  { "imagegrabscreen", fg_imagegrabscreen },
  { "imagegrabwindow", fg_imagegrabwindow },
  { "imageinterlace", fg_imageinterlace },
  { "imageistruecolor", fg_imageistruecolor },
  { "imagejpeg", fg_imagejpeg },
  { "imagelayereffect", fg_imagelayereffect },
  { "imageline", fg_imageline },
  { "imageloadfont", fg_imageloadfont },
  { "imagepalettecopy", fg_imagepalettecopy },
  { "imagepng", fg_imagepng },
  { "imagepolygon", fg_imagepolygon },
  { "imagepsbbox", fg_imagepsbbox },
  { "imagepsencodefont", fg_imagepsencodefont },
  { "imagepsextendfont", fg_imagepsextendfont },
  { "imagepsfreefont", fg_imagepsfreefont },
  { "imagepsloadfont", fg_imagepsloadfont },
  { "imagepsslantfont", fg_imagepsslantfont },
  { "imagepstext", fg_imagepstext },
  { "imagerectangle", fg_imagerectangle },
  { "imagerotate", fg_imagerotate },
  { "imagesavealpha", fg_imagesavealpha },
  { "imagesetbrush", fg_imagesetbrush },
  { "imagesetpixel", fg_imagesetpixel },
  { "imagesetstyle", fg_imagesetstyle },
  { "imagesetthickness", fg_imagesetthickness },
  { "imagesettile", fg_imagesettile },
  { "imagestring", fg_imagestring },
  { "imagestringup", fg_imagestringup },
  { "imagesx", fg_imagesx },
  { "imagesy", fg_imagesy },
  { "imagetruecolortopalette", fg_imagetruecolortopalette },
  { "imagettfbbox", fg_imagettfbbox },
  { "imagettftext", fg_imagettftext },
  { "imagetypes", fg_imagetypes },
  { "imagewbmp", fg_imagewbmp },
  { "imagexbm", fg_imagexbm },
  { "iptcembed", fg_iptcembed },
  { "iptcparse", fg_iptcparse },
  { "jpeg2wbmp", fg_jpeg2wbmp },
  { "png2wbmp", fg_png2wbmp },
  { "exif_imagetype", fg_exif_imagetype },
  { "exif_read_data", fg_exif_read_data },
  { "read_exif_data", fg_read_exif_data },
  { "exif_tagname", fg_exif_tagname },
  { "exif_thumbnail", fg_exif_thumbnail },
  { "imap_8bit", fg_imap_8bit },
  { "imap_alerts", fg_imap_alerts },
  { "imap_append", fg_imap_append },
  { "imap_base64", fg_imap_base64 },
  { "imap_binary", fg_imap_binary },
  { "imap_body", fg_imap_body },
  { "imap_bodystruct", fg_imap_bodystruct },
  { "imap_check", fg_imap_check },
  { "imap_clearflag_full", fg_imap_clearflag_full },
  { "imap_close", fg_imap_close },
  { "imap_createmailbox", fg_imap_createmailbox },
  { "imap_delete", fg_imap_delete },
  { "imap_deletemailbox", fg_imap_deletemailbox },
  { "imap_errors", fg_imap_errors },
  { "imap_expunge", fg_imap_expunge },
  { "imap_fetch_overview", fg_imap_fetch_overview },
  { "imap_fetchbody", fg_imap_fetchbody },
  { "imap_fetchheader", fg_imap_fetchheader },
  { "imap_fetchstructure", fg_imap_fetchstructure },
  { "imap_gc", fg_imap_gc },
  { "imap_get_quota", fg_imap_get_quota },
  { "imap_get_quotaroot", fg_imap_get_quotaroot },
  { "imap_getacl", fg_imap_getacl },
  { "imap_getmailboxes", fg_imap_getmailboxes },
  { "imap_getsubscribed", fg_imap_getsubscribed },
  { "imap_header", fg_imap_header },
  { "imap_headerinfo", fg_imap_headerinfo },
  { "imap_headers", fg_imap_headers },
  { "imap_last_error", fg_imap_last_error },
  { "imap_list", fg_imap_list },
  { "imap_listmailbox", fg_imap_listmailbox },
  { "imap_listscan", fg_imap_listscan },
  { "imap_listsubscribed", fg_imap_listsubscribed },
  { "imap_lsub", fg_imap_lsub },
  { "imap_mail_compose", fg_imap_mail_compose },
  { "imap_mail_copy", fg_imap_mail_copy },
  { "imap_mail_move", fg_imap_mail_move },
  { "imap_mail", fg_imap_mail },
  { "imap_mailboxmsginfo", fg_imap_mailboxmsginfo },
  { "imap_mime_header_decode", fg_imap_mime_header_decode },
  { "imap_msgno", fg_imap_msgno },
  { "imap_num_msg", fg_imap_num_msg },
  { "imap_num_recent", fg_imap_num_recent },
  { "imap_open", fg_imap_open },
  { "imap_ping", fg_imap_ping },
  { "imap_qprint", fg_imap_qprint },
  { "imap_renamemailbox", fg_imap_renamemailbox },
  { "imap_reopen", fg_imap_reopen },
  { "imap_rfc822_parse_adrlist", fg_imap_rfc822_parse_adrlist },
  { "imap_rfc822_parse_headers", fg_imap_rfc822_parse_headers },
  { "imap_rfc822_write_address", fg_imap_rfc822_write_address },
  { "imap_savebody", fg_imap_savebody },
  { "imap_scanmailbox", fg_imap_scanmailbox },
  { "imap_search", fg_imap_search },
  { "imap_set_quota", fg_imap_set_quota },
  { "imap_setacl", fg_imap_setacl },
  { "imap_setflag_full", fg_imap_setflag_full },
  { "imap_sort", fg_imap_sort },
  { "imap_status", fg_imap_status },
  { "imap_subscribe", fg_imap_subscribe },
  { "imap_thread", fg_imap_thread },
  { "imap_timeout", fg_imap_timeout },
  { "imap_uid", fg_imap_uid },
  { "imap_undelete", fg_imap_undelete },
  { "imap_unsubscribe", fg_imap_unsubscribe },
  { "imap_utf7_decode", fg_imap_utf7_decode },
  { "imap_utf7_encode", fg_imap_utf7_encode },
  { "imap_utf8", fg_imap_utf8 },
  { "intl_get_error_code", fg_intl_get_error_code },
  { "intl_get_error_message", fg_intl_get_error_message },
  { "intl_error_name", fg_intl_error_name },
  { "intl_is_failure", fg_intl_is_failure },
  { "collator_asort", fg_collator_asort },
  { "collator_compare", fg_collator_compare },
  { "collator_create", fg_collator_create },
  { "collator_get_attribute", fg_collator_get_attribute },
  { "collator_get_error_code", fg_collator_get_error_code },
  { "collator_get_error_message", fg_collator_get_error_message },
  { "collator_get_locale", fg_collator_get_locale },
  { "collator_get_strength", fg_collator_get_strength },
  { "collator_set_attribute", fg_collator_set_attribute },
  { "collator_set_strength", fg_collator_set_strength },
  { "collator_sort_with_sort_keys", fg_collator_sort_with_sort_keys },
  { "collator_sort", fg_collator_sort },
  { "idn_to_ascii", fg_idn_to_ascii },
  { "idn_to_unicode", fg_idn_to_unicode },
  { "idn_to_utf8", fg_idn_to_utf8 },
  { "ftok", fg_ftok },
  { "msg_get_queue", fg_msg_get_queue },
  { "msg_queue_exists", fg_msg_queue_exists },
  { "msg_send", fg_msg_send },
  { "msg_receive", fg_msg_receive },
  { "msg_remove_queue", fg_msg_remove_queue },
  { "msg_set_queue", fg_msg_set_queue },
  { "msg_stat_queue", fg_msg_stat_queue },
  { "sem_acquire", fg_sem_acquire },
  { "sem_get", fg_sem_get },
  { "sem_release", fg_sem_release },
  { "sem_remove", fg_sem_remove },
  { "shm_attach", fg_shm_attach },
  { "shm_detach", fg_shm_detach },
  { "shm_remove", fg_shm_remove },
  { "shm_get_var", fg_shm_get_var },
  { "shm_has_var", fg_shm_has_var },
  { "shm_put_var", fg_shm_put_var },
  { "shm_remove_var", fg_shm_remove_var },
  { "hphp_recursiveiteratoriterator___construct", fg_hphp_recursiveiteratoriterator___construct },
  { "hphp_recursiveiteratoriterator_getinneriterator", fg_hphp_recursiveiteratoriterator_getinneriterator },
  { "hphp_recursiveiteratoriterator_current", fg_hphp_recursiveiteratoriterator_current },
  { "hphp_recursiveiteratoriterator_key", fg_hphp_recursiveiteratoriterator_key },
  { "hphp_recursiveiteratoriterator_next", fg_hphp_recursiveiteratoriterator_next },
  { "hphp_recursiveiteratoriterator_rewind", fg_hphp_recursiveiteratoriterator_rewind },
  { "hphp_recursiveiteratoriterator_valid", fg_hphp_recursiveiteratoriterator_valid },
  { "hphp_directoryiterator___construct", fg_hphp_directoryiterator___construct },
  { "hphp_directoryiterator_key", fg_hphp_directoryiterator_key },
  { "hphp_directoryiterator_next", fg_hphp_directoryiterator_next },
  { "hphp_directoryiterator_rewind", fg_hphp_directoryiterator_rewind },
  { "hphp_directoryiterator_seek", fg_hphp_directoryiterator_seek },
  { "hphp_directoryiterator_current", fg_hphp_directoryiterator_current },
  { "hphp_directoryiterator___tostring", fg_hphp_directoryiterator___tostring },
  { "hphp_directoryiterator_valid", fg_hphp_directoryiterator_valid },
  { "hphp_directoryiterator_isdot", fg_hphp_directoryiterator_isdot },
  { "hphp_recursivedirectoryiterator___construct", fg_hphp_recursivedirectoryiterator___construct },
  { "hphp_recursivedirectoryiterator_key", fg_hphp_recursivedirectoryiterator_key },
  { "hphp_recursivedirectoryiterator_next", fg_hphp_recursivedirectoryiterator_next },
  { "hphp_recursivedirectoryiterator_rewind", fg_hphp_recursivedirectoryiterator_rewind },
  { "hphp_recursivedirectoryiterator_seek", fg_hphp_recursivedirectoryiterator_seek },
  { "hphp_recursivedirectoryiterator_current", fg_hphp_recursivedirectoryiterator_current },
  { "hphp_recursivedirectoryiterator___tostring", fg_hphp_recursivedirectoryiterator___tostring },
  { "hphp_recursivedirectoryiterator_valid", fg_hphp_recursivedirectoryiterator_valid },
  { "hphp_recursivedirectoryiterator_haschildren", fg_hphp_recursivedirectoryiterator_haschildren },
  { "hphp_recursivedirectoryiterator_getchildren", fg_hphp_recursivedirectoryiterator_getchildren },
  { "hphp_recursivedirectoryiterator_getsubpath", fg_hphp_recursivedirectoryiterator_getsubpath },
  { "hphp_recursivedirectoryiterator_getsubpathname", fg_hphp_recursivedirectoryiterator_getsubpathname },
  { "json_encode", fg_json_encode },
  { "json_decode", fg_json_decode },
  { "ldap_connect", fg_ldap_connect },
  { "ldap_explode_dn", fg_ldap_explode_dn },
  { "ldap_dn2ufn", fg_ldap_dn2ufn },
  { "ldap_err2str", fg_ldap_err2str },
  { "ldap_add", fg_ldap_add },
  { "ldap_mod_add", fg_ldap_mod_add },
  { "ldap_mod_del", fg_ldap_mod_del },
  { "ldap_mod_replace", fg_ldap_mod_replace },
  { "ldap_modify", fg_ldap_modify },
  { "ldap_bind", fg_ldap_bind },
  { "ldap_set_rebind_proc", fg_ldap_set_rebind_proc },
  { "ldap_sort", fg_ldap_sort },
  { "ldap_start_tls", fg_ldap_start_tls },
  { "ldap_unbind", fg_ldap_unbind },
  { "ldap_get_option", fg_ldap_get_option },
  { "ldap_set_option", fg_ldap_set_option },
  { "ldap_close", fg_ldap_close },
  { "ldap_list", fg_ldap_list },
  { "ldap_read", fg_ldap_read },
  { "ldap_search", fg_ldap_search },
  { "ldap_rename", fg_ldap_rename },
  { "ldap_delete", fg_ldap_delete },
  { "ldap_compare", fg_ldap_compare },
  { "ldap_errno", fg_ldap_errno },
  { "ldap_error", fg_ldap_error },
  { "ldap_get_dn", fg_ldap_get_dn },
  { "ldap_count_entries", fg_ldap_count_entries },
  { "ldap_get_entries", fg_ldap_get_entries },
  { "ldap_first_entry", fg_ldap_first_entry },
  { "ldap_next_entry", fg_ldap_next_entry },
  { "ldap_get_attributes", fg_ldap_get_attributes },
  { "ldap_first_attribute", fg_ldap_first_attribute },
  { "ldap_next_attribute", fg_ldap_next_attribute },
  { "ldap_first_reference", fg_ldap_first_reference },
  { "ldap_next_reference", fg_ldap_next_reference },
  { "ldap_parse_reference", fg_ldap_parse_reference },
  { "ldap_parse_result", fg_ldap_parse_result },
  { "ldap_free_result", fg_ldap_free_result },
  { "ldap_get_values_len", fg_ldap_get_values_len },
  { "ldap_get_values", fg_ldap_get_values },
  { "magickgetcopyright", fg_magickgetcopyright },
  { "magickgethomeurl", fg_magickgethomeurl },
  { "magickgetpackagename", fg_magickgetpackagename },
  { "magickgetquantumdepth", fg_magickgetquantumdepth },
  { "magickgetreleasedate", fg_magickgetreleasedate },
  { "magickgetresourcelimit", fg_magickgetresourcelimit },
  { "magickgetversion", fg_magickgetversion },
  { "magickgetversionnumber", fg_magickgetversionnumber },
  { "magickgetversionstring", fg_magickgetversionstring },
  { "magickqueryconfigureoption", fg_magickqueryconfigureoption },
  { "magickqueryconfigureoptions", fg_magickqueryconfigureoptions },
  { "magickqueryfonts", fg_magickqueryfonts },
  { "magickqueryformats", fg_magickqueryformats },
  { "magicksetresourcelimit", fg_magicksetresourcelimit },
  { "newdrawingwand", fg_newdrawingwand },
  { "newmagickwand", fg_newmagickwand },
  { "newpixeliterator", fg_newpixeliterator },
  { "newpixelregioniterator", fg_newpixelregioniterator },
  { "newpixelwand", fg_newpixelwand },
  { "newpixelwandarray", fg_newpixelwandarray },
  { "newpixelwands", fg_newpixelwands },
  { "destroydrawingwand", fg_destroydrawingwand },
  { "destroymagickwand", fg_destroymagickwand },
  { "destroypixeliterator", fg_destroypixeliterator },
  { "destroypixelwand", fg_destroypixelwand },
  { "destroypixelwandarray", fg_destroypixelwandarray },
  { "destroypixelwands", fg_destroypixelwands },
  { "isdrawingwand", fg_isdrawingwand },
  { "ismagickwand", fg_ismagickwand },
  { "ispixeliterator", fg_ispixeliterator },
  { "ispixelwand", fg_ispixelwand },
  { "cleardrawingwand", fg_cleardrawingwand },
  { "clearmagickwand", fg_clearmagickwand },
  { "clearpixeliterator", fg_clearpixeliterator },
  { "clearpixelwand", fg_clearpixelwand },
  { "clonedrawingwand", fg_clonedrawingwand },
  { "clonemagickwand", fg_clonemagickwand },
  { "wandgetexception", fg_wandgetexception },
  { "wandgetexceptionstring", fg_wandgetexceptionstring },
  { "wandgetexceptiontype", fg_wandgetexceptiontype },
  { "wandhasexception", fg_wandhasexception },
  { "drawaffine", fg_drawaffine },
  { "drawannotation", fg_drawannotation },
  { "drawarc", fg_drawarc },
  { "drawbezier", fg_drawbezier },
  { "drawcircle", fg_drawcircle },
  { "drawcolor", fg_drawcolor },
  { "drawcomment", fg_drawcomment },
  { "drawcomposite", fg_drawcomposite },
  { "drawellipse", fg_drawellipse },
  { "drawgetclippath", fg_drawgetclippath },
  { "drawgetcliprule", fg_drawgetcliprule },
  { "drawgetclipunits", fg_drawgetclipunits },
  { "drawgetexception", fg_drawgetexception },
  { "drawgetexceptionstring", fg_drawgetexceptionstring },
  { "drawgetexceptiontype", fg_drawgetexceptiontype },
  { "drawgetfillalpha", fg_drawgetfillalpha },
  { "drawgetfillcolor", fg_drawgetfillcolor },
  { "drawgetfillopacity", fg_drawgetfillopacity },
  { "drawgetfillrule", fg_drawgetfillrule },
  { "drawgetfont", fg_drawgetfont },
  { "drawgetfontfamily", fg_drawgetfontfamily },
  { "drawgetfontsize", fg_drawgetfontsize },
  { "drawgetfontstretch", fg_drawgetfontstretch },
  { "drawgetfontstyle", fg_drawgetfontstyle },
  { "drawgetfontweight", fg_drawgetfontweight },
  { "drawgetgravity", fg_drawgetgravity },
  { "drawgetstrokealpha", fg_drawgetstrokealpha },
  { "drawgetstrokeantialias", fg_drawgetstrokeantialias },
  { "drawgetstrokecolor", fg_drawgetstrokecolor },
  { "drawgetstrokedasharray", fg_drawgetstrokedasharray },
  { "drawgetstrokedashoffset", fg_drawgetstrokedashoffset },
  { "drawgetstrokelinecap", fg_drawgetstrokelinecap },
  { "drawgetstrokelinejoin", fg_drawgetstrokelinejoin },
  { "drawgetstrokemiterlimit", fg_drawgetstrokemiterlimit },
  { "drawgetstrokeopacity", fg_drawgetstrokeopacity },
  { "drawgetstrokewidth", fg_drawgetstrokewidth },
  { "drawgettextalignment", fg_drawgettextalignment },
  { "drawgettextantialias", fg_drawgettextantialias },
  { "drawgettextdecoration", fg_drawgettextdecoration },
  { "drawgettextencoding", fg_drawgettextencoding },
  { "drawgettextundercolor", fg_drawgettextundercolor },
  { "drawgetvectorgraphics", fg_drawgetvectorgraphics },
  { "drawline", fg_drawline },
  { "drawmatte", fg_drawmatte },
  { "drawpathclose", fg_drawpathclose },
  { "drawpathcurvetoabsolute", fg_drawpathcurvetoabsolute },
  { "drawpathcurvetoquadraticbezierabsolute", fg_drawpathcurvetoquadraticbezierabsolute },
  { "drawpathcurvetoquadraticbezierrelative", fg_drawpathcurvetoquadraticbezierrelative },
  { "drawpathcurvetoquadraticbeziersmoothabsolute", fg_drawpathcurvetoquadraticbeziersmoothabsolute },
  { "drawpathcurvetoquadraticbeziersmoothrelative", fg_drawpathcurvetoquadraticbeziersmoothrelative },
  { "drawpathcurvetorelative", fg_drawpathcurvetorelative },
  { "drawpathcurvetosmoothabsolute", fg_drawpathcurvetosmoothabsolute },
  { "drawpathcurvetosmoothrelative", fg_drawpathcurvetosmoothrelative },
  { "drawpathellipticarcabsolute", fg_drawpathellipticarcabsolute },
  { "drawpathellipticarcrelative", fg_drawpathellipticarcrelative },
  { "drawpathfinish", fg_drawpathfinish },
  { "drawpathlinetoabsolute", fg_drawpathlinetoabsolute },
  { "drawpathlinetohorizontalabsolute", fg_drawpathlinetohorizontalabsolute },
  { "drawpathlinetohorizontalrelative", fg_drawpathlinetohorizontalrelative },
  { "drawpathlinetorelative", fg_drawpathlinetorelative },
  { "drawpathlinetoverticalabsolute", fg_drawpathlinetoverticalabsolute },
  { "drawpathlinetoverticalrelative", fg_drawpathlinetoverticalrelative },
  { "drawpathmovetoabsolute", fg_drawpathmovetoabsolute },
  { "drawpathmovetorelative", fg_drawpathmovetorelative },
  { "drawpathstart", fg_drawpathstart },
  { "drawpoint", fg_drawpoint },
  { "drawpolygon", fg_drawpolygon },
  { "drawpolyline", fg_drawpolyline },
  { "drawrectangle", fg_drawrectangle },
  { "drawrender", fg_drawrender },
  { "drawrotate", fg_drawrotate },
  { "drawroundrectangle", fg_drawroundrectangle },
  { "drawscale", fg_drawscale },
  { "drawsetclippath", fg_drawsetclippath },
  { "drawsetcliprule", fg_drawsetcliprule },
  { "drawsetclipunits", fg_drawsetclipunits },
  { "drawsetfillalpha", fg_drawsetfillalpha },
  { "drawsetfillcolor", fg_drawsetfillcolor },
  { "drawsetfillopacity", fg_drawsetfillopacity },
  { "drawsetfillpatternurl", fg_drawsetfillpatternurl },
  { "drawsetfillrule", fg_drawsetfillrule },
  { "drawsetfont", fg_drawsetfont },
  { "drawsetfontfamily", fg_drawsetfontfamily },
  { "drawsetfontsize", fg_drawsetfontsize },
  { "drawsetfontstretch", fg_drawsetfontstretch },
  { "drawsetfontstyle", fg_drawsetfontstyle },
  { "drawsetfontweight", fg_drawsetfontweight },
  { "drawsetgravity", fg_drawsetgravity },
  { "drawsetstrokealpha", fg_drawsetstrokealpha },
  { "drawsetstrokeantialias", fg_drawsetstrokeantialias },
  { "drawsetstrokecolor", fg_drawsetstrokecolor },
  { "drawsetstrokedasharray", fg_drawsetstrokedasharray },
  { "drawsetstrokedashoffset", fg_drawsetstrokedashoffset },
  { "drawsetstrokelinecap", fg_drawsetstrokelinecap },
  { "drawsetstrokelinejoin", fg_drawsetstrokelinejoin },
  { "drawsetstrokemiterlimit", fg_drawsetstrokemiterlimit },
  { "drawsetstrokeopacity", fg_drawsetstrokeopacity },
  { "drawsetstrokepatternurl", fg_drawsetstrokepatternurl },
  { "drawsetstrokewidth", fg_drawsetstrokewidth },
  { "drawsettextalignment", fg_drawsettextalignment },
  { "drawsettextantialias", fg_drawsettextantialias },
  { "drawsettextdecoration", fg_drawsettextdecoration },
  { "drawsettextencoding", fg_drawsettextencoding },
  { "drawsettextundercolor", fg_drawsettextundercolor },
  { "drawsetvectorgraphics", fg_drawsetvectorgraphics },
  { "drawsetviewbox", fg_drawsetviewbox },
  { "drawskewx", fg_drawskewx },
  { "drawskewy", fg_drawskewy },
  { "drawtranslate", fg_drawtranslate },
  { "pushdrawingwand", fg_pushdrawingwand },
  { "drawpushclippath", fg_drawpushclippath },
  { "drawpushdefs", fg_drawpushdefs },
  { "drawpushpattern", fg_drawpushpattern },
  { "popdrawingwand", fg_popdrawingwand },
  { "drawpopclippath", fg_drawpopclippath },
  { "drawpopdefs", fg_drawpopdefs },
  { "drawpoppattern", fg_drawpoppattern },
  { "magickadaptivethresholdimage", fg_magickadaptivethresholdimage },
  { "magickaddimage", fg_magickaddimage },
  { "magickaddnoiseimage", fg_magickaddnoiseimage },
  { "magickaffinetransformimage", fg_magickaffinetransformimage },
  { "magickannotateimage", fg_magickannotateimage },
  { "magickappendimages", fg_magickappendimages },
  { "magickaverageimages", fg_magickaverageimages },
  { "magickblackthresholdimage", fg_magickblackthresholdimage },
  { "magickblurimage", fg_magickblurimage },
  { "magickborderimage", fg_magickborderimage },
  { "magickcharcoalimage", fg_magickcharcoalimage },
  { "magickchopimage", fg_magickchopimage },
  { "magickclipimage", fg_magickclipimage },
  { "magickclippathimage", fg_magickclippathimage },
  { "magickcoalesceimages", fg_magickcoalesceimages },
  { "magickcolorfloodfillimage", fg_magickcolorfloodfillimage },
  { "magickcolorizeimage", fg_magickcolorizeimage },
  { "magickcombineimages", fg_magickcombineimages },
  { "magickcommentimage", fg_magickcommentimage },
  { "magickcompareimages", fg_magickcompareimages },
  { "magickcompositeimage", fg_magickcompositeimage },
  { "magickconstituteimage", fg_magickconstituteimage },
  { "magickcontrastimage", fg_magickcontrastimage },
  { "magickconvolveimage", fg_magickconvolveimage },
  { "magickcropimage", fg_magickcropimage },
  { "magickcyclecolormapimage", fg_magickcyclecolormapimage },
  { "magickdeconstructimages", fg_magickdeconstructimages },
  { "magickdescribeimage", fg_magickdescribeimage },
  { "magickdespeckleimage", fg_magickdespeckleimage },
  { "magickdrawimage", fg_magickdrawimage },
  { "magickechoimageblob", fg_magickechoimageblob },
  { "magickechoimagesblob", fg_magickechoimagesblob },
  { "magickedgeimage", fg_magickedgeimage },
  { "magickembossimage", fg_magickembossimage },
  { "magickenhanceimage", fg_magickenhanceimage },
  { "magickequalizeimage", fg_magickequalizeimage },
  { "magickevaluateimage", fg_magickevaluateimage },
  { "magickflattenimages", fg_magickflattenimages },
  { "magickflipimage", fg_magickflipimage },
  { "magickflopimage", fg_magickflopimage },
  { "magickframeimage", fg_magickframeimage },
  { "magickfximage", fg_magickfximage },
  { "magickgammaimage", fg_magickgammaimage },
  { "magickgaussianblurimage", fg_magickgaussianblurimage },
  { "magickgetcharheight", fg_magickgetcharheight },
  { "magickgetcharwidth", fg_magickgetcharwidth },
  { "magickgetexception", fg_magickgetexception },
  { "magickgetexceptionstring", fg_magickgetexceptionstring },
  { "magickgetexceptiontype", fg_magickgetexceptiontype },
  { "magickgetfilename", fg_magickgetfilename },
  { "magickgetformat", fg_magickgetformat },
  { "magickgetimage", fg_magickgetimage },
  { "magickgetimagebackgroundcolor", fg_magickgetimagebackgroundcolor },
  { "magickgetimageblob", fg_magickgetimageblob },
  { "magickgetimageblueprimary", fg_magickgetimageblueprimary },
  { "magickgetimagebordercolor", fg_magickgetimagebordercolor },
  { "magickgetimagechannelmean", fg_magickgetimagechannelmean },
  { "magickgetimagecolormapcolor", fg_magickgetimagecolormapcolor },
  { "magickgetimagecolors", fg_magickgetimagecolors },
  { "magickgetimagecolorspace", fg_magickgetimagecolorspace },
  { "magickgetimagecompose", fg_magickgetimagecompose },
  { "magickgetimagecompression", fg_magickgetimagecompression },
  { "magickgetimagecompressionquality", fg_magickgetimagecompressionquality },
  { "magickgetimagedelay", fg_magickgetimagedelay },
  { "magickgetimagedepth", fg_magickgetimagedepth },
  { "magickgetimagedispose", fg_magickgetimagedispose },
  { "magickgetimageextrema", fg_magickgetimageextrema },
  { "magickgetimagefilename", fg_magickgetimagefilename },
  { "magickgetimageformat", fg_magickgetimageformat },
  { "magickgetimagegamma", fg_magickgetimagegamma },
  { "magickgetimagegreenprimary", fg_magickgetimagegreenprimary },
  { "magickgetimageheight", fg_magickgetimageheight },
  { "magickgetimagehistogram", fg_magickgetimagehistogram },
  { "magickgetimageindex", fg_magickgetimageindex },
  { "magickgetimageinterlacescheme", fg_magickgetimageinterlacescheme },
  { "magickgetimageiterations", fg_magickgetimageiterations },
  { "magickgetimagemattecolor", fg_magickgetimagemattecolor },
  { "magickgetimagemimetype", fg_magickgetimagemimetype },
  { "magickgetimagepixels", fg_magickgetimagepixels },
  { "magickgetimageprofile", fg_magickgetimageprofile },
  { "magickgetimageredprimary", fg_magickgetimageredprimary },
  { "magickgetimagerenderingintent", fg_magickgetimagerenderingintent },
  { "magickgetimageresolution", fg_magickgetimageresolution },
  { "magickgetimagescene", fg_magickgetimagescene },
  { "magickgetimagesignature", fg_magickgetimagesignature },
  { "magickgetimagesize", fg_magickgetimagesize },
  { "magickgetimagetype", fg_magickgetimagetype },
  { "magickgetimageunits", fg_magickgetimageunits },
  { "magickgetimagevirtualpixelmethod", fg_magickgetimagevirtualpixelmethod },
  { "magickgetimagewhitepoint", fg_magickgetimagewhitepoint },
  { "magickgetimagewidth", fg_magickgetimagewidth },
  { "magickgetimagesblob", fg_magickgetimagesblob },
  { "magickgetinterlacescheme", fg_magickgetinterlacescheme },
  { "magickgetmaxtextadvance", fg_magickgetmaxtextadvance },
  { "magickgetmimetype", fg_magickgetmimetype },
  { "magickgetnumberimages", fg_magickgetnumberimages },
  { "magickgetsamplingfactors", fg_magickgetsamplingfactors },
  { "magickgetsize", fg_magickgetsize },
  { "magickgetstringheight", fg_magickgetstringheight },
  { "magickgetstringwidth", fg_magickgetstringwidth },
  { "magickgettextascent", fg_magickgettextascent },
  { "magickgettextdescent", fg_magickgettextdescent },
  { "magickgetwandsize", fg_magickgetwandsize },
  { "magickhasnextimage", fg_magickhasnextimage },
  { "magickhaspreviousimage", fg_magickhaspreviousimage },
  { "magickimplodeimage", fg_magickimplodeimage },
  { "magicklabelimage", fg_magicklabelimage },
  { "magicklevelimage", fg_magicklevelimage },
  { "magickmagnifyimage", fg_magickmagnifyimage },
  { "magickmapimage", fg_magickmapimage },
  { "magickmattefloodfillimage", fg_magickmattefloodfillimage },
  { "magickmedianfilterimage", fg_magickmedianfilterimage },
  { "magickminifyimage", fg_magickminifyimage },
  { "magickmodulateimage", fg_magickmodulateimage },
  { "magickmontageimage", fg_magickmontageimage },
  { "magickmorphimages", fg_magickmorphimages },
  { "magickmosaicimages", fg_magickmosaicimages },
  { "magickmotionblurimage", fg_magickmotionblurimage },
  { "magicknegateimage", fg_magicknegateimage },
  { "magicknewimage", fg_magicknewimage },
  { "magicknextimage", fg_magicknextimage },
  { "magicknormalizeimage", fg_magicknormalizeimage },
  { "magickoilpaintimage", fg_magickoilpaintimage },
  { "magickpaintopaqueimage", fg_magickpaintopaqueimage },
  { "magickpainttransparentimage", fg_magickpainttransparentimage },
  { "magickpingimage", fg_magickpingimage },
  { "magickposterizeimage", fg_magickposterizeimage },
  { "magickpreviewimages", fg_magickpreviewimages },
  { "magickpreviousimage", fg_magickpreviousimage },
  { "magickprofileimage", fg_magickprofileimage },
  { "magickquantizeimage", fg_magickquantizeimage },
  { "magickquantizeimages", fg_magickquantizeimages },
  { "magickqueryfontmetrics", fg_magickqueryfontmetrics },
  { "magickradialblurimage", fg_magickradialblurimage },
  { "magickraiseimage", fg_magickraiseimage },
  { "magickreadimage", fg_magickreadimage },
  { "magickreadimageblob", fg_magickreadimageblob },
  { "magickreadimagefile", fg_magickreadimagefile },
  { "magickreadimages", fg_magickreadimages },
  { "magickreducenoiseimage", fg_magickreducenoiseimage },
  { "magickremoveimage", fg_magickremoveimage },
  { "magickremoveimageprofile", fg_magickremoveimageprofile },
  { "magickremoveimageprofiles", fg_magickremoveimageprofiles },
  { "magickresampleimage", fg_magickresampleimage },
  { "magickresetiterator", fg_magickresetiterator },
  { "magickresizeimage", fg_magickresizeimage },
  { "magickrollimage", fg_magickrollimage },
  { "magickrotateimage", fg_magickrotateimage },
  { "magicksampleimage", fg_magicksampleimage },
  { "magickscaleimage", fg_magickscaleimage },
  { "magickseparateimagechannel", fg_magickseparateimagechannel },
  { "magicksetcompressionquality", fg_magicksetcompressionquality },
  { "magicksetfilename", fg_magicksetfilename },
  { "magicksetfirstiterator", fg_magicksetfirstiterator },
  { "magicksetformat", fg_magicksetformat },
  { "magicksetimage", fg_magicksetimage },
  { "magicksetimagebackgroundcolor", fg_magicksetimagebackgroundcolor },
  { "magicksetimagebias", fg_magicksetimagebias },
  { "magicksetimageblueprimary", fg_magicksetimageblueprimary },
  { "magicksetimagebordercolor", fg_magicksetimagebordercolor },
  { "magicksetimagecolormapcolor", fg_magicksetimagecolormapcolor },
  { "magicksetimagecolorspace", fg_magicksetimagecolorspace },
  { "magicksetimagecompose", fg_magicksetimagecompose },
  { "magicksetimagecompression", fg_magicksetimagecompression },
  { "magicksetimagecompressionquality", fg_magicksetimagecompressionquality },
  { "magicksetimagedelay", fg_magicksetimagedelay },
  { "magicksetimagedepth", fg_magicksetimagedepth },
  { "magicksetimagedispose", fg_magicksetimagedispose },
  { "magicksetimagefilename", fg_magicksetimagefilename },
  { "magicksetimageformat", fg_magicksetimageformat },
  { "magicksetimagegamma", fg_magicksetimagegamma },
  { "magicksetimagegreenprimary", fg_magicksetimagegreenprimary },
  { "magicksetimageindex", fg_magicksetimageindex },
  { "magicksetimageinterlacescheme", fg_magicksetimageinterlacescheme },
  { "magicksetimageiterations", fg_magicksetimageiterations },
  { "magicksetimagemattecolor", fg_magicksetimagemattecolor },
  { "magicksetimageoption", fg_magicksetimageoption },
  { "magicksetimagepixels", fg_magicksetimagepixels },
  { "magicksetimageprofile", fg_magicksetimageprofile },
  { "magicksetimageredprimary", fg_magicksetimageredprimary },
  { "magicksetimagerenderingintent", fg_magicksetimagerenderingintent },
  { "magicksetimageresolution", fg_magicksetimageresolution },
  { "magicksetimagescene", fg_magicksetimagescene },
  { "magicksetimagetype", fg_magicksetimagetype },
  { "magicksetimageunits", fg_magicksetimageunits },
  { "magicksetimagevirtualpixelmethod", fg_magicksetimagevirtualpixelmethod },
  { "magicksetimagewhitepoint", fg_magicksetimagewhitepoint },
  { "magicksetinterlacescheme", fg_magicksetinterlacescheme },
  { "magicksetlastiterator", fg_magicksetlastiterator },
  { "magicksetpassphrase", fg_magicksetpassphrase },
  { "magicksetresolution", fg_magicksetresolution },
  { "magicksetsamplingfactors", fg_magicksetsamplingfactors },
  { "magicksetsize", fg_magicksetsize },
  { "magicksetwandsize", fg_magicksetwandsize },
  { "magicksharpenimage", fg_magicksharpenimage },
  { "magickshaveimage", fg_magickshaveimage },
  { "magickshearimage", fg_magickshearimage },
  { "magicksolarizeimage", fg_magicksolarizeimage },
  { "magickspliceimage", fg_magickspliceimage },
  { "magickspreadimage", fg_magickspreadimage },
  { "magicksteganoimage", fg_magicksteganoimage },
  { "magickstereoimage", fg_magickstereoimage },
  { "magickstripimage", fg_magickstripimage },
  { "magickswirlimage", fg_magickswirlimage },
  { "magicktextureimage", fg_magicktextureimage },
  { "magickthresholdimage", fg_magickthresholdimage },
  { "magicktintimage", fg_magicktintimage },
  { "magicktransformimage", fg_magicktransformimage },
  { "magicktrimimage", fg_magicktrimimage },
  { "magickunsharpmaskimage", fg_magickunsharpmaskimage },
  { "magickwaveimage", fg_magickwaveimage },
  { "magickwhitethresholdimage", fg_magickwhitethresholdimage },
  { "magickwriteimage", fg_magickwriteimage },
  { "magickwriteimagefile", fg_magickwriteimagefile },
  { "magickwriteimages", fg_magickwriteimages },
  { "magickwriteimagesfile", fg_magickwriteimagesfile },
  { "pixelgetalpha", fg_pixelgetalpha },
  { "pixelgetalphaquantum", fg_pixelgetalphaquantum },
  { "pixelgetblack", fg_pixelgetblack },
  { "pixelgetblackquantum", fg_pixelgetblackquantum },
  { "pixelgetblue", fg_pixelgetblue },
  { "pixelgetbluequantum", fg_pixelgetbluequantum },
  { "pixelgetcolorasstring", fg_pixelgetcolorasstring },
  { "pixelgetcolorcount", fg_pixelgetcolorcount },
  { "pixelgetcyan", fg_pixelgetcyan },
  { "pixelgetcyanquantum", fg_pixelgetcyanquantum },
  { "pixelgetexception", fg_pixelgetexception },
  { "pixelgetexceptionstring", fg_pixelgetexceptionstring },
  { "pixelgetexceptiontype", fg_pixelgetexceptiontype },
  { "pixelgetgreen", fg_pixelgetgreen },
  { "pixelgetgreenquantum", fg_pixelgetgreenquantum },
  { "pixelgetindex", fg_pixelgetindex },
  { "pixelgetmagenta", fg_pixelgetmagenta },
  { "pixelgetmagentaquantum", fg_pixelgetmagentaquantum },
  { "pixelgetopacity", fg_pixelgetopacity },
  { "pixelgetopacityquantum", fg_pixelgetopacityquantum },
  { "pixelgetquantumcolor", fg_pixelgetquantumcolor },
  { "pixelgetred", fg_pixelgetred },
  { "pixelgetredquantum", fg_pixelgetredquantum },
  { "pixelgetyellow", fg_pixelgetyellow },
  { "pixelgetyellowquantum", fg_pixelgetyellowquantum },
  { "pixelsetalpha", fg_pixelsetalpha },
  { "pixelsetalphaquantum", fg_pixelsetalphaquantum },
  { "pixelsetblack", fg_pixelsetblack },
  { "pixelsetblackquantum", fg_pixelsetblackquantum },
  { "pixelsetblue", fg_pixelsetblue },
  { "pixelsetbluequantum", fg_pixelsetbluequantum },
  { "pixelsetcolor", fg_pixelsetcolor },
  { "pixelsetcolorcount", fg_pixelsetcolorcount },
  { "pixelsetcyan", fg_pixelsetcyan },
  { "pixelsetcyanquantum", fg_pixelsetcyanquantum },
  { "pixelsetgreen", fg_pixelsetgreen },
  { "pixelsetgreenquantum", fg_pixelsetgreenquantum },
  { "pixelsetindex", fg_pixelsetindex },
  { "pixelsetmagenta", fg_pixelsetmagenta },
  { "pixelsetmagentaquantum", fg_pixelsetmagentaquantum },
  { "pixelsetopacity", fg_pixelsetopacity },
  { "pixelsetopacityquantum", fg_pixelsetopacityquantum },
  { "pixelsetquantumcolor", fg_pixelsetquantumcolor },
  { "pixelsetred", fg_pixelsetred },
  { "pixelsetredquantum", fg_pixelsetredquantum },
  { "pixelsetyellow", fg_pixelsetyellow },
  { "pixelsetyellowquantum", fg_pixelsetyellowquantum },
  { "pixelgetiteratorexception", fg_pixelgetiteratorexception },
  { "pixelgetiteratorexceptionstring", fg_pixelgetiteratorexceptionstring },
  { "pixelgetiteratorexceptiontype", fg_pixelgetiteratorexceptiontype },
  { "pixelgetnextiteratorrow", fg_pixelgetnextiteratorrow },
  { "pixelresetiterator", fg_pixelresetiterator },
  { "pixelsetiteratorrow", fg_pixelsetiteratorrow },
  { "pixelsynciterator", fg_pixelsynciterator },
  { "mail", fg_mail },
  { "ezmlm_hash", fg_ezmlm_hash },
  { "mailparse_msg_create", fg_mailparse_msg_create },
  { "mailparse_msg_free", fg_mailparse_msg_free },
  { "mailparse_msg_parse_file", fg_mailparse_msg_parse_file },
  { "mailparse_msg_parse", fg_mailparse_msg_parse },
  { "mailparse_msg_extract_part_file", fg_mailparse_msg_extract_part_file },
  { "mailparse_msg_extract_whole_part_file", fg_mailparse_msg_extract_whole_part_file },
  { "mailparse_msg_extract_part", fg_mailparse_msg_extract_part },
  { "mailparse_msg_get_part_data", fg_mailparse_msg_get_part_data },
  { "mailparse_msg_get_part", fg_mailparse_msg_get_part },
  { "mailparse_msg_get_structure", fg_mailparse_msg_get_structure },
  { "mailparse_rfc822_parse_addresses", fg_mailparse_rfc822_parse_addresses },
  { "mailparse_stream_encode", fg_mailparse_stream_encode },
  { "mailparse_uudecode_all", fg_mailparse_uudecode_all },
  { "mailparse_determine_best_xfer_encoding", fg_mailparse_determine_best_xfer_encoding },
  { "pi", fg_pi },
  { "min", fg_min },
  { "max", fg_max },
  { "abs", fg_abs },
  { "is_finite", fg_is_finite },
  { "is_infinite", fg_is_infinite },
  { "is_nan", fg_is_nan },
  { "ceil", fg_ceil },
  { "floor", fg_floor },
  { "round", fg_round },
  { "deg2rad", fg_deg2rad },
  { "rad2deg", fg_rad2deg },
  { "decbin", fg_decbin },
  { "dechex", fg_dechex },
  { "decoct", fg_decoct },
  { "bindec", fg_bindec },
  { "hexdec", fg_hexdec },
  { "octdec", fg_octdec },
  { "base_convert", fg_base_convert },
  { "pow", fg_pow },
  { "exp", fg_exp },
  { "expm1", fg_expm1 },
  { "log10", fg_log10 },
  { "log1p", fg_log1p },
  { "log", fg_log },
  { "cos", fg_cos },
  { "cosh", fg_cosh },
  { "sin", fg_sin },
  { "sinh", fg_sinh },
  { "tan", fg_tan },
  { "tanh", fg_tanh },
  { "acos", fg_acos },
  { "acosh", fg_acosh },
  { "asin", fg_asin },
  { "asinh", fg_asinh },
  { "atan", fg_atan },
  { "atanh", fg_atanh },
  { "atan2", fg_atan2 },
  { "hypot", fg_hypot },
  { "fmod", fg_fmod },
  { "sqrt", fg_sqrt },
  { "getrandmax", fg_getrandmax },
  { "srand", fg_srand },
  { "rand", fg_rand },
  { "mt_getrandmax", fg_mt_getrandmax },
  { "mt_srand", fg_mt_srand },
  { "mt_rand", fg_mt_rand },
  { "lcg_value", fg_lcg_value },
  { "mb_list_encodings", fg_mb_list_encodings },
  { "mb_list_encodings_alias_names", fg_mb_list_encodings_alias_names },
  { "mb_list_mime_names", fg_mb_list_mime_names },
  { "mb_check_encoding", fg_mb_check_encoding },
  { "mb_convert_case", fg_mb_convert_case },
  { "mb_convert_encoding", fg_mb_convert_encoding },
  { "mb_convert_kana", fg_mb_convert_kana },
  { "mb_convert_variables", fg_mb_convert_variables },
  { "mb_decode_mimeheader", fg_mb_decode_mimeheader },
  { "mb_decode_numericentity", fg_mb_decode_numericentity },
  { "mb_detect_encoding", fg_mb_detect_encoding },
  { "mb_detect_order", fg_mb_detect_order },
  { "mb_encode_mimeheader", fg_mb_encode_mimeheader },
  { "mb_encode_numericentity", fg_mb_encode_numericentity },
  { "mb_ereg_match", fg_mb_ereg_match },
  { "mb_ereg_replace", fg_mb_ereg_replace },
  { "mb_ereg_search_getpos", fg_mb_ereg_search_getpos },
  { "mb_ereg_search_getregs", fg_mb_ereg_search_getregs },
  { "mb_ereg_search_init", fg_mb_ereg_search_init },
  { "mb_ereg_search_pos", fg_mb_ereg_search_pos },
  { "mb_ereg_search_regs", fg_mb_ereg_search_regs },
  { "mb_ereg_search_setpos", fg_mb_ereg_search_setpos },
  { "mb_ereg_search", fg_mb_ereg_search },
  { "mb_ereg", fg_mb_ereg },
  { "mb_eregi_replace", fg_mb_eregi_replace },
  { "mb_eregi", fg_mb_eregi },
  { "mb_get_info", fg_mb_get_info },
  { "mb_http_input", fg_mb_http_input },
  { "mb_http_output", fg_mb_http_output },
  { "mb_internal_encoding", fg_mb_internal_encoding },
  { "mb_language", fg_mb_language },
  { "mb_output_handler", fg_mb_output_handler },
  { "mb_parse_str", fg_mb_parse_str },
  { "mb_preferred_mime_name", fg_mb_preferred_mime_name },
  { "mb_regex_encoding", fg_mb_regex_encoding },
  { "mb_regex_set_options", fg_mb_regex_set_options },
  { "mb_send_mail", fg_mb_send_mail },
  { "mb_split", fg_mb_split },
  { "mb_strcut", fg_mb_strcut },
  { "mb_strimwidth", fg_mb_strimwidth },
  { "mb_stripos", fg_mb_stripos },
  { "mb_stristr", fg_mb_stristr },
  { "mb_strlen", fg_mb_strlen },
  { "mb_strpos", fg_mb_strpos },
  { "mb_strrchr", fg_mb_strrchr },
  { "mb_strrichr", fg_mb_strrichr },
  { "mb_strripos", fg_mb_strripos },
  { "mb_strrpos", fg_mb_strrpos },
  { "mb_strstr", fg_mb_strstr },
  { "mb_strtolower", fg_mb_strtolower },
  { "mb_strtoupper", fg_mb_strtoupper },
  { "mb_strwidth", fg_mb_strwidth },
  { "mb_substitute_character", fg_mb_substitute_character },
  { "mb_substr_count", fg_mb_substr_count },
  { "mb_substr", fg_mb_substr },
  { "mcrypt_module_open", fg_mcrypt_module_open },
  { "mcrypt_module_close", fg_mcrypt_module_close },
  { "mcrypt_list_algorithms", fg_mcrypt_list_algorithms },
  { "mcrypt_list_modes", fg_mcrypt_list_modes },
  { "mcrypt_module_get_algo_block_size", fg_mcrypt_module_get_algo_block_size },
  { "mcrypt_module_get_algo_key_size", fg_mcrypt_module_get_algo_key_size },
  { "mcrypt_module_get_supported_key_sizes", fg_mcrypt_module_get_supported_key_sizes },
  { "mcrypt_module_is_block_algorithm_mode", fg_mcrypt_module_is_block_algorithm_mode },
  { "mcrypt_module_is_block_algorithm", fg_mcrypt_module_is_block_algorithm },
  { "mcrypt_module_is_block_mode", fg_mcrypt_module_is_block_mode },
  { "mcrypt_module_self_test", fg_mcrypt_module_self_test },
  { "mcrypt_create_iv", fg_mcrypt_create_iv },
  { "mcrypt_encrypt", fg_mcrypt_encrypt },
  { "mcrypt_decrypt", fg_mcrypt_decrypt },
  { "mcrypt_cbc", fg_mcrypt_cbc },
  { "mcrypt_cfb", fg_mcrypt_cfb },
  { "mcrypt_ecb", fg_mcrypt_ecb },
  { "mcrypt_ofb", fg_mcrypt_ofb },
  { "mcrypt_get_block_size", fg_mcrypt_get_block_size },
  { "mcrypt_get_cipher_name", fg_mcrypt_get_cipher_name },
  { "mcrypt_get_iv_size", fg_mcrypt_get_iv_size },
  { "mcrypt_get_key_size", fg_mcrypt_get_key_size },
  { "mcrypt_enc_get_algorithms_name", fg_mcrypt_enc_get_algorithms_name },
  { "mcrypt_enc_get_block_size", fg_mcrypt_enc_get_block_size },
  { "mcrypt_enc_get_iv_size", fg_mcrypt_enc_get_iv_size },
  { "mcrypt_enc_get_key_size", fg_mcrypt_enc_get_key_size },
  { "mcrypt_enc_get_modes_name", fg_mcrypt_enc_get_modes_name },
  { "mcrypt_enc_get_supported_key_sizes", fg_mcrypt_enc_get_supported_key_sizes },
  { "mcrypt_enc_is_block_algorithm_mode", fg_mcrypt_enc_is_block_algorithm_mode },
  { "mcrypt_enc_is_block_algorithm", fg_mcrypt_enc_is_block_algorithm },
  { "mcrypt_enc_is_block_mode", fg_mcrypt_enc_is_block_mode },
  { "mcrypt_enc_self_test", fg_mcrypt_enc_self_test },
  { "mcrypt_generic", fg_mcrypt_generic },
  { "mcrypt_generic_init", fg_mcrypt_generic_init },
  { "mdecrypt_generic", fg_mdecrypt_generic },
  { "mcrypt_generic_deinit", fg_mcrypt_generic_deinit },
  { "mcrypt_generic_end", fg_mcrypt_generic_end },
  { "memcache_connect", fg_memcache_connect },
  { "memcache_pconnect", fg_memcache_pconnect },
  { "memcache_add", fg_memcache_add },
  { "memcache_set", fg_memcache_set },
  { "memcache_replace", fg_memcache_replace },
  { "memcache_get", fg_memcache_get },
  { "memcache_delete", fg_memcache_delete },
  { "memcache_increment", fg_memcache_increment },
  { "memcache_decrement", fg_memcache_decrement },
  { "memcache_close", fg_memcache_close },
  { "memcache_debug", fg_memcache_debug },
  { "memcache_get_version", fg_memcache_get_version },
  { "memcache_flush", fg_memcache_flush },
  { "memcache_setoptimeout", fg_memcache_setoptimeout },
  { "memcache_get_server_status", fg_memcache_get_server_status },
  { "memcache_set_compress_threshold", fg_memcache_set_compress_threshold },
  { "memcache_get_stats", fg_memcache_get_stats },
  { "memcache_get_extended_stats", fg_memcache_get_extended_stats },
  { "memcache_set_server_params", fg_memcache_set_server_params },
  { "memcache_add_server", fg_memcache_add_server },
  { "connection_aborted", fg_connection_aborted },
  { "connection_status", fg_connection_status },
  { "connection_timeout", fg_connection_timeout },
  { "constant", fg_constant },
  { "define", fg_define },
  { "defined", fg_defined },
  { "die", fg_die },
  { "exit", fg_exit },
  { "eval", fg_eval },
  { "get_browser", fg_get_browser },
  { "__halt_compiler", fg___halt_compiler },
  { "highlight_file", fg_highlight_file },
  { "show_source", fg_show_source },
  { "highlight_string", fg_highlight_string },
  { "ignore_user_abort", fg_ignore_user_abort },
  { "pack", fg_pack },
  { "php_check_syntax", fg_php_check_syntax },
  { "php_strip_whitespace", fg_php_strip_whitespace },
  { "sleep", fg_sleep },
  { "usleep", fg_usleep },
  { "time_nanosleep", fg_time_nanosleep },
  { "time_sleep_until", fg_time_sleep_until },
  { "uniqid", fg_uniqid },
  { "unpack", fg_unpack },
  { "sys_getloadavg", fg_sys_getloadavg },
  { "token_get_all", fg_token_get_all },
  { "token_name", fg_token_name },
  { "hphp_process_abort", fg_hphp_process_abort },
  { "hphp_to_string", fg_hphp_to_string },
  { "mysql_connect", fg_mysql_connect },
  { "mysql_pconnect", fg_mysql_pconnect },
  { "mysql_connect_with_db", fg_mysql_connect_with_db },
  { "mysql_pconnect_with_db", fg_mysql_pconnect_with_db },
  { "mysql_set_charset", fg_mysql_set_charset },
  { "mysql_ping", fg_mysql_ping },
  { "mysql_escape_string", fg_mysql_escape_string },
  { "mysql_real_escape_string", fg_mysql_real_escape_string },
  { "mysql_client_encoding", fg_mysql_client_encoding },
  { "mysql_close", fg_mysql_close },
  { "mysql_errno", fg_mysql_errno },
  { "mysql_error", fg_mysql_error },
  { "mysql_warning_count", fg_mysql_warning_count },
  { "mysql_get_client_info", fg_mysql_get_client_info },
  { "mysql_get_host_info", fg_mysql_get_host_info },
  { "mysql_get_proto_info", fg_mysql_get_proto_info },
  { "mysql_get_server_info", fg_mysql_get_server_info },
  { "mysql_info", fg_mysql_info },
  { "mysql_insert_id", fg_mysql_insert_id },
  { "mysql_stat", fg_mysql_stat },
  { "mysql_thread_id", fg_mysql_thread_id },
  { "mysql_create_db", fg_mysql_create_db },
  { "mysql_select_db", fg_mysql_select_db },
  { "mysql_drop_db", fg_mysql_drop_db },
  { "mysql_affected_rows", fg_mysql_affected_rows },
  { "mysql_set_timeout", fg_mysql_set_timeout },
  { "mysql_query", fg_mysql_query },
  { "mysql_multi_query", fg_mysql_multi_query },
  { "mysql_next_result", fg_mysql_next_result },
  { "mysql_more_results", fg_mysql_more_results },
  { "mysql_fetch_result", fg_mysql_fetch_result },
  { "mysql_unbuffered_query", fg_mysql_unbuffered_query },
  { "mysql_db_query", fg_mysql_db_query },
  { "mysql_list_dbs", fg_mysql_list_dbs },
  { "mysql_list_tables", fg_mysql_list_tables },
  { "mysql_list_fields", fg_mysql_list_fields },
  { "mysql_list_processes", fg_mysql_list_processes },
  { "mysql_db_name", fg_mysql_db_name },
  { "mysql_tablename", fg_mysql_tablename },
  { "mysql_num_fields", fg_mysql_num_fields },
  { "mysql_num_rows", fg_mysql_num_rows },
  { "mysql_free_result", fg_mysql_free_result },
  { "mysql_data_seek", fg_mysql_data_seek },
  { "mysql_fetch_row", fg_mysql_fetch_row },
  { "mysql_fetch_assoc", fg_mysql_fetch_assoc },
  { "mysql_fetch_array", fg_mysql_fetch_array },
  { "mysql_fetch_lengths", fg_mysql_fetch_lengths },
  { "mysql_fetch_object", fg_mysql_fetch_object },
  { "mysql_result", fg_mysql_result },
  { "mysql_fetch_field", fg_mysql_fetch_field },
  { "mysql_field_seek", fg_mysql_field_seek },
  { "mysql_field_name", fg_mysql_field_name },
  { "mysql_field_table", fg_mysql_field_table },
  { "mysql_field_len", fg_mysql_field_len },
  { "mysql_field_type", fg_mysql_field_type },
  { "mysql_field_flags", fg_mysql_field_flags },
  { "gethostname", fg_gethostname },
  { "gethostbyaddr", fg_gethostbyaddr },
  { "gethostbyname", fg_gethostbyname },
  { "gethostbynamel", fg_gethostbynamel },
  { "getprotobyname", fg_getprotobyname },
  { "getprotobynumber", fg_getprotobynumber },
  { "getservbyname", fg_getservbyname },
  { "getservbyport", fg_getservbyport },
  { "inet_ntop", fg_inet_ntop },
  { "inet_pton", fg_inet_pton },
  { "ip2long", fg_ip2long },
  { "long2ip", fg_long2ip },
  { "dns_check_record", fg_dns_check_record },
  { "checkdnsrr", fg_checkdnsrr },
  { "dns_get_record", fg_dns_get_record },
  { "dns_get_mx", fg_dns_get_mx },
  { "getmxrr", fg_getmxrr },
  { "fsockopen", fg_fsockopen },
  { "pfsockopen", fg_pfsockopen },
  { "socket_get_status", fg_socket_get_status },
  { "socket_set_blocking", fg_socket_set_blocking },
  { "socket_set_timeout", fg_socket_set_timeout },
  { "header", fg_header },
  { "headers_list", fg_headers_list },
  { "headers_sent", fg_headers_sent },
  { "header_register_callback", fg_header_register_callback },
  { "header_remove", fg_header_remove },
  { "setcookie", fg_setcookie },
  { "setrawcookie", fg_setrawcookie },
  { "define_syslog_variables", fg_define_syslog_variables },
  { "openlog", fg_openlog },
  { "closelog", fg_closelog },
  { "syslog", fg_syslog },
  { "openssl_csr_export_to_file", fg_openssl_csr_export_to_file },
  { "openssl_csr_export", fg_openssl_csr_export },
  { "openssl_csr_get_public_key", fg_openssl_csr_get_public_key },
  { "openssl_csr_get_subject", fg_openssl_csr_get_subject },
  { "openssl_csr_new", fg_openssl_csr_new },
  { "openssl_csr_sign", fg_openssl_csr_sign },
  { "openssl_error_string", fg_openssl_error_string },
  { "openssl_open", fg_openssl_open },
  { "openssl_pkcs12_export_to_file", fg_openssl_pkcs12_export_to_file },
  { "openssl_pkcs12_export", fg_openssl_pkcs12_export },
  { "openssl_pkcs12_read", fg_openssl_pkcs12_read },
  { "openssl_pkcs7_decrypt", fg_openssl_pkcs7_decrypt },
  { "openssl_pkcs7_encrypt", fg_openssl_pkcs7_encrypt },
  { "openssl_pkcs7_sign", fg_openssl_pkcs7_sign },
  { "openssl_pkcs7_verify", fg_openssl_pkcs7_verify },
  { "openssl_pkey_export_to_file", fg_openssl_pkey_export_to_file },
  { "openssl_pkey_export", fg_openssl_pkey_export },
  { "openssl_pkey_free", fg_openssl_pkey_free },
  { "openssl_free_key", fg_openssl_free_key },
  { "openssl_pkey_get_details", fg_openssl_pkey_get_details },
  { "openssl_pkey_get_private", fg_openssl_pkey_get_private },
  { "openssl_get_privatekey", fg_openssl_get_privatekey },
  { "openssl_pkey_get_public", fg_openssl_pkey_get_public },
  { "openssl_get_publickey", fg_openssl_get_publickey },
  { "openssl_pkey_new", fg_openssl_pkey_new },
  { "openssl_private_decrypt", fg_openssl_private_decrypt },
  { "openssl_private_encrypt", fg_openssl_private_encrypt },
  { "openssl_public_decrypt", fg_openssl_public_decrypt },
  { "openssl_public_encrypt", fg_openssl_public_encrypt },
  { "openssl_seal", fg_openssl_seal },
  { "openssl_sign", fg_openssl_sign },
  { "openssl_verify", fg_openssl_verify },
  { "openssl_x509_check_private_key", fg_openssl_x509_check_private_key },
  { "openssl_x509_checkpurpose", fg_openssl_x509_checkpurpose },
  { "openssl_x509_export_to_file", fg_openssl_x509_export_to_file },
  { "openssl_x509_export", fg_openssl_x509_export },
  { "openssl_x509_free", fg_openssl_x509_free },
  { "openssl_x509_parse", fg_openssl_x509_parse },
  { "openssl_x509_read", fg_openssl_x509_read },
  { "openssl_random_pseudo_bytes", fg_openssl_random_pseudo_bytes },
  { "openssl_cipher_iv_length", fg_openssl_cipher_iv_length },
  { "openssl_encrypt", fg_openssl_encrypt },
  { "openssl_decrypt", fg_openssl_decrypt },
  { "openssl_digest", fg_openssl_digest },
  { "openssl_get_cipher_methods", fg_openssl_get_cipher_methods },
  { "openssl_get_md_methods", fg_openssl_get_md_methods },
  { "assert_options", fg_assert_options },
  { "assert", fg_assert },
  { "dl", fg_dl },
  { "extension_loaded", fg_extension_loaded },
  { "get_loaded_extensions", fg_get_loaded_extensions },
  { "get_extension_funcs", fg_get_extension_funcs },
  { "get_cfg_var", fg_get_cfg_var },
  { "get_current_user", fg_get_current_user },
  { "get_defined_constants", fg_get_defined_constants },
  { "get_include_path", fg_get_include_path },
  { "restore_include_path", fg_restore_include_path },
  { "set_include_path", fg_set_include_path },
  { "get_included_files", fg_get_included_files },
  { "inclued_get_data", fg_inclued_get_data },
  { "get_magic_quotes_gpc", fg_get_magic_quotes_gpc },
  { "get_magic_quotes_runtime", fg_get_magic_quotes_runtime },
  { "get_required_files", fg_get_required_files },
  { "getenv", fg_getenv },
  { "getlastmod", fg_getlastmod },
  { "getmygid", fg_getmygid },
  { "getmyinode", fg_getmyinode },
  { "getmypid", fg_getmypid },
  { "getmyuid", fg_getmyuid },
  { "getopt", fg_getopt },
  { "getrusage", fg_getrusage },
  { "clock_getres", fg_clock_getres },
  { "clock_gettime", fg_clock_gettime },
  { "clock_settime", fg_clock_settime },
  { "cpu_get_count", fg_cpu_get_count },
  { "cpu_get_model", fg_cpu_get_model },
  { "ini_alter", fg_ini_alter },
  { "ini_get_all", fg_ini_get_all },
  { "ini_get", fg_ini_get },
  { "ini_restore", fg_ini_restore },
  { "ini_set", fg_ini_set },
  { "memory_get_allocation", fg_memory_get_allocation },
  { "memory_get_peak_usage", fg_memory_get_peak_usage },
  { "memory_get_usage", fg_memory_get_usage },
  { "php_ini_scanned_files", fg_php_ini_scanned_files },
  { "php_logo_guid", fg_php_logo_guid },
  { "php_sapi_name", fg_php_sapi_name },
  { "php_uname", fg_php_uname },
  { "phpcredits", fg_phpcredits },
  { "phpinfo", fg_phpinfo },
  { "phpversion", fg_phpversion },
  { "putenv", fg_putenv },
  { "set_magic_quotes_runtime", fg_set_magic_quotes_runtime },
  { "set_time_limit", fg_set_time_limit },
  { "sys_get_temp_dir", fg_sys_get_temp_dir },
  { "version_compare", fg_version_compare },
  { "gc_enabled", fg_gc_enabled },
  { "gc_enable", fg_gc_enable },
  { "gc_disable", fg_gc_disable },
  { "gc_collect_cycles", fg_gc_collect_cycles },
  { "zend_logo_guid", fg_zend_logo_guid },
  { "zend_thread_id", fg_zend_thread_id },
  { "zend_version", fg_zend_version },
  { "ob_start", fg_ob_start },
  { "ob_clean", fg_ob_clean },
  { "ob_flush", fg_ob_flush },
  { "ob_end_clean", fg_ob_end_clean },
  { "ob_end_flush", fg_ob_end_flush },
  { "flush", fg_flush },
  { "ob_get_clean", fg_ob_get_clean },
  { "ob_get_contents", fg_ob_get_contents },
  { "ob_get_flush", fg_ob_get_flush },
  { "ob_get_length", fg_ob_get_length },
  { "ob_get_level", fg_ob_get_level },
  { "ob_get_status", fg_ob_get_status },
  { "ob_gzhandler", fg_ob_gzhandler },
  { "ob_implicit_flush", fg_ob_implicit_flush },
  { "ob_list_handlers", fg_ob_list_handlers },
  { "output_add_rewrite_var", fg_output_add_rewrite_var },
  { "output_reset_rewrite_vars", fg_output_reset_rewrite_vars },
  { "hphp_crash_log", fg_hphp_crash_log },
  { "hphp_stats", fg_hphp_stats },
  { "hphp_get_stats", fg_hphp_get_stats },
  { "hphp_get_status", fg_hphp_get_status },
  { "hphp_get_iostatus", fg_hphp_get_iostatus },
  { "hphp_set_iostatus_address", fg_hphp_set_iostatus_address },
  { "hphp_get_timers", fg_hphp_get_timers },
  { "hphp_output_global_state", fg_hphp_output_global_state },
  { "hphp_instruction_counter", fg_hphp_instruction_counter },
  { "hphp_get_hardware_counters", fg_hphp_get_hardware_counters },
  { "hphp_set_hardware_events", fg_hphp_set_hardware_events },
  { "hphp_clear_hardware_events", fg_hphp_clear_hardware_events },
  { "pdo_drivers", fg_pdo_drivers },
  { "posix_access", fg_posix_access },
  { "posix_ctermid", fg_posix_ctermid },
  { "posix_get_last_error", fg_posix_get_last_error },
  { "posix_getcwd", fg_posix_getcwd },
  { "posix_getegid", fg_posix_getegid },
  { "posix_geteuid", fg_posix_geteuid },
  { "posix_getgid", fg_posix_getgid },
  { "posix_getgrgid", fg_posix_getgrgid },
  { "posix_getgrnam", fg_posix_getgrnam },
  { "posix_getgroups", fg_posix_getgroups },
  { "posix_getlogin", fg_posix_getlogin },
  { "posix_getpgid", fg_posix_getpgid },
  { "posix_getpgrp", fg_posix_getpgrp },
  { "posix_getpid", fg_posix_getpid },
  { "posix_getppid", fg_posix_getppid },
  { "posix_getpwnam", fg_posix_getpwnam },
  { "posix_getpwuid", fg_posix_getpwuid },
  { "posix_getrlimit", fg_posix_getrlimit },
  { "posix_getsid", fg_posix_getsid },
  { "posix_getuid", fg_posix_getuid },
  { "posix_initgroups", fg_posix_initgroups },
  { "posix_isatty", fg_posix_isatty },
  { "posix_kill", fg_posix_kill },
  { "posix_mkfifo", fg_posix_mkfifo },
  { "posix_mknod", fg_posix_mknod },
  { "posix_setegid", fg_posix_setegid },
  { "posix_seteuid", fg_posix_seteuid },
  { "posix_setgid", fg_posix_setgid },
  { "posix_setpgid", fg_posix_setpgid },
  { "posix_setsid", fg_posix_setsid },
  { "posix_setuid", fg_posix_setuid },
  { "posix_strerror", fg_posix_strerror },
  { "posix_times", fg_posix_times },
  { "posix_ttyname", fg_posix_ttyname },
  { "posix_uname", fg_posix_uname },
  { "preg_grep", fg_preg_grep },
  { "preg_match", fg_preg_match },
  { "preg_match_all", fg_preg_match_all },
  { "preg_replace", fg_preg_replace },
  { "preg_replace_callback", fg_preg_replace_callback },
  { "preg_split", fg_preg_split },
  { "preg_quote", fg_preg_quote },
  { "preg_last_error", fg_preg_last_error },
  { "ereg_replace", fg_ereg_replace },
  { "eregi_replace", fg_eregi_replace },
  { "ereg", fg_ereg },
  { "eregi", fg_eregi },
  { "split", fg_split },
  { "spliti", fg_spliti },
  { "sql_regcase", fg_sql_regcase },
  { "pcntl_alarm", fg_pcntl_alarm },
  { "pcntl_exec", fg_pcntl_exec },
  { "pcntl_fork", fg_pcntl_fork },
  { "pcntl_getpriority", fg_pcntl_getpriority },
  { "pcntl_setpriority", fg_pcntl_setpriority },
  { "pcntl_signal", fg_pcntl_signal },
  { "pcntl_wait", fg_pcntl_wait },
  { "pcntl_waitpid", fg_pcntl_waitpid },
  { "pcntl_wexitstatus", fg_pcntl_wexitstatus },
  { "pcntl_wifexited", fg_pcntl_wifexited },
  { "pcntl_wifsignaled", fg_pcntl_wifsignaled },
  { "pcntl_wifstopped", fg_pcntl_wifstopped },
  { "pcntl_wstopsig", fg_pcntl_wstopsig },
  { "pcntl_wtermsig", fg_pcntl_wtermsig },
  { "pcntl_signal_dispatch", fg_pcntl_signal_dispatch },
  { "shell_exec", fg_shell_exec },
  { "exec", fg_exec },
  { "passthru", fg_passthru },
  { "system", fg_system },
  { "proc_open", fg_proc_open },
  { "proc_terminate", fg_proc_terminate },
  { "proc_close", fg_proc_close },
  { "proc_get_status", fg_proc_get_status },
  { "proc_nice", fg_proc_nice },
  { "escapeshellarg", fg_escapeshellarg },
  { "escapeshellcmd", fg_escapeshellcmd },
  { "hphp_get_extension_info", fg_hphp_get_extension_info },
  { "hphp_get_method_info", fg_hphp_get_method_info },
  { "hphp_get_closure_info", fg_hphp_get_closure_info },
  { "hphp_get_class_constant", fg_hphp_get_class_constant },
  { "hphp_get_class_info", fg_hphp_get_class_info },
  { "hphp_get_function_info", fg_hphp_get_function_info },
  { "hphp_invoke", fg_hphp_invoke },
  { "hphp_invoke_method", fg_hphp_invoke_method },
  { "hphp_instanceof", fg_hphp_instanceof },
  { "hphp_create_object", fg_hphp_create_object },
  { "hphp_get_property", fg_hphp_get_property },
  { "hphp_set_property", fg_hphp_set_property },
  { "hphp_get_static_property", fg_hphp_get_static_property },
  { "hphp_set_static_property", fg_hphp_set_static_property },
  { "hphp_get_original_class_name", fg_hphp_get_original_class_name },
  { "hphp_scalar_typehints_enabled", fg_hphp_scalar_typehints_enabled },
  { "dangling_server_proxy_old_request", fg_dangling_server_proxy_old_request },
  { "dangling_server_proxy_new_request", fg_dangling_server_proxy_new_request },
  { "pagelet_server_is_enabled", fg_pagelet_server_is_enabled },
  { "pagelet_server_task_start", fg_pagelet_server_task_start },
  { "pagelet_server_task_status", fg_pagelet_server_task_status },
  { "pagelet_server_task_result", fg_pagelet_server_task_result },
  { "pagelet_server_flush", fg_pagelet_server_flush },
  { "xbox_send_message", fg_xbox_send_message },
  { "xbox_post_message", fg_xbox_post_message },
  { "xbox_task_start", fg_xbox_task_start },
  { "xbox_task_status", fg_xbox_task_status },
  { "xbox_task_result", fg_xbox_task_result },
  { "xbox_process_call_message", fg_xbox_process_call_message },
  { "xbox_get_thread_timeout", fg_xbox_get_thread_timeout },
  { "xbox_set_thread_timeout", fg_xbox_set_thread_timeout },
  { "xbox_schedule_thread_reset", fg_xbox_schedule_thread_reset },
  { "xbox_get_thread_time", fg_xbox_get_thread_time },
  { "session_set_cookie_params", fg_session_set_cookie_params },
  { "session_get_cookie_params", fg_session_get_cookie_params },
  { "session_name", fg_session_name },
  { "session_module_name", fg_session_module_name },
  { "session_set_save_handler", fg_session_set_save_handler },
  { "session_save_path", fg_session_save_path },
  { "session_id", fg_session_id },
  { "session_regenerate_id", fg_session_regenerate_id },
  { "session_cache_limiter", fg_session_cache_limiter },
  { "session_cache_expire", fg_session_cache_expire },
  { "session_encode", fg_session_encode },
  { "session_decode", fg_session_decode },
  { "session_start", fg_session_start },
  { "session_destroy", fg_session_destroy },
  { "session_unset", fg_session_unset },
  { "session_commit", fg_session_commit },
  { "session_write_close", fg_session_write_close },
  { "session_register", fg_session_register },
  { "session_unregister", fg_session_unregister },
  { "session_is_registered", fg_session_is_registered },
  { "simplexml_load_string", fg_simplexml_load_string },
  { "simplexml_load_file", fg_simplexml_load_file },
  { "libxml_get_errors", fg_libxml_get_errors },
  { "libxml_get_last_error", fg_libxml_get_last_error },
  { "libxml_clear_errors", fg_libxml_clear_errors },
  { "libxml_use_internal_errors", fg_libxml_use_internal_errors },
  { "libxml_set_streams_context", fg_libxml_set_streams_context },
  { "libxml_disable_entity_loader", fg_libxml_disable_entity_loader },
  { "use_soap_error_handler", fg_use_soap_error_handler },
  { "is_soap_fault", fg_is_soap_fault },
  { "_soap_active_version", fg__soap_active_version },
  { "socket_create", fg_socket_create },
  { "socket_create_listen", fg_socket_create_listen },
  { "socket_create_pair", fg_socket_create_pair },
  { "socket_get_option", fg_socket_get_option },
  { "socket_getpeername", fg_socket_getpeername },
  { "socket_getsockname", fg_socket_getsockname },
  { "socket_set_block", fg_socket_set_block },
  { "socket_set_nonblock", fg_socket_set_nonblock },
  { "socket_set_option", fg_socket_set_option },
  { "socket_connect", fg_socket_connect },
  { "socket_bind", fg_socket_bind },
  { "socket_listen", fg_socket_listen },
  { "socket_select", fg_socket_select },
  { "socket_server", fg_socket_server },
  { "socket_accept", fg_socket_accept },
  { "socket_read", fg_socket_read },
  { "socket_write", fg_socket_write },
  { "socket_send", fg_socket_send },
  { "socket_sendto", fg_socket_sendto },
  { "socket_recv", fg_socket_recv },
  { "socket_recvfrom", fg_socket_recvfrom },
  { "socket_shutdown", fg_socket_shutdown },
  { "socket_close", fg_socket_close },
  { "socket_strerror", fg_socket_strerror },
  { "socket_last_error", fg_socket_last_error },
  { "socket_clear_error", fg_socket_clear_error },
  { "getaddrinfo", fg_getaddrinfo },
  { "spl_classes", fg_spl_classes },
  { "spl_object_hash", fg_spl_object_hash },
  { "hphp_object_pointer", fg_hphp_object_pointer },
  { "hphp_get_this", fg_hphp_get_this },
  { "class_implements", fg_class_implements },
  { "class_parents", fg_class_parents },
  { "class_uses", fg_class_uses },
  { "iterator_apply", fg_iterator_apply },
  { "iterator_count", fg_iterator_count },
  { "iterator_to_array", fg_iterator_to_array },
  { "spl_autoload_call", fg_spl_autoload_call },
  { "spl_autoload_extensions", fg_spl_autoload_extensions },
  { "spl_autoload_functions", fg_spl_autoload_functions },
  { "spl_autoload_register", fg_spl_autoload_register },
  { "spl_autoload_unregister", fg_spl_autoload_unregister },
  { "spl_autoload", fg_spl_autoload },
  { "hphp_splfileinfo___construct", fg_hphp_splfileinfo___construct },
  { "hphp_splfileinfo_getatime", fg_hphp_splfileinfo_getatime },
  { "hphp_splfileinfo_getbasename", fg_hphp_splfileinfo_getbasename },
  { "hphp_splfileinfo_getctime", fg_hphp_splfileinfo_getctime },
  { "hphp_splfileinfo_getfileinfo", fg_hphp_splfileinfo_getfileinfo },
  { "hphp_splfileinfo_getfilename", fg_hphp_splfileinfo_getfilename },
  { "hphp_splfileinfo_getgroup", fg_hphp_splfileinfo_getgroup },
  { "hphp_splfileinfo_getinode", fg_hphp_splfileinfo_getinode },
  { "hphp_splfileinfo_getlinktarget", fg_hphp_splfileinfo_getlinktarget },
  { "hphp_splfileinfo_getmtime", fg_hphp_splfileinfo_getmtime },
  { "hphp_splfileinfo_getowner", fg_hphp_splfileinfo_getowner },
  { "hphp_splfileinfo_getpath", fg_hphp_splfileinfo_getpath },
  { "hphp_splfileinfo_getpathinfo", fg_hphp_splfileinfo_getpathinfo },
  { "hphp_splfileinfo_getpathname", fg_hphp_splfileinfo_getpathname },
  { "hphp_splfileinfo_getperms", fg_hphp_splfileinfo_getperms },
  { "hphp_splfileinfo_getrealpath", fg_hphp_splfileinfo_getrealpath },
  { "hphp_splfileinfo_getsize", fg_hphp_splfileinfo_getsize },
  { "hphp_splfileinfo_gettype", fg_hphp_splfileinfo_gettype },
  { "hphp_splfileinfo_isdir", fg_hphp_splfileinfo_isdir },
  { "hphp_splfileinfo_isexecutable", fg_hphp_splfileinfo_isexecutable },
  { "hphp_splfileinfo_isfile", fg_hphp_splfileinfo_isfile },
  { "hphp_splfileinfo_islink", fg_hphp_splfileinfo_islink },
  { "hphp_splfileinfo_isreadable", fg_hphp_splfileinfo_isreadable },
  { "hphp_splfileinfo_iswritable", fg_hphp_splfileinfo_iswritable },
  { "hphp_splfileinfo_openfile", fg_hphp_splfileinfo_openfile },
  { "hphp_splfileinfo_setfileclass", fg_hphp_splfileinfo_setfileclass },
  { "hphp_splfileinfo_setinfoclass", fg_hphp_splfileinfo_setinfoclass },
  { "hphp_splfileinfo___tostring", fg_hphp_splfileinfo___tostring },
  { "hphp_splfileobject___construct", fg_hphp_splfileobject___construct },
  { "hphp_splfileobject_current", fg_hphp_splfileobject_current },
  { "hphp_splfileobject_eof", fg_hphp_splfileobject_eof },
  { "hphp_splfileobject_fflush", fg_hphp_splfileobject_fflush },
  { "hphp_splfileobject_fgetc", fg_hphp_splfileobject_fgetc },
  { "hphp_splfileobject_fgetcsv", fg_hphp_splfileobject_fgetcsv },
  { "hphp_splfileobject_fgets", fg_hphp_splfileobject_fgets },
  { "hphp_splfileobject_fgetss", fg_hphp_splfileobject_fgetss },
  { "hphp_splfileobject_flock", fg_hphp_splfileobject_flock },
  { "hphp_splfileobject_fpassthru", fg_hphp_splfileobject_fpassthru },
  { "hphp_splfileobject_fscanf", fg_hphp_splfileobject_fscanf },
  { "hphp_splfileobject_fseek", fg_hphp_splfileobject_fseek },
  { "hphp_splfileobject_fstat", fg_hphp_splfileobject_fstat },
  { "hphp_splfileobject_ftell", fg_hphp_splfileobject_ftell },
  { "hphp_splfileobject_ftruncate", fg_hphp_splfileobject_ftruncate },
  { "hphp_splfileobject_fwrite", fg_hphp_splfileobject_fwrite },
  { "hphp_splfileobject_getcvscontrol", fg_hphp_splfileobject_getcvscontrol },
  { "hphp_splfileobject_getflags", fg_hphp_splfileobject_getflags },
  { "hphp_splfileobject_getmaxlinelen", fg_hphp_splfileobject_getmaxlinelen },
  { "hphp_splfileobject_key", fg_hphp_splfileobject_key },
  { "hphp_splfileobject_next", fg_hphp_splfileobject_next },
  { "hphp_splfileobject_rewind", fg_hphp_splfileobject_rewind },
  { "hphp_splfileobject_valid", fg_hphp_splfileobject_valid },
  { "hphp_splfileobject_seek", fg_hphp_splfileobject_seek },
  { "hphp_splfileobject_setcsvcontrol", fg_hphp_splfileobject_setcsvcontrol },
  { "hphp_splfileobject_setflags", fg_hphp_splfileobject_setflags },
  { "hphp_splfileobject_setmaxlinelen", fg_hphp_splfileobject_setmaxlinelen },
  { "stream_context_create", fg_stream_context_create },
  { "stream_context_get_default", fg_stream_context_get_default },
  { "stream_context_get_options", fg_stream_context_get_options },
  { "stream_context_set_option", fg_stream_context_set_option },
  { "stream_context_set_param", fg_stream_context_set_param },
  { "stream_copy_to_stream", fg_stream_copy_to_stream },
  { "stream_encoding", fg_stream_encoding },
  { "stream_bucket_append", fg_stream_bucket_append },
  { "stream_bucket_prepend", fg_stream_bucket_prepend },
  { "stream_bucket_make_writeable", fg_stream_bucket_make_writeable },
  { "stream_bucket_new", fg_stream_bucket_new },
  { "stream_filter_register", fg_stream_filter_register },
  { "stream_filter_remove", fg_stream_filter_remove },
  { "stream_filter_append", fg_stream_filter_append },
  { "stream_filter_prepend", fg_stream_filter_prepend },
  { "stream_get_contents", fg_stream_get_contents },
  { "stream_get_filters", fg_stream_get_filters },
  { "stream_get_line", fg_stream_get_line },
  { "stream_get_meta_data", fg_stream_get_meta_data },
  { "stream_get_transports", fg_stream_get_transports },
  { "stream_get_wrappers", fg_stream_get_wrappers },
  { "stream_register_wrapper", fg_stream_register_wrapper },
  { "stream_wrapper_register", fg_stream_wrapper_register },
  { "stream_wrapper_restore", fg_stream_wrapper_restore },
  { "stream_wrapper_unregister", fg_stream_wrapper_unregister },
  { "stream_resolve_include_path", fg_stream_resolve_include_path },
  { "stream_select", fg_stream_select },
  { "stream_set_blocking", fg_stream_set_blocking },
  { "stream_set_timeout", fg_stream_set_timeout },
  { "stream_set_write_buffer", fg_stream_set_write_buffer },
  { "set_file_buffer", fg_set_file_buffer },
  { "stream_socket_accept", fg_stream_socket_accept },
  { "stream_socket_server", fg_stream_socket_server },
  { "stream_socket_client", fg_stream_socket_client },
  { "stream_socket_enable_crypto", fg_stream_socket_enable_crypto },
  { "stream_socket_get_name", fg_stream_socket_get_name },
  { "stream_socket_pair", fg_stream_socket_pair },
  { "stream_socket_recvfrom", fg_stream_socket_recvfrom },
  { "stream_socket_sendto", fg_stream_socket_sendto },
  { "stream_socket_shutdown", fg_stream_socket_shutdown },
  { "addcslashes", fg_addcslashes },
  { "stripcslashes", fg_stripcslashes },
  { "addslashes", fg_addslashes },
  { "stripslashes", fg_stripslashes },
  { "bin2hex", fg_bin2hex },
  { "hex2bin", fg_hex2bin },
  { "nl2br", fg_nl2br },
  { "quotemeta", fg_quotemeta },
  { "str_shuffle", fg_str_shuffle },
  { "strrev", fg_strrev },
  { "strtolower", fg_strtolower },
  { "strtoupper", fg_strtoupper },
  { "ucfirst", fg_ucfirst },
  { "ucwords", fg_ucwords },
  { "strip_tags", fg_strip_tags },
  { "trim", fg_trim },
  { "ltrim", fg_ltrim },
  { "rtrim", fg_rtrim },
  { "chop", fg_chop },
  { "explode", fg_explode },
  { "implode", fg_implode },
  { "join", fg_join },
  { "str_split", fg_str_split },
  { "chunk_split", fg_chunk_split },
  { "strtok", fg_strtok },
  { "str_replace", fg_str_replace },
  { "str_ireplace", fg_str_ireplace },
  { "substr_replace", fg_substr_replace },
  { "substr", fg_substr },
  { "str_pad", fg_str_pad },
  { "str_repeat", fg_str_repeat },
  { "wordwrap", fg_wordwrap },
  { "html_entity_decode", fg_html_entity_decode },
  { "htmlentities", fg_htmlentities },
  { "htmlspecialchars_decode", fg_htmlspecialchars_decode },
  { "htmlspecialchars", fg_htmlspecialchars },
  { "fb_htmlspecialchars", fg_fb_htmlspecialchars },
  { "quoted_printable_encode", fg_quoted_printable_encode },
  { "quoted_printable_decode", fg_quoted_printable_decode },
  { "convert_uudecode", fg_convert_uudecode },
  { "convert_uuencode", fg_convert_uuencode },
  { "str_rot13", fg_str_rot13 },
  { "crc32", fg_crc32 },
  { "crypt", fg_crypt },
  { "md5", fg_md5 },
  { "sha1", fg_sha1 },
  { "strtr", fg_strtr },
  { "convert_cyr_string", fg_convert_cyr_string },
  { "get_html_translation_table", fg_get_html_translation_table },
  { "hebrev", fg_hebrev },
  { "hebrevc", fg_hebrevc },
  { "setlocale", fg_setlocale },
  { "localeconv", fg_localeconv },
  { "nl_langinfo", fg_nl_langinfo },
  { "printf", fg_printf },
  { "vprintf", fg_vprintf },
  { "sprintf", fg_sprintf },
  { "vsprintf", fg_vsprintf },
  { "sscanf", fg_sscanf },
  { "chr", fg_chr },
  { "ord", fg_ord },
  { "money_format", fg_money_format },
  { "number_format", fg_number_format },
  { "strcmp", fg_strcmp },
  { "strncmp", fg_strncmp },
  { "strnatcmp", fg_strnatcmp },
  { "strcasecmp", fg_strcasecmp },
  { "strncasecmp", fg_strncasecmp },
  { "strnatcasecmp", fg_strnatcasecmp },
  { "strcoll", fg_strcoll },
  { "substr_compare", fg_substr_compare },
  { "strchr", fg_strchr },
  { "strrchr", fg_strrchr },
  { "strstr", fg_strstr },
  { "stristr", fg_stristr },
  { "strpbrk", fg_strpbrk },
  { "strpos", fg_strpos },
  { "stripos", fg_stripos },
  { "strrpos", fg_strrpos },
  { "strripos", fg_strripos },
  { "substr_count", fg_substr_count },
  { "strspn", fg_strspn },
  { "strcspn", fg_strcspn },
  { "strlen", fg_strlen },
  { "count_chars", fg_count_chars },
  { "str_word_count", fg_str_word_count },
  { "levenshtein", fg_levenshtein },
  { "similar_text", fg_similar_text },
  { "soundex", fg_soundex },
  { "metaphone", fg_metaphone },
  { "parse_str", fg_parse_str },
  { "hphp_is_service_thread", fg_hphp_is_service_thread },
  { "hphp_service_thread_started", fg_hphp_service_thread_started },
  { "hphp_service_thread_stopped", fg_hphp_service_thread_stopped },
  { "hphp_thread_is_warmup_enabled", fg_hphp_thread_is_warmup_enabled },
  { "hphp_thread_set_warmup_enabled", fg_hphp_thread_set_warmup_enabled },
  { "hphp_get_thread_id", fg_hphp_get_thread_id },
  { "hphp_gettid", fg_hphp_gettid },
  { "thrift_protocol_write_binary", fg_thrift_protocol_write_binary },
  { "thrift_protocol_read_binary", fg_thrift_protocol_read_binary },
  { "thrift_protocol_set_compact_version", fg_thrift_protocol_set_compact_version },
  { "thrift_protocol_write_compact", fg_thrift_protocol_write_compact },
  { "thrift_protocol_read_compact", fg_thrift_protocol_read_compact },
  { "base64_decode", fg_base64_decode },
  { "base64_encode", fg_base64_encode },
  { "get_headers", fg_get_headers },
  { "get_meta_tags", fg_get_meta_tags },
  { "http_build_query", fg_http_build_query },
  { "parse_url", fg_parse_url },
  { "rawurldecode", fg_rawurldecode },
  { "rawurlencode", fg_rawurlencode },
  { "urldecode", fg_urldecode },
  { "urlencode", fg_urlencode },
  { "is_bool", fg_is_bool },
  { "is_int", fg_is_int },
  { "is_integer", fg_is_integer },
  { "is_long", fg_is_long },
  { "is_double", fg_is_double },
  { "is_float", fg_is_float },
  { "is_numeric", fg_is_numeric },
  { "is_real", fg_is_real },
  { "is_string", fg_is_string },
  { "is_scalar", fg_is_scalar },
  { "is_array", fg_is_array },
  { "is_object", fg_is_object },
  { "is_resource", fg_is_resource },
  { "is_null", fg_is_null },
  { "gettype", fg_gettype },
  { "get_resource_type", fg_get_resource_type },
  { "intval", fg_intval },
  { "doubleval", fg_doubleval },
  { "floatval", fg_floatval },
  { "strval", fg_strval },
  { "settype", fg_settype },
  { "print_r", fg_print_r },
  { "var_export", fg_var_export },
  { "var_dump", fg_var_dump },
  { "debug_zval_dump", fg_debug_zval_dump },
  { "serialize", fg_serialize },
  { "unserialize", fg_unserialize },
  { "get_defined_vars", fg_get_defined_vars },
  { "import_request_variables", fg_import_request_variables },
  { "extract", fg_extract },
  { "xml_parser_create", fg_xml_parser_create },
  { "xml_parser_free", fg_xml_parser_free },
  { "xml_parse", fg_xml_parse },
  { "xml_parse_into_struct", fg_xml_parse_into_struct },
  { "xml_parser_create_ns", fg_xml_parser_create_ns },
  { "xml_parser_get_option", fg_xml_parser_get_option },
  { "xml_parser_set_option", fg_xml_parser_set_option },
  { "xml_set_character_data_handler", fg_xml_set_character_data_handler },
  { "xml_set_default_handler", fg_xml_set_default_handler },
  { "xml_set_element_handler", fg_xml_set_element_handler },
  { "xml_set_processing_instruction_handler", fg_xml_set_processing_instruction_handler },
  { "xml_set_start_namespace_decl_handler", fg_xml_set_start_namespace_decl_handler },
  { "xml_set_end_namespace_decl_handler", fg_xml_set_end_namespace_decl_handler },
  { "xml_set_unparsed_entity_decl_handler", fg_xml_set_unparsed_entity_decl_handler },
  { "xml_set_external_entity_ref_handler", fg_xml_set_external_entity_ref_handler },
  { "xml_set_notation_decl_handler", fg_xml_set_notation_decl_handler },
  { "xml_set_object", fg_xml_set_object },
  { "xml_get_current_byte_index", fg_xml_get_current_byte_index },
  { "xml_get_current_column_number", fg_xml_get_current_column_number },
  { "xml_get_current_line_number", fg_xml_get_current_line_number },
  { "xml_get_error_code", fg_xml_get_error_code },
  { "xml_error_string", fg_xml_error_string },
  { "utf8_decode", fg_utf8_decode },
  { "utf8_encode", fg_utf8_encode },
  { "xmlwriter_open_memory", fg_xmlwriter_open_memory },
  { "xmlwriter_open_uri", fg_xmlwriter_open_uri },
  { "xmlwriter_set_indent_string", fg_xmlwriter_set_indent_string },
  { "xmlwriter_set_indent", fg_xmlwriter_set_indent },
  { "xmlwriter_start_document", fg_xmlwriter_start_document },
  { "xmlwriter_start_element", fg_xmlwriter_start_element },
  { "xmlwriter_start_element_ns", fg_xmlwriter_start_element_ns },
  { "xmlwriter_write_element_ns", fg_xmlwriter_write_element_ns },
  { "xmlwriter_write_element", fg_xmlwriter_write_element },
  { "xmlwriter_end_element", fg_xmlwriter_end_element },
  { "xmlwriter_full_end_element", fg_xmlwriter_full_end_element },
  { "xmlwriter_start_attribute_ns", fg_xmlwriter_start_attribute_ns },
  { "xmlwriter_start_attribute", fg_xmlwriter_start_attribute },
  { "xmlwriter_write_attribute_ns", fg_xmlwriter_write_attribute_ns },
  { "xmlwriter_write_attribute", fg_xmlwriter_write_attribute },
  { "xmlwriter_end_attribute", fg_xmlwriter_end_attribute },
  { "xmlwriter_start_cdata", fg_xmlwriter_start_cdata },
  { "xmlwriter_write_cdata", fg_xmlwriter_write_cdata },
  { "xmlwriter_end_cdata", fg_xmlwriter_end_cdata },
  { "xmlwriter_start_comment", fg_xmlwriter_start_comment },
  { "xmlwriter_write_comment", fg_xmlwriter_write_comment },
  { "xmlwriter_end_comment", fg_xmlwriter_end_comment },
  { "xmlwriter_end_document", fg_xmlwriter_end_document },
  { "xmlwriter_start_pi", fg_xmlwriter_start_pi },
  { "xmlwriter_write_pi", fg_xmlwriter_write_pi },
  { "xmlwriter_end_pi", fg_xmlwriter_end_pi },
  { "xmlwriter_text", fg_xmlwriter_text },
  { "xmlwriter_write_raw", fg_xmlwriter_write_raw },
  { "xmlwriter_start_dtd", fg_xmlwriter_start_dtd },
  { "xmlwriter_write_dtd", fg_xmlwriter_write_dtd },
  { "xmlwriter_start_dtd_element", fg_xmlwriter_start_dtd_element },
  { "xmlwriter_write_dtd_element", fg_xmlwriter_write_dtd_element },
  { "xmlwriter_end_dtd_element", fg_xmlwriter_end_dtd_element },
  { "xmlwriter_start_dtd_attlist", fg_xmlwriter_start_dtd_attlist },
  { "xmlwriter_write_dtd_attlist", fg_xmlwriter_write_dtd_attlist },
  { "xmlwriter_end_dtd_attlist", fg_xmlwriter_end_dtd_attlist },
  { "xmlwriter_start_dtd_entity", fg_xmlwriter_start_dtd_entity },
  { "xmlwriter_write_dtd_entity", fg_xmlwriter_write_dtd_entity },
  { "xmlwriter_end_dtd_entity", fg_xmlwriter_end_dtd_entity },
  { "xmlwriter_end_dtd", fg_xmlwriter_end_dtd },
  { "xmlwriter_flush", fg_xmlwriter_flush },
  { "xmlwriter_output_memory", fg_xmlwriter_output_memory },
  { "readgzfile", fg_readgzfile },
  { "gzfile", fg_gzfile },
  { "gzcompress", fg_gzcompress },
  { "gzuncompress", fg_gzuncompress },
  { "gzdeflate", fg_gzdeflate },
  { "gzinflate", fg_gzinflate },
  { "gzencode", fg_gzencode },
  { "gzdecode", fg_gzdecode },
  { "zlib_get_coding_type", fg_zlib_get_coding_type },
  { "gzopen", fg_gzopen },
  { "gzclose", fg_gzclose },
  { "gzrewind", fg_gzrewind },
  { "gzeof", fg_gzeof },
  { "gzgetc", fg_gzgetc },
  { "gzgets", fg_gzgets },
  { "gzgetss", fg_gzgetss },
  { "gzread", fg_gzread },
  { "gzpassthru", fg_gzpassthru },
  { "gzseek", fg_gzseek },
  { "gztell", fg_gztell },
  { "gzwrite", fg_gzwrite },
  { "gzputs", fg_gzputs },
  { "qlzcompress", fg_qlzcompress },
  { "qlzuncompress", fg_qlzuncompress },
  { "sncompress", fg_sncompress },
  { "snuncompress", fg_snuncompress },
  { "nzcompress", fg_nzcompress },
  { "nzuncompress", fg_nzuncompress }
};

static const long long hhbc_ext_method_count_DummyClosure = 1;
static const HhbcExtMethodInfo hhbc_ext_methods_DummyClosure[] = {
  { "__construct", tg_12DummyClosure___construct }
};

static const long long hhbc_ext_method_count_Vector = 27;
static const HhbcExtMethodInfo hhbc_ext_methods_Vector[] = {
  { "__construct", tg_6Vector___construct },
  { "isEmpty", tg_6Vector_isEmpty },
  { "count", tg_6Vector_count },
  { "at", tg_6Vector_at },
  { "get", tg_6Vector_get },
  { "put", tg_6Vector_put },
  { "clear", tg_6Vector_clear },
  { "contains", tg_6Vector_contains },
  { "append", tg_6Vector_append },
  { "add", tg_6Vector_add },
  { "pop", tg_6Vector_pop },
  { "resize", tg_6Vector_resize },
  { "toArray", tg_6Vector_toArray },
  { "getIterator", tg_6Vector_getIterator },
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

static const long long hhbc_ext_method_count_Map = 26;
static const HhbcExtMethodInfo hhbc_ext_methods_Map[] = {
  { "__construct", tg_3Map___construct },
  { "isEmpty", tg_3Map_isEmpty },
  { "count", tg_3Map_count },
  { "at", tg_3Map_at },
  { "get", tg_3Map_get },
  { "put", tg_3Map_put },
  { "clear", tg_3Map_clear },
  { "contains", tg_3Map_contains },
  { "remove", tg_3Map_remove },
  { "discard", tg_3Map_discard },
  { "toArray", tg_3Map_toArray },
  { "copyAsArray", tg_3Map_copyAsArray },
  { "toKeysArray", tg_3Map_toKeysArray },
  { "values", tg_3Map_values },
  { "toValuesArray", tg_3Map_toValuesArray },
  { "updateFromArray", tg_3Map_updateFromArray },
  { "updateFromIterable", tg_3Map_updateFromIterable },
  { "differenceByKey", tg_3Map_differenceByKey },
  { "getIterator", tg_3Map_getIterator },
  { "__toString", tg_3Map___toString },
  { "__get", tg_3Map___get },
  { "__set", tg_3Map___set },
  { "__isset", tg_3Map___isset },
  { "__unset", tg_3Map___unset },
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

static const long long hhbc_ext_method_count_StableMap = 26;
static const HhbcExtMethodInfo hhbc_ext_methods_StableMap[] = {
  { "__construct", tg_9StableMap___construct },
  { "isEmpty", tg_9StableMap_isEmpty },
  { "count", tg_9StableMap_count },
  { "at", tg_9StableMap_at },
  { "get", tg_9StableMap_get },
  { "put", tg_9StableMap_put },
  { "clear", tg_9StableMap_clear },
  { "contains", tg_9StableMap_contains },
  { "remove", tg_9StableMap_remove },
  { "discard", tg_9StableMap_discard },
  { "toArray", tg_9StableMap_toArray },
  { "copyAsArray", tg_9StableMap_copyAsArray },
  { "toKeysArray", tg_9StableMap_toKeysArray },
  { "values", tg_9StableMap_values },
  { "toValuesArray", tg_9StableMap_toValuesArray },
  { "updateFromArray", tg_9StableMap_updateFromArray },
  { "updateFromIterable", tg_9StableMap_updateFromIterable },
  { "differenceByKey", tg_9StableMap_differenceByKey },
  { "getIterator", tg_9StableMap_getIterator },
  { "__get", tg_9StableMap___get },
  { "__set", tg_9StableMap___set },
  { "__isset", tg_9StableMap___isset },
  { "__unset", tg_9StableMap___unset },
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

static const long long hhbc_ext_method_count_Continuation = 18;
static const HhbcExtMethodInfo hhbc_ext_methods_Continuation[] = {
  { "__construct", tg_12Continuation___construct },
  { "update", tg_12Continuation_update },
  { "done", tg_12Continuation_done },
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
  { "raised", tg_12Continuation_raised },
  { "receive", tg_12Continuation_receive },
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

const long long hhbc_ext_class_count = 59;
const HhbcExtClassInfo hhbc_ext_classes[] = {
  { "DummyClosure", new_DummyClosure_Instance, sizeof(c_DummyClosure), hhbc_ext_method_count_DummyClosure, hhbc_ext_methods_DummyClosure },
  { "Vector", new_Vector_Instance, sizeof(c_Vector), hhbc_ext_method_count_Vector, hhbc_ext_methods_Vector },
  { "VectorIterator", new_VectorIterator_Instance, sizeof(c_VectorIterator), hhbc_ext_method_count_VectorIterator, hhbc_ext_methods_VectorIterator },
  { "Map", new_Map_Instance, sizeof(c_Map), hhbc_ext_method_count_Map, hhbc_ext_methods_Map },
  { "MapIterator", new_MapIterator_Instance, sizeof(c_MapIterator), hhbc_ext_method_count_MapIterator, hhbc_ext_methods_MapIterator },
  { "StableMap", new_StableMap_Instance, sizeof(c_StableMap), hhbc_ext_method_count_StableMap, hhbc_ext_methods_StableMap },
  { "StableMapIterator", new_StableMapIterator_Instance, sizeof(c_StableMapIterator), hhbc_ext_method_count_StableMapIterator, hhbc_ext_methods_StableMapIterator },
  { "Continuation", new_Continuation_Instance, sizeof(c_Continuation), hhbc_ext_method_count_Continuation, hhbc_ext_methods_Continuation },
  { "DummyContinuation", new_DummyContinuation_Instance, sizeof(c_DummyContinuation), hhbc_ext_method_count_DummyContinuation, hhbc_ext_methods_DummyContinuation },
  { "DateTime", new_DateTime_Instance, sizeof(c_DateTime), hhbc_ext_method_count_DateTime, hhbc_ext_methods_DateTime },
  { "DateTimeZone", new_DateTimeZone_Instance, sizeof(c_DateTimeZone), hhbc_ext_method_count_DateTimeZone, hhbc_ext_methods_DateTimeZone },
  { "DateInterval", new_DateInterval_Instance, sizeof(c_DateInterval), hhbc_ext_method_count_DateInterval, hhbc_ext_methods_DateInterval },
  { "DebuggerProxyCmdUser", new_DebuggerProxyCmdUser_Instance, sizeof(c_DebuggerProxyCmdUser), hhbc_ext_method_count_DebuggerProxyCmdUser, hhbc_ext_methods_DebuggerProxyCmdUser },
  { "DebuggerClientCmdUser", new_DebuggerClientCmdUser_Instance, sizeof(c_DebuggerClientCmdUser), hhbc_ext_method_count_DebuggerClientCmdUser, hhbc_ext_methods_DebuggerClientCmdUser },
  { "DebuggerClient", new_DebuggerClient_Instance, sizeof(c_DebuggerClient), hhbc_ext_method_count_DebuggerClient, hhbc_ext_methods_DebuggerClient },
  { "DOMNode", new_DOMNode_Instance, sizeof(c_DOMNode), hhbc_ext_method_count_DOMNode, hhbc_ext_methods_DOMNode },
  { "DOMAttr", new_DOMAttr_Instance, sizeof(c_DOMAttr), hhbc_ext_method_count_DOMAttr, hhbc_ext_methods_DOMAttr },
  { "DOMCharacterData", new_DOMCharacterData_Instance, sizeof(c_DOMCharacterData), hhbc_ext_method_count_DOMCharacterData, hhbc_ext_methods_DOMCharacterData },
  { "DOMComment", new_DOMComment_Instance, sizeof(c_DOMComment), hhbc_ext_method_count_DOMComment, hhbc_ext_methods_DOMComment },
  { "DOMText", new_DOMText_Instance, sizeof(c_DOMText), hhbc_ext_method_count_DOMText, hhbc_ext_methods_DOMText },
  { "DOMCDATASection", new_DOMCDATASection_Instance, sizeof(c_DOMCDATASection), hhbc_ext_method_count_DOMCDATASection, hhbc_ext_methods_DOMCDATASection },
  { "DOMDocument", new_DOMDocument_Instance, sizeof(c_DOMDocument), hhbc_ext_method_count_DOMDocument, hhbc_ext_methods_DOMDocument },
  { "DOMDocumentFragment", new_DOMDocumentFragment_Instance, sizeof(c_DOMDocumentFragment), hhbc_ext_method_count_DOMDocumentFragment, hhbc_ext_methods_DOMDocumentFragment },
  { "DOMDocumentType", new_DOMDocumentType_Instance, sizeof(c_DOMDocumentType), hhbc_ext_method_count_DOMDocumentType, hhbc_ext_methods_DOMDocumentType },
  { "DOMElement", new_DOMElement_Instance, sizeof(c_DOMElement), hhbc_ext_method_count_DOMElement, hhbc_ext_methods_DOMElement },
  { "DOMEntity", new_DOMEntity_Instance, sizeof(c_DOMEntity), hhbc_ext_method_count_DOMEntity, hhbc_ext_methods_DOMEntity },
  { "DOMEntityReference", new_DOMEntityReference_Instance, sizeof(c_DOMEntityReference), hhbc_ext_method_count_DOMEntityReference, hhbc_ext_methods_DOMEntityReference },
  { "DOMNotation", new_DOMNotation_Instance, sizeof(c_DOMNotation), hhbc_ext_method_count_DOMNotation, hhbc_ext_methods_DOMNotation },
  { "DOMProcessingInstruction", new_DOMProcessingInstruction_Instance, sizeof(c_DOMProcessingInstruction), hhbc_ext_method_count_DOMProcessingInstruction, hhbc_ext_methods_DOMProcessingInstruction },
  { "DOMNodeIterator", new_DOMNodeIterator_Instance, sizeof(c_DOMNodeIterator), hhbc_ext_method_count_DOMNodeIterator, hhbc_ext_methods_DOMNodeIterator },
  { "DOMNamedNodeMap", new_DOMNamedNodeMap_Instance, sizeof(c_DOMNamedNodeMap), hhbc_ext_method_count_DOMNamedNodeMap, hhbc_ext_methods_DOMNamedNodeMap },
  { "DOMNodeList", new_DOMNodeList_Instance, sizeof(c_DOMNodeList), hhbc_ext_method_count_DOMNodeList, hhbc_ext_methods_DOMNodeList },
  { "DOMImplementation", new_DOMImplementation_Instance, sizeof(c_DOMImplementation), hhbc_ext_method_count_DOMImplementation, hhbc_ext_methods_DOMImplementation },
  { "DOMXPath", new_DOMXPath_Instance, sizeof(c_DOMXPath), hhbc_ext_method_count_DOMXPath, hhbc_ext_methods_DOMXPath },
  { "EncodingDetector", new_EncodingDetector_Instance, sizeof(c_EncodingDetector), hhbc_ext_method_count_EncodingDetector, hhbc_ext_methods_EncodingDetector },
  { "EncodingMatch", new_EncodingMatch_Instance, sizeof(c_EncodingMatch), hhbc_ext_method_count_EncodingMatch, hhbc_ext_methods_EncodingMatch },
  { "SpoofChecker", new_SpoofChecker_Instance, sizeof(c_SpoofChecker), hhbc_ext_method_count_SpoofChecker, hhbc_ext_methods_SpoofChecker },
  { "ImageSprite", new_ImageSprite_Instance, sizeof(c_ImageSprite), hhbc_ext_method_count_ImageSprite, hhbc_ext_methods_ImageSprite },
  { "Collator", new_Collator_Instance, sizeof(c_Collator), hhbc_ext_method_count_Collator, hhbc_ext_methods_Collator },
  { "Locale", new_Locale_Instance, sizeof(c_Locale), hhbc_ext_method_count_Locale, hhbc_ext_methods_Locale },
  { "Normalizer", new_Normalizer_Instance, sizeof(c_Normalizer), hhbc_ext_method_count_Normalizer, hhbc_ext_methods_Normalizer },
  { "MutableArrayIterator", new_MutableArrayIterator_Instance, sizeof(c_MutableArrayIterator), hhbc_ext_method_count_MutableArrayIterator, hhbc_ext_methods_MutableArrayIterator },
  { "Memcache", new_Memcache_Instance, sizeof(c_Memcache), hhbc_ext_method_count_Memcache, hhbc_ext_methods_Memcache },
  { "Memcached", new_Memcached_Instance, sizeof(c_Memcached), hhbc_ext_method_count_Memcached, hhbc_ext_methods_Memcached },
  { "PDO", new_PDO_Instance, sizeof(c_PDO), hhbc_ext_method_count_PDO, hhbc_ext_methods_PDO },
  { "PDOStatement", new_PDOStatement_Instance, sizeof(c_PDOStatement), hhbc_ext_method_count_PDOStatement, hhbc_ext_methods_PDOStatement },
  { "SimpleXMLElement", new_SimpleXMLElement_Instance, sizeof(c_SimpleXMLElement), hhbc_ext_method_count_SimpleXMLElement, hhbc_ext_methods_SimpleXMLElement },
  { "LibXMLError", new_LibXMLError_Instance, sizeof(c_LibXMLError), hhbc_ext_method_count_LibXMLError, hhbc_ext_methods_LibXMLError },
  { "SimpleXMLElementIterator", new_SimpleXMLElementIterator_Instance, sizeof(c_SimpleXMLElementIterator), hhbc_ext_method_count_SimpleXMLElementIterator, hhbc_ext_methods_SimpleXMLElementIterator },
  { "SoapServer", new_SoapServer_Instance, sizeof(c_SoapServer), hhbc_ext_method_count_SoapServer, hhbc_ext_methods_SoapServer },
  { "SoapClient", new_SoapClient_Instance, sizeof(c_SoapClient), hhbc_ext_method_count_SoapClient, hhbc_ext_methods_SoapClient },
  { "SoapVar", new_SoapVar_Instance, sizeof(c_SoapVar), hhbc_ext_method_count_SoapVar, hhbc_ext_methods_SoapVar },
  { "SoapParam", new_SoapParam_Instance, sizeof(c_SoapParam), hhbc_ext_method_count_SoapParam, hhbc_ext_methods_SoapParam },
  { "SoapHeader", new_SoapHeader_Instance, sizeof(c_SoapHeader), hhbc_ext_method_count_SoapHeader, hhbc_ext_methods_SoapHeader },
  { "SQLite3", new_SQLite3_Instance, sizeof(c_SQLite3), hhbc_ext_method_count_SQLite3, hhbc_ext_methods_SQLite3 },
  { "SQLite3Stmt", new_SQLite3Stmt_Instance, sizeof(c_SQLite3Stmt), hhbc_ext_method_count_SQLite3Stmt, hhbc_ext_methods_SQLite3Stmt },
  { "SQLite3Result", new_SQLite3Result_Instance, sizeof(c_SQLite3Result), hhbc_ext_method_count_SQLite3Result, hhbc_ext_methods_SQLite3Result },
  { "XMLReader", new_XMLReader_Instance, sizeof(c_XMLReader), hhbc_ext_method_count_XMLReader, hhbc_ext_methods_XMLReader },
  { "XMLWriter", new_XMLWriter_Instance, sizeof(c_XMLWriter), hhbc_ext_method_count_XMLWriter, hhbc_ext_methods_XMLWriter }
};


} // !HPHP

