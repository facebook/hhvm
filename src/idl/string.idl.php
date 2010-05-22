<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// transformations and manipulations

f('addcslashes',   String, array('str' => String, 'charlist' => String));
f('stripcslashes', String, array('str' => String));
f('addslashes',    String, array('str' => String));
f('stripslashes',  String, array('str' => String));
f('bin2hex',       String, array('str' => String));
f('nl2br',         String, array('str' => String));
f('quotemeta',     String, array('str' => String));
f('str_shuffle',   String, array('str' => String));
f('strrev',        String, array('str' => String));
f('strtolower',    String, array('str' => String));
f('strtoupper',    String, array('str' => String));
f('ucfirst',       String, array('str' => String));
f('ucwords',       String, array('str' => String));

f('strip_tags', String,
  array('str' => String,
        'allowable_tags' => array(String, '""')));

f('trim', String,
  array('str' => String,
        'charlist' => array(String, 'k_HPHP_TRIM_CHARLIST')));

f('ltrim', String,
  array('str' => String,
        'charlist' => array(String, 'k_HPHP_TRIM_CHARLIST')));

f('rtrim', String,
  array('str' => String,
        'charlist' => array(String, 'k_HPHP_TRIM_CHARLIST')));

f('chop', String,
  array('str' => String,
        'charlist' => array(String, 'k_HPHP_TRIM_CHARLIST')));

f('explode', Variant,
  array('delimiter' => String,
        'str' => String,
        'limit' => array(Int32, '0x7FFFFFFF')));

f('implode', String,
  array('arg1' => Variant,
        'arg2' => array(Variant, 'null_variant')));

f('join', String,
  array('glue' => Variant,
        'pieces' => array(Variant, 'null_variant')));

f('str_split', Variant,
  array('str' => String,
        'split_length' => array(Int32, '1')));

f('chunk_split', Variant,
  array('body' => String,
        'chunklen' => array(Int32, '76'),
        'end' => array(String, '"\r\n"')));

f('strtok', Variant,
  array('str' => String,
        'token' => array(Variant, 'null_variant')));

f('str_replace', Variant,
  array('search' => Variant,
        'replace' => Variant,
        'subject' => Variant,
        'count' => array(Int32 | Reference, 'null')));

f('str_ireplace', Variant,
  array('search' => Variant,
        'replace' => Variant,
        'subject' => Variant,
        'count' => array(Int32 | Reference, 'null')));

f('substr_replace', Variant,
  array('str' => Variant,
        'replacement' => Variant,
        'start' => Variant,
        'length' => array(Variant, '0x7FFFFFFF')));

f('substr', Variant,
  array('str' => String,
        'start' => Int32,
        'length' => array(Int32, '0x7FFFFFFF')));

f('str_pad', String,
  array('input' => String,
        'pad_length' => Int32,
        'pad_string' => array(String, '" "'),
        'pad_type' => array(Int32, 'k_STR_PAD_RIGHT')));

f('str_repeat', String,
  array('input' => String,
        'multiplier' => Int32));

f('wordwrap', Variant,
  array('str' => String,
        'width' => array(Int32, '75'),
        'wordbreak' => array(String, '"\n"'),
        'cut' => array(Boolean, 'false')));

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

f('html_entity_decode', String,
  array('str' => String,
        'quote_style' => array(Int32, 'k_ENT_COMPAT'),
        'charset' => array(String, '""')));

f('htmlentities', String,
  array('str' => String,
        'quote_style' => array(Int32, 'k_ENT_COMPAT'),
        'charset' => array(String, '""'),
        'double_encode' => array(Boolean, 'true')));

f('htmlspecialchars_decode', String,
  array('str' => String,
        'quote_style' => array(Int32, 'k_ENT_COMPAT')));

f('htmlspecialchars', String,
  array('str' => String,
        'quote_style' => array(Int32, 'k_ENT_COMPAT'),
        'charset' => array(String, '""'),
        'double_encode' => array(Boolean, 'true')));

f('quoted_printable_encode', String,
  array('str' => String));

f('quoted_printable_decode', String,
  array('str' => String));

f('convert_uudecode', Variant,
  array('data' => String));

f('convert_uuencode', Variant,
  array('data' => String));

f('str_rot13', String,
  array('str' => String));

f('crc32', Int64,
  array('str' => String));

f('crypt', String,
  array('str' => String,
        'salt' => array(String, '""')));

f('md5', String,
  array('str' => String,
        'raw_output' => array(Boolean, 'false')));

f('sha1', String,
  array('str' => String,
        'raw_output' => array(Boolean, 'false')));

f('strtr', Variant,
  array('str' => String,
        'from' => Variant,
        'to' => array(Variant, 'null_variant')));

f('convert_cyr_string', String,
  array('str' => String,
        'from' => String,
        'to' => String));

f('get_html_translation_table', StringMap,
  array('table' => array(Int32, '0'),
        'quote_style' => array(Int32, 'k_ENT_COMPAT')));

f('hebrev', String,
  array('hebrew_text' => String,
        'max_chars_per_line' => array(Int32, '0')));

f('hebrevc', String,
  array('hebrew_text' => String,
        'max_chars_per_line' => array(Int32, '0')));

f('setlocale', Variant,
  array('category' => Int32,
        'locale' => Variant),
  VariableArguments);

f('localeconv', VariantMap);

f('nl_langinfo', String,
  array('item' => Int32));

///////////////////////////////////////////////////////////////////////////////
// input/output

f('echo', NULL,
  array('arg' => String), VariableArguments);

f('print', Int32,
  array('arg' => String));

f('printf', Variant,
  array('format' => String), VariableArguments);

f('vprintf', Variant,
  array('format' => String,
        'args' => VariantVec));

f('sprintf', Variant,
  array('format' => String), VariableArguments);

f('vsprintf', Variant,
  array('format' => String,
        'args' => VariantVec));

f('sscanf', Variant,
  array('str' => String,
        'format' => String),
  ReferenceVariableArguments);

f('chr', String, array('ascii' => Int64));
f('ord', Int64,   array('str' => String));

f('money_format', Variant,
  array('format' => String,
        'number' => Double));

f('number_format', String,
  array('number' => Double,
        'decimals' => array(Int32, '0'),
        'dec_point' => array(String, '"."'),
        'thousands_sep' => array(String, '","')));

///////////////////////////////////////////////////////////////////////////////
// analysis

f('strcmp', Int32,
  array('str1' => String,
        'str2' => String));

f('strncmp', Int32,
  array('str1' => String,
        'str2' => String,
        'len' => Int32));

f('strnatcmp', Int32,
  array('str1' => String,
        'str2' => String));

f('strcasecmp', Int32,
  array('str1' => String,
        'str2' => String));

f('strncasecmp', Int32,
  array('str1' => String,
        'str2' => String,
        'len' => Int32));

f('strnatcasecmp', Int32,
  array('str1' => String,
        'str2' => String));

f('strcoll', Int32,
  array('str1' => String,
        'str2' => String));

f('substr_compare', Variant,
  array('main_str' => String,
        'str' => String,
        'offset' => Int32,
        'length' => array(Int32, '0'),
        'case_insensitivity' => array(Boolean, 'false')));

f('strchr', Variant,
  array('haystack' => String,
        'needle' => Variant));

f('strrchr', Variant,
  array('haystack' => String,
        'needle' => Variant));

f('strstr', Variant,
  array('haystack' => String,
        'needle' => Variant));

f('stristr', Variant,
  array('haystack' => String,
        'needle' => Variant));

f('strpbrk', Variant,
  array('haystack' => String,
        'char_list' => String));

f('strpos', Variant,
  array('haystack' => String,
        'needle' => Variant,
        'offset' => array(Int32, '0')));

f('stripos', Variant,
  array('haystack' => String,
        'needle' => Variant,
        'offset' => array(Int32, '0')));

f('strrpos', Variant,
  array('haystack' => String,
        'needle' => Variant,
        'offset' => array(Int32, '-1')));

f('strripos', Variant,
  array('haystack' => String,
        'needle' => Variant,
        'offset' => array(Int32, '-1')));

f('substr_count', Variant,
  array('haystack' => String,
        'needle' => String,
        'offset' => array(Int32, '0'),
        'length' => array(Int32, '0x7FFFFFFF')));

f('strspn', Variant,
  array('str1' => String,
        'str2' => String,
        'start' => array(Int32, '0'),
        'length' => array(Int32, '0x7FFFFFFF')));

f('strcspn', Variant,
  array('str1' => String,
        'str2' => String,
        'start' => array(Int32, '0'),
        'length' => array(Int32, '0x7FFFFFFF')));

f('strlen', Int32,
  array('str' => String));

f('count_chars', Variant,
  array('str' => String,
        'mode' => array(Int64, '0')));

f('str_word_count', Variant,
  array('str' => String,
        'format' => array(Int64, '0'),
        'charlist' => array(String, '""')));

f('levenshtein', Int32,
  array('str1' => String,
        'str2' => String,
        'cost_ins' => array(Int32, '1'),
        'cost_rep' => array(Int32, '1'),
        'cost_del' => array(Int32, '1')));

f('similar_text', Int32,
  array('first' => String,
        'second' => String,
        'percent' => array(Double | Reference, 'null')));

f('soundex', Variant,
  array('str' => String));

f('metaphone', Variant,
  array('str' => String,
        'phones' => array(Int32, '0')));

///////////////////////////////////////////////////////////////////////////////
// special

f('parse_str', NULL,
  array('str' => String,
        'arr' => array(VariantMap | Reference, 'null')));

