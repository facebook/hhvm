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

TypedValue* fh_array_change_key_case(TypedValue* _rv, TypedValue* input, bool upper) asm("_ZN4HPHP23f_array_change_key_caseERKNS_7VariantEb");

TypedValue* fh_array_chunk(TypedValue* _rv, TypedValue* input, int size, bool preserve_keys) asm("_ZN4HPHP13f_array_chunkERKNS_7VariantEib");

TypedValue* fh_array_column(TypedValue* _rv, TypedValue* arr, TypedValue* val_key, TypedValue* idx_key) asm("_ZN4HPHP14f_array_columnERKNS_7VariantES2_S2_");

TypedValue* fh_array_combine(TypedValue* _rv, TypedValue* keys, TypedValue* values) asm("_ZN4HPHP15f_array_combineERKNS_7VariantES2_");

TypedValue* fh_array_count_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP20f_array_count_valuesERKNS_7VariantE");

TypedValue* fh_array_fill_keys(TypedValue* _rv, TypedValue* keys, TypedValue* value) asm("_ZN4HPHP17f_array_fill_keysERKNS_7VariantES2_");

TypedValue* fh_array_fill(TypedValue* _rv, int start_index, int num, TypedValue* value) asm("_ZN4HPHP12f_array_fillEiiRKNS_7VariantE");

TypedValue* fh_array_filter(TypedValue* _rv, TypedValue* input, TypedValue* callback) asm("_ZN4HPHP14f_array_filterERKNS_7VariantES2_");

TypedValue* fh_array_flip(TypedValue* _rv, TypedValue* trans) asm("_ZN4HPHP12f_array_flipERKNS_7VariantE");

bool fh_array_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP18f_array_key_existsERKNS_7VariantES2_");

bool fh_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP12f_key_existsERKNS_7VariantES2_");

TypedValue* fh_array_keys(TypedValue* _rv, TypedValue* input, TypedValue* search_value, bool strict) asm("_ZN4HPHP12f_array_keysERKNS_7VariantES2_b");

TypedValue* fh_array_map(TypedValue* _rv, int64_t _argc, TypedValue* callback, TypedValue* arr1, Value* _argv) asm("_ZN4HPHP11f_array_mapEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_array_merge(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP13f_array_mergeEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_array_merge_recursive(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP23f_array_merge_recursiveEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_array_replace(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP15f_array_replaceEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_array_replace_recursive(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP25f_array_replace_recursiveEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_array_pad(TypedValue* _rv, TypedValue* input, int pad_size, TypedValue* pad_value) asm("_ZN4HPHP11f_array_padERKNS_7VariantEiS2_");

TypedValue* fh_array_pop(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11f_array_popERKNS_14VRefParamValueE");

TypedValue* fh_array_product(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP15f_array_productERKNS_7VariantE");

TypedValue* fh_array_push(TypedValue* _rv, int64_t _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP12f_array_pushEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_array_rand(TypedValue* _rv, TypedValue* input, int num_req) asm("_ZN4HPHP12f_array_randERKNS_7VariantEi");

TypedValue* fh_array_reduce(TypedValue* _rv, TypedValue* input, TypedValue* callback, TypedValue* initial) asm("_ZN4HPHP14f_array_reduceERKNS_7VariantES2_S2_");

TypedValue* fh_array_reverse(TypedValue* _rv, TypedValue* array, bool preserve_keys) asm("_ZN4HPHP15f_array_reverseERKNS_7VariantEb");

TypedValue* fh_array_search(TypedValue* _rv, TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP14f_array_searchERKNS_7VariantES2_b");

TypedValue* fh_array_shift(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13f_array_shiftERKNS_14VRefParamValueE");

TypedValue* fh_array_slice(TypedValue* _rv, TypedValue* array, int offset, TypedValue* length, bool preserve_keys) asm("_ZN4HPHP13f_array_sliceERKNS_7VariantEiS2_b");

TypedValue* fh_array_splice(TypedValue* _rv, TypedValue* input, int offset, TypedValue* length, TypedValue* replacement) asm("_ZN4HPHP14f_array_spliceERKNS_14VRefParamValueEiRKNS_7VariantES5_");

TypedValue* fh_array_sum(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11f_array_sumERKNS_7VariantE");

long fh_array_unshift(int64_t _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP15f_array_unshiftEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_array_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP14f_array_valuesERKNS_7VariantE");

bool fh_array_walk_recursive(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP22f_array_walk_recursiveERKNS_14VRefParamValueERKNS_7VariantES5_");

bool fh_array_walk(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP12f_array_walkERKNS_14VRefParamValueERKNS_7VariantES5_");

Value* fh_compact(Value* _rv, int64_t _argc, TypedValue* varname, Value* _argv) asm("_ZN4HPHP9f_compactEiRKNS_7VariantERKNS_5ArrayE");

bool fh_shuffle(TypedValue* array) asm("_ZN4HPHP9f_shuffleERKNS_14VRefParamValueE");

long fh_count(TypedValue* var, bool recursive) asm("_ZN4HPHP7f_countERKNS_7VariantEb");

long fh_sizeof(TypedValue* var, bool recursive) asm("_ZN4HPHP8f_sizeofERKNS_7VariantEb");

TypedValue* fh_each(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_eachERKNS_14VRefParamValueE");

TypedValue* fh_current(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9f_currentERKNS_14VRefParamValueE");

TypedValue* fh_hphp_current_ref(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP18f_hphp_current_refERKNS_14VRefParamValueE");

TypedValue* fh_next(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_nextERKNS_14VRefParamValueE");

TypedValue* fh_pos(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_posERKNS_14VRefParamValueE");

TypedValue* fh_prev(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_prevERKNS_14VRefParamValueE");

TypedValue* fh_reset(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7f_resetERKNS_14VRefParamValueE");

TypedValue* fh_end(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_endERKNS_14VRefParamValueE");

TypedValue* fh_key(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_keyERKNS_14VRefParamValueE");

TypedValue* fh_hphp_get_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP19f_hphp_get_iteratorERKNS_7VariantE");

TypedValue* fh_hphp_get_mutable_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP27f_hphp_get_mutable_iteratorERKNS_14VRefParamValueE");

bool fh_in_array(TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP10f_in_arrayERKNS_7VariantES2_b");

TypedValue* fh_range(TypedValue* _rv, TypedValue* low, TypedValue* high, TypedValue* step) asm("_ZN4HPHP7f_rangeERKNS_7VariantES2_S2_");

TypedValue* fh_array_diff(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP12f_array_diffEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_array_udiff(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP13f_array_udiffEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fh_array_diff_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP18f_array_diff_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_array_diff_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_diff_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fh_array_udiff_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_udiff_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fh_array_udiff_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP20f_array_udiff_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

TypedValue* fh_array_diff_key(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP16f_array_diff_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_array_diff_ukey(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP17f_array_diff_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fh_array_intersect(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP17f_array_intersectEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_array_uintersect(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP18f_array_uintersectEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fh_array_intersect_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP23f_array_intersect_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_array_intersect_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_intersect_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fh_array_uintersect_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_uintersect_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fh_array_uintersect_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP25f_array_uintersect_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

TypedValue* fh_array_intersect_key(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP21f_array_intersect_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_array_intersect_ukey(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP22f_array_intersect_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

bool fh_sort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP6f_sortERKNS_14VRefParamValueEib");

bool fh_rsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_rsortERKNS_14VRefParamValueEib");

bool fh_asort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_asortERKNS_14VRefParamValueEib");

bool fh_arsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP8f_arsortERKNS_14VRefParamValueEib");

bool fh_ksort(TypedValue* array, int sort_flags) asm("_ZN4HPHP7f_ksortERKNS_14VRefParamValueEi");

bool fh_krsort(TypedValue* array, int sort_flags) asm("_ZN4HPHP8f_krsortERKNS_14VRefParamValueEi");

TypedValue* fh_natsort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9f_natsortERKNS_14VRefParamValueE");

TypedValue* fh_natcasesort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13f_natcasesortERKNS_14VRefParamValueE");

bool fh_usort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP7f_usortERKNS_14VRefParamValueERKNS_7VariantE");

bool fh_uasort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uasortERKNS_14VRefParamValueERKNS_7VariantE");

bool fh_uksort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uksortERKNS_14VRefParamValueERKNS_7VariantE");

bool fh_array_multisort(int64_t _argc, TypedValue* ar1, Value* _argv) asm("_ZN4HPHP17f_array_multisortEiRKNS_14VRefParamValueERKNS_5ArrayE");

TypedValue* fh_array_unique(TypedValue* _rv, TypedValue* array, int sort_flags) asm("_ZN4HPHP14f_array_uniqueERKNS_7VariantEi");

Value* fh_i18n_loc_get_default(Value* _rv) asm("_ZN4HPHP22f_i18n_loc_get_defaultEv");

bool fh_i18n_loc_set_default(Value* locale) asm("_ZN4HPHP22f_i18n_loc_set_defaultERKNS_6StringE");

bool fh_i18n_loc_set_attribute(long attr, long val) asm("_ZN4HPHP24f_i18n_loc_set_attributeEll");

bool fh_i18n_loc_set_strength(long strength) asm("_ZN4HPHP23f_i18n_loc_set_strengthEl");

TypedValue* fh_i18n_loc_get_error_code(TypedValue* _rv) asm("_ZN4HPHP25f_i18n_loc_get_error_codeEv");

} // namespace HPHP
