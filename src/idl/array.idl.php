<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

k("UCOL_DEFAULT",                  Int64);

k("UCOL_PRIMARY",                  Int64);
k("UCOL_SECONDARY",                Int64);
k("UCOL_TERTIARY",                 Int64);
k("UCOL_DEFAULT_STRENGTH",         Int64);
k("UCOL_QUATERNARY",               Int64);
k("UCOL_IDENTICAL",                Int64);

k("UCOL_OFF",                      Int64);
k("UCOL_ON",                       Int64);

k("UCOL_SHIFTED",                  Int64);
k("UCOL_NON_IGNORABLE",            Int64);

k("UCOL_LOWER_FIRST",              Int64);
k("UCOL_UPPER_FIRST",              Int64);

k("UCOL_FRENCH_COLLATION",         Int64);
k("UCOL_ALTERNATE_HANDLING",       Int64);
k("UCOL_CASE_FIRST",               Int64);
k("UCOL_CASE_LEVEL",               Int64);
k("UCOL_NORMALIZATION_MODE",       Int64);
k("UCOL_STRENGTH",                 Int64);
k("UCOL_HIRAGANA_QUATERNARY_MODE", Int64);
k("UCOL_NUMERIC_COLLATION",        Int64);


f('array_change_key_case', Variant,
  array('input' => Variant,
        'upper' => array(Boolean, 'false')));

f('array_chunk', Variant,
  array('input' => Variant,
        'size' => Int32,
        'preserve_keys' => array(Boolean, 'false')));

f('array_combine', Variant,
  array('keys' => Variant,
        'values' => Variant));

f('array_count_values', Variant,
  array('input' => Variant));

f('array_fill_keys', Variant,
  array('keys' => Variant,
        'value' => Variant));

f('array_fill', Variant,
  array('start_index' => Int32,
        'num' => Int32,
        'value' => Variant));

f('array_filter', Variant,
  array('input' => Variant,
        'callback' => array(Variant, 'null_variant')));

f('array_flip', Variant,
  array('trans' => Variant));

f('array_key_exists', Boolean,
  array('key' => Variant,
        'search' => Variant));

f('array_keys', Variant,
  array('input' => Variant,
        'search_value' => array(Variant, 'null_variant'),
        'strict' => array(Boolean, 'false')));

f('array_map', Variant,
  array('callback' => Variant,
        'arr1' => Variant),
  VariableArguments);

f('array_merge_recursive', Variant,
  array('array1' => Variant),
  VariableArguments);

f('array_merge', Variant,
  array('array1' => Variant),
  VariableArguments);

f('array_replace_recursive', Variant,
  array('array1' => Variant),
  VariableArguments);

f('array_replace', Variant,
  array('array1' => Variant),
  VariableArguments);

f('array_multisort', Boolean,
  array('ar1' => Variant | Reference),
  ReferenceVariableArguments);

f('array_pad', Variant,
  array('input' => Variant,
        'pad_size' => Int32,
        'pad_value' => Variant));

f('array_pop', Variant,
  array('array' => VariantMap | Reference));

f('array_product', Variant,
  array('array' => Variant));

f('array_push', Variant,
  array('array' => VariantMap | Reference,
        'var' => Variant),
  VariableArguments);

f('array_rand', Variant,
  array('input' => Variant,
        'num_req' => array(Int32, '1')));

f('array_reduce', Variant,
  array('input' => Variant,
        'callback' => Variant,
        'initial' => array(Variant, 'null_variant')));

f('array_reverse', Variant,
  array('array' => Variant,
        'preserve_keys' => array(Boolean, 'false')));

f('array_search', Variant,
  array('needle' => Variant,
        'haystack' => Variant,
        'strict' => array(Boolean, 'false')));

f('array_shift', Variant,
  array('array' => VariantMap | Reference));

f('array_slice', Variant,
  array('array' => Variant,
        'offset' => Int32,
        'length' => array(Variant, 'null_variant'),
        'preserve_keys' => array(Boolean, 'false')));

f('array_splice', Variant,
  array('input' => VariantMap | Reference,
        'offset' => Int32,
        'length' => array(Variant, 'null_variant'),
        'replacement' => array(Variant, 'null_variant')));

f('array_sum', Variant,
  array('array' => Variant));

f('array_unique', Variant,
  array('array' => Variant));

f('array_unshift', Int32,
  array('array' => VariantMap | Reference,
        'var' => Variant),
  VariableArguments);

f('array_values', Variant,
  array('input' => Variant));

f('array_walk_recursive', Boolean,
  array('input' => VariantMap | Reference,
        'funcname' => Variant,
        'userdata' => array(Variant, 'null_variant')));

f('array_walk', Boolean,
  array('input' => VariantMap | Reference,
        'funcname' => Variant,
        'userdata' => array(Variant, 'null_variant')));

f('compact', VariantMap,
  array('varname' => Variant),
  VariableArguments);

f('shuffle', Boolean,
  array('array' => VariantMap | Reference));

f('count', Int32,
  array('var' => Variant,
        'recursive' => array(Boolean, 'false')));

f('sizeof', Int32,
  array('var' => Variant,
        'recursive' => array(Boolean, 'false')));

f('each', Variant,
  array('array' => VariantMap | Reference));

f('current', Variant,
  array('array' => VariantMap | Reference));

f('next', Variant,
  array('array' => VariantMap | Reference));

f('pos', Variant,
  array('array' => VariantMap | Reference));

f('prev', Variant,
  array('array' => VariantMap | Reference));

f('reset', Variant,
  array('array' => VariantMap | Reference));

f('end', Variant,
  array('array' => VariantMap | Reference));

f('in_array', Boolean,
  array('needle' => Variant,
        'haystack' => Variant,
        'strict' => array(Boolean, 'false')));

f('key', Variant,
  array('array' => VariantMap | Reference));

f('range', Variant,
  array('low' => Variant,
        'high' => Variant,
        'step' => array(Variant, '1')));

///////////////////////////////////////////////////////////////////////////////

f('array_diff', Variant,
  array('array1' => Variant,
        'array2' => Variant),
  VariableArguments);

f('array_udiff', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'data_compare_func' => Variant),
  VariableArguments);

f('array_diff_assoc', Variant,
  array('array1' => Variant,
        'array2' => Variant),
  VariableArguments);

f('array_diff_uassoc', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'key_compare_func' => Variant),
  VariableArguments);

f('array_udiff_assoc', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'data_compare_func' => Variant),
  VariableArguments);

f('array_udiff_uassoc', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'data_compare_func' => Variant,
        'key_compare_func' => Variant),
  VariableArguments);

f('array_diff_key', Variant,
  array('array1' => Variant,
        'array2' => Variant),
  VariableArguments);

f('array_diff_ukey', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'key_compare_func' => Variant),
  VariableArguments);

f('array_intersect', Variant,
  array('array1' => Variant,
        'array2' => Variant),
  VariableArguments);

f('array_uintersect', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'data_compare_func' => Variant),
  VariableArguments);

f('array_intersect_assoc', Variant,
  array('array1' => Variant,
        'array2' => Variant),
  VariableArguments);

f('array_intersect_uassoc', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'key_compare_func' => Variant),
  VariableArguments);

f('array_uintersect_assoc', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'data_compare_func' => Variant),
  VariableArguments);

f('array_uintersect_uassoc', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'data_compare_func' => Variant,
        'key_compare_func' => Variant),
  VariableArguments);

f('array_intersect_key', Variant,
  array('array1' => Variant,
        'array2' => Variant),
  VariableArguments);

f('array_intersect_ukey', Variant,
  array('array1' => Variant,
        'array2' => Variant,
        'key_compare_func' => Variant),
  VariableArguments);

///////////////////////////////////////////////////////////////////////////////

f('sort', Boolean,
  array('array' => VariantMap | Reference,
        'sort_flags' => array(Int32, '0'),
        'use_collator' => array(Boolean, 'false')));

f('rsort', Boolean,
  array('array' => VariantMap | Reference,
        'sort_flags' => array(Int32, '0'),
        'use_collator' => array(Boolean, 'false')));

f('asort', Boolean,
  array('array' => VariantMap | Reference,
        'sort_flags' => array(Int32, '0'),
        'use_collator' => array(Boolean, 'false')));

f('arsort', Boolean,
  array('array' => VariantMap | Reference,
        'sort_flags' => array(Int32, '0'),
        'use_collator' => array(Boolean, 'false')));

f('ksort', Boolean,
  array('array' => VariantMap | Reference,
        'sort_flags' => array(Int32, '0')));

f('krsort', Boolean,
  array('array' => VariantMap | Reference,
        'sort_flags' => array(Int32, '0')));

f('usort', Boolean,
  array('array' => VariantMap | Reference,
        'cmp_function' => Variant));

f('uasort', Boolean,
  array('array' => VariantMap | Reference,
        'cmp_function' => Variant));

f('uksort', Boolean,
  array('array' => VariantMap | Reference,
        'cmp_function' => Variant));

f('natsort', Variant,
  array('array' => VariantMap | Reference));

f('natcasesort', Variant,
  array('array' => VariantMap | Reference));

f('i18n_loc_get_default', String);
f('i18n_loc_set_default', Boolean,
  array('locale' => String));
f('i18n_loc_set_attribute', Boolean,
  array('attr' => Int64, 'val' => Int64));
f('i18n_loc_set_strength', Boolean,
  array('strength' => Int64));
f('i18n_loc_get_error_code', Variant);
