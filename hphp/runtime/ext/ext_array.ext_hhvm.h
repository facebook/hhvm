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
HPHP::Variant HPHP::f_array_change_key_case(HPHP::Variant const&, bool)
_ZN4HPHP23f_array_change_key_caseERKNS_7VariantEb

(return value) => rax
_rv => rdi
input => rsi
upper => rdx
*/

TypedValue* fh_array_change_key_case(TypedValue* _rv, TypedValue* input, bool upper) asm("_ZN4HPHP23f_array_change_key_caseERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_array_chunk(HPHP::Variant const&, int, bool)
_ZN4HPHP13f_array_chunkERKNS_7VariantEib

(return value) => rax
_rv => rdi
input => rsi
size => rdx
preserve_keys => rcx
*/

TypedValue* fh_array_chunk(TypedValue* _rv, TypedValue* input, int size, bool preserve_keys) asm("_ZN4HPHP13f_array_chunkERKNS_7VariantEib");

/*
HPHP::Variant HPHP::f_array_combine(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP15f_array_combineERKNS_7VariantES2_

(return value) => rax
_rv => rdi
keys => rsi
values => rdx
*/

TypedValue* fh_array_combine(TypedValue* _rv, TypedValue* keys, TypedValue* values) asm("_ZN4HPHP15f_array_combineERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_array_count_values(HPHP::Variant const&)
_ZN4HPHP20f_array_count_valuesERKNS_7VariantE

(return value) => rax
_rv => rdi
input => rsi
*/

TypedValue* fh_array_count_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP20f_array_count_valuesERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_array_fill_keys(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP17f_array_fill_keysERKNS_7VariantES2_

(return value) => rax
_rv => rdi
keys => rsi
value => rdx
*/

TypedValue* fh_array_fill_keys(TypedValue* _rv, TypedValue* keys, TypedValue* value) asm("_ZN4HPHP17f_array_fill_keysERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_array_filter(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_array_filterERKNS_7VariantES2_

(return value) => rax
_rv => rdi
input => rsi
callback => rdx
*/

TypedValue* fh_array_filter(TypedValue* _rv, TypedValue* input, TypedValue* callback) asm("_ZN4HPHP14f_array_filterERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_array_flip(HPHP::Variant const&)
_ZN4HPHP12f_array_flipERKNS_7VariantE

(return value) => rax
_rv => rdi
trans => rsi
*/

TypedValue* fh_array_flip(TypedValue* _rv, TypedValue* trans) asm("_ZN4HPHP12f_array_flipERKNS_7VariantE");

/*
bool HPHP::f_array_key_exists(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP18f_array_key_existsERKNS_7VariantES2_

(return value) => rax
key => rdi
search => rsi
*/

bool fh_array_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP18f_array_key_existsERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_array_keys(HPHP::Variant const&, HPHP::Variant const&, bool)
_ZN4HPHP12f_array_keysERKNS_7VariantES2_b

(return value) => rax
_rv => rdi
input => rsi
search_value => rdx
strict => rcx
*/

TypedValue* fh_array_keys(TypedValue* _rv, TypedValue* input, TypedValue* search_value, bool strict) asm("_ZN4HPHP12f_array_keysERKNS_7VariantES2_b");

/*
HPHP::Variant HPHP::f_array_map(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP11f_array_mapEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
callback => rdx
arr1 => rcx
_argv => r8
*/

TypedValue* fh_array_map(TypedValue* _rv, long long _argc, TypedValue* callback, TypedValue* arr1, Value* _argv) asm("_ZN4HPHP11f_array_mapEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_merge_recursive(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP23f_array_merge_recursiveEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_merge_recursive(TypedValue* _rv, long long _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP23f_array_merge_recursiveEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_merge(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP13f_array_mergeEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_merge(TypedValue* _rv, long long _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP13f_array_mergeEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_replace_recursive(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP25f_array_replace_recursiveEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_replace_recursive(TypedValue* _rv, long long _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP25f_array_replace_recursiveEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_replace(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP15f_array_replaceEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_replace(TypedValue* _rv, long long _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP15f_array_replaceEiRKNS_7VariantERKNS_5ArrayE");

/*
bool HPHP::f_array_multisort(int, HPHP::VRefParamValue const&, HPHP::Array const&)
_ZN4HPHP17f_array_multisortEiRKNS_14VRefParamValueERKNS_5ArrayE

(return value) => rax
_argc => rdi
ar1 => rsi
_argv => rdx
*/

bool fh_array_multisort(long long _argc, TypedValue* ar1, Value* _argv) asm("_ZN4HPHP17f_array_multisortEiRKNS_14VRefParamValueERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_pad(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP11f_array_padERKNS_7VariantEiS2_

(return value) => rax
_rv => rdi
input => rsi
pad_size => rdx
pad_value => rcx
*/

TypedValue* fh_array_pad(TypedValue* _rv, TypedValue* input, int pad_size, TypedValue* pad_value) asm("_ZN4HPHP11f_array_padERKNS_7VariantEiS2_");

/*
HPHP::Variant HPHP::f_array_product(HPHP::Variant const&)
_ZN4HPHP15f_array_productERKNS_7VariantE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_product(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP15f_array_productERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_array_push(int, HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP12f_array_pushEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array => rdx
var => rcx
_argv => r8
*/

TypedValue* fh_array_push(TypedValue* _rv, long long _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP12f_array_pushEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_rand(HPHP::Variant const&, int)
_ZN4HPHP12f_array_randERKNS_7VariantEi

(return value) => rax
_rv => rdi
input => rsi
num_req => rdx
*/

TypedValue* fh_array_rand(TypedValue* _rv, TypedValue* input, int num_req) asm("_ZN4HPHP12f_array_randERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_array_reduce(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_array_reduceERKNS_7VariantES2_S2_

(return value) => rax
_rv => rdi
input => rsi
callback => rdx
initial => rcx
*/

TypedValue* fh_array_reduce(TypedValue* _rv, TypedValue* input, TypedValue* callback, TypedValue* initial) asm("_ZN4HPHP14f_array_reduceERKNS_7VariantES2_S2_");

/*
HPHP::Variant HPHP::f_array_reverse(HPHP::Variant const&, bool)
_ZN4HPHP15f_array_reverseERKNS_7VariantEb

(return value) => rax
_rv => rdi
array => rsi
preserve_keys => rdx
*/

TypedValue* fh_array_reverse(TypedValue* _rv, TypedValue* array, bool preserve_keys) asm("_ZN4HPHP15f_array_reverseERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_array_search(HPHP::Variant const&, HPHP::Variant const&, bool)
_ZN4HPHP14f_array_searchERKNS_7VariantES2_b

(return value) => rax
_rv => rdi
needle => rsi
haystack => rdx
strict => rcx
*/

TypedValue* fh_array_search(TypedValue* _rv, TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP14f_array_searchERKNS_7VariantES2_b");

/*
HPHP::Variant HPHP::f_array_slice(HPHP::Variant const&, int, HPHP::Variant const&, bool)
_ZN4HPHP13f_array_sliceERKNS_7VariantEiS2_b

(return value) => rax
_rv => rdi
array => rsi
offset => rdx
length => rcx
preserve_keys => r8
*/

TypedValue* fh_array_slice(TypedValue* _rv, TypedValue* array, int offset, TypedValue* length, bool preserve_keys) asm("_ZN4HPHP13f_array_sliceERKNS_7VariantEiS2_b");

/*
HPHP::Variant HPHP::f_array_splice(HPHP::VRefParamValue const&, int, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_array_spliceERKNS_14VRefParamValueEiRKNS_7VariantES5_

(return value) => rax
_rv => rdi
input => rsi
offset => rdx
length => rcx
replacement => r8
*/

TypedValue* fh_array_splice(TypedValue* _rv, TypedValue* input, int offset, TypedValue* length, TypedValue* replacement) asm("_ZN4HPHP14f_array_spliceERKNS_14VRefParamValueEiRKNS_7VariantES5_");

/*
HPHP::Variant HPHP::f_array_sum(HPHP::Variant const&)
_ZN4HPHP11f_array_sumERKNS_7VariantE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_sum(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11f_array_sumERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_array_unique(HPHP::Variant const&, int)
_ZN4HPHP14f_array_uniqueERKNS_7VariantEi

(return value) => rax
_rv => rdi
array => rsi
sort_flags => rdx
*/

TypedValue* fh_array_unique(TypedValue* _rv, TypedValue* array, int sort_flags) asm("_ZN4HPHP14f_array_uniqueERKNS_7VariantEi");

/*
long long HPHP::f_array_unshift(int, HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP15f_array_unshiftEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_argc => rdi
array => rsi
var => rdx
_argv => rcx
*/

long long fh_array_unshift(long long _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP15f_array_unshiftEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_values(HPHP::Variant const&)
_ZN4HPHP14f_array_valuesERKNS_7VariantE

(return value) => rax
_rv => rdi
input => rsi
*/

TypedValue* fh_array_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP14f_array_valuesERKNS_7VariantE");

/*
bool HPHP::f_array_walk_recursive(HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP22f_array_walk_recursiveERKNS_14VRefParamValueERKNS_7VariantES5_

(return value) => rax
input => rdi
funcname => rsi
userdata => rdx
*/

bool fh_array_walk_recursive(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP22f_array_walk_recursiveERKNS_14VRefParamValueERKNS_7VariantES5_");

/*
bool HPHP::f_array_walk(HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP12f_array_walkERKNS_14VRefParamValueERKNS_7VariantES5_

(return value) => rax
input => rdi
funcname => rsi
userdata => rdx
*/

bool fh_array_walk(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP12f_array_walkERKNS_14VRefParamValueERKNS_7VariantES5_");

/*
HPHP::Array HPHP::f_compact(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP9f_compactEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
varname => rdx
_argv => rcx
*/

Value* fh_compact(Value* _rv, long long _argc, TypedValue* varname, Value* _argv) asm("_ZN4HPHP9f_compactEiRKNS_7VariantERKNS_5ArrayE");

/*
bool HPHP::f_shuffle(HPHP::VRefParamValue const&)
_ZN4HPHP9f_shuffleERKNS_14VRefParamValueE

(return value) => rax
array => rdi
*/

bool fh_shuffle(TypedValue* array) asm("_ZN4HPHP9f_shuffleERKNS_14VRefParamValueE");

/*
long long HPHP::f_count(HPHP::Variant const&, bool)
_ZN4HPHP7f_countERKNS_7VariantEb

(return value) => rax
var => rdi
recursive => rsi
*/

long long fh_count(TypedValue* var, bool recursive) asm("_ZN4HPHP7f_countERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_hphp_get_iterator(HPHP::Variant const&)
_ZN4HPHP19f_hphp_get_iteratorERKNS_7VariantE

(return value) => rax
_rv => rdi
iterable => rsi
*/

TypedValue* fh_hphp_get_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP19f_hphp_get_iteratorERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_hphp_get_mutable_iterator(HPHP::VRefParamValue const&)
_ZN4HPHP27f_hphp_get_mutable_iteratorERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
iterable => rsi
*/

TypedValue* fh_hphp_get_mutable_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP27f_hphp_get_mutable_iteratorERKNS_14VRefParamValueE");

/*
bool HPHP::f_in_array(HPHP::Variant const&, HPHP::Variant const&, bool)
_ZN4HPHP10f_in_arrayERKNS_7VariantES2_b

(return value) => rax
needle => rdi
haystack => rsi
strict => rdx
*/

bool fh_in_array(TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP10f_in_arrayERKNS_7VariantES2_b");

/*
HPHP::Variant HPHP::f_range(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP7f_rangeERKNS_7VariantES2_S2_

(return value) => rax
_rv => rdi
low => rsi
high => rdx
step => rcx
*/

TypedValue* fh_range(TypedValue* _rv, TypedValue* low, TypedValue* high, TypedValue* step) asm("_ZN4HPHP7f_rangeERKNS_7VariantES2_S2_");

/*
HPHP::Variant HPHP::f_array_diff(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP12f_array_diffEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_diff(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP12f_array_diffEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_udiff(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP13f_array_udiffEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_udiff(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP13f_array_udiffEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_diff_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP18f_array_diff_assocEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_diff_assoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP18f_array_diff_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_diff_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP19f_array_diff_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_diff_uassoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_diff_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_udiff_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP19f_array_udiff_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_udiff_assoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_udiff_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_udiff_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP20f_array_udiff_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
key_compare_func => r9
_argv => st0
*/

TypedValue* fh_array_udiff_uassoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP20f_array_udiff_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_diff_key(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP16f_array_diff_keyEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_diff_key(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP16f_array_diff_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_diff_ukey(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP17f_array_diff_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_diff_ukey(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP17f_array_diff_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_intersect(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP17f_array_intersectEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_intersect(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP17f_array_intersectEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_uintersect(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP18f_array_uintersectEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_uintersect(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP18f_array_uintersectEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_intersect_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP23f_array_intersect_assocEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_intersect_assoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP23f_array_intersect_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_intersect_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24f_array_intersect_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_intersect_uassoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_intersect_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_uintersect_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24f_array_uintersect_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_uintersect_assoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_uintersect_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_uintersect_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP25f_array_uintersect_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
key_compare_func => r9
_argv => st0
*/

TypedValue* fh_array_uintersect_uassoc(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP25f_array_uintersect_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_intersect_key(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP21f_array_intersect_keyEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_intersect_key(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP21f_array_intersect_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_array_intersect_ukey(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP22f_array_intersect_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_intersect_ukey(TypedValue* _rv, long long _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP22f_array_intersect_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

/*
bool HPHP::f_sort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP6f_sortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_sort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP6f_sortERKNS_14VRefParamValueEib");

/*
bool HPHP::f_rsort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP7f_rsortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_rsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_rsortERKNS_14VRefParamValueEib");

/*
bool HPHP::f_asort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP7f_asortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_asort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_asortERKNS_14VRefParamValueEib");

/*
bool HPHP::f_arsort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP8f_arsortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_arsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP8f_arsortERKNS_14VRefParamValueEib");

/*
bool HPHP::f_ksort(HPHP::VRefParamValue const&, int)
_ZN4HPHP7f_ksortERKNS_14VRefParamValueEi

(return value) => rax
array => rdi
sort_flags => rsi
*/

bool fh_ksort(TypedValue* array, int sort_flags) asm("_ZN4HPHP7f_ksortERKNS_14VRefParamValueEi");

/*
bool HPHP::f_krsort(HPHP::VRefParamValue const&, int)
_ZN4HPHP8f_krsortERKNS_14VRefParamValueEi

(return value) => rax
array => rdi
sort_flags => rsi
*/

bool fh_krsort(TypedValue* array, int sort_flags) asm("_ZN4HPHP8f_krsortERKNS_14VRefParamValueEi");

/*
bool HPHP::f_usort(HPHP::VRefParamValue const&, HPHP::Variant const&)
_ZN4HPHP7f_usortERKNS_14VRefParamValueERKNS_7VariantE

(return value) => rax
array => rdi
cmp_function => rsi
*/

bool fh_usort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP7f_usortERKNS_14VRefParamValueERKNS_7VariantE");

/*
bool HPHP::f_uasort(HPHP::VRefParamValue const&, HPHP::Variant const&)
_ZN4HPHP8f_uasortERKNS_14VRefParamValueERKNS_7VariantE

(return value) => rax
array => rdi
cmp_function => rsi
*/

bool fh_uasort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uasortERKNS_14VRefParamValueERKNS_7VariantE");

/*
bool HPHP::f_uksort(HPHP::VRefParamValue const&, HPHP::Variant const&)
_ZN4HPHP8f_uksortERKNS_14VRefParamValueERKNS_7VariantE

(return value) => rax
array => rdi
cmp_function => rsi
*/

bool fh_uksort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uksortERKNS_14VRefParamValueERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_natsort(HPHP::VRefParamValue const&)
_ZN4HPHP9f_natsortERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_natsort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9f_natsortERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_natcasesort(HPHP::VRefParamValue const&)
_ZN4HPHP13f_natcasesortERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_natcasesort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13f_natcasesortERKNS_14VRefParamValueE");

/*
HPHP::String HPHP::f_i18n_loc_get_default()
_ZN4HPHP22f_i18n_loc_get_defaultEv

(return value) => rax
_rv => rdi
*/

Value* fh_i18n_loc_get_default(Value* _rv) asm("_ZN4HPHP22f_i18n_loc_get_defaultEv");

/*
bool HPHP::f_i18n_loc_set_default(HPHP::String const&)
_ZN4HPHP22f_i18n_loc_set_defaultERKNS_6StringE

(return value) => rax
locale => rdi
*/

bool fh_i18n_loc_set_default(Value* locale) asm("_ZN4HPHP22f_i18n_loc_set_defaultERKNS_6StringE");

/*
bool HPHP::f_i18n_loc_set_attribute(long long, long long)
_ZN4HPHP24f_i18n_loc_set_attributeExx

(return value) => rax
attr => rdi
val => rsi
*/

bool fh_i18n_loc_set_attribute(long long attr, long long val) asm("_ZN4HPHP24f_i18n_loc_set_attributeExx");

/*
bool HPHP::f_i18n_loc_set_strength(long long)
_ZN4HPHP23f_i18n_loc_set_strengthEx

(return value) => rax
strength => rdi
*/

bool fh_i18n_loc_set_strength(long long strength) asm("_ZN4HPHP23f_i18n_loc_set_strengthEx");

/*
HPHP::Variant HPHP::f_i18n_loc_get_error_code()
_ZN4HPHP25f_i18n_loc_get_error_codeEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_i18n_loc_get_error_code(TypedValue* _rv) asm("_ZN4HPHP25f_i18n_loc_get_error_codeEv");


} // !HPHP

