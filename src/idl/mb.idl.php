<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// multibyte string functions

f('mb_list_encodings', StringVec);

f('mb_list_encodings_alias_names', Variant,
  array('name' => array(String, 'null_string')));

f('mb_list_mime_names', Variant,
  array('name' => array(String, 'null_string')));

f('mb_check_encoding', Boolean,
  array('var' => array(String, 'null_string'),
        'encoding' => array(String, 'null_string')));

f('mb_convert_case', Variant,
  array('str' => String,
        'mode' => Int32,
        'encoding' => array(String, 'null_string')));

f('mb_convert_encoding', Variant,
  array('str' => String,
        'to_encoding' => String,
        'from_encoding' => array(Variant, 'null_variant')));

f('mb_convert_kana', Variant,
  array('str' => String,
        'option' => array(String, 'null_string'),
        'encoding' => array(String, 'null_string')));

f('mb_convert_variables', Variant,
  array('to_encoding' => String,
        'from_encoding' => Variant,
        'vars' => Variant | Reference),
  VariableArguments);

f('mb_decode_mimeheader', Variant,
  array('str' => String));

f('mb_decode_numericentity', Variant,
  array('str' => String,
        'convmap' => Variant,
        'encoding' => array(String, 'null_string')));

f('mb_detect_encoding', Variant,
  array('str' => String,
        'encoding_list' => array(Variant, 'null_variant'),
        'strict' => array(Variant, 'null_variant')));

f('mb_detect_order', Variant,
  array('encoding_list' => array(Variant, 'null_variant')));

f('mb_encode_mimeheader', Variant,
  array('str' => String,
        'charset' => array(String, 'null_string'),
        'transfer_encoding' => array(String, 'null_string'),
        'linefeed' => array(String, '"\r\n"'),
        'indent' => array(Int32, '0')));

f('mb_encode_numericentity', Variant,
  array('str' => String,
        'convmap' => Variant,
        'encoding' => array(String, 'null_string')));

f('mb_ereg_match', Boolean,
  array('pattern' => String,
        'str' => String,
        'option' => array(String, 'null_string')));

f('mb_ereg_replace', Variant,
  array('pattern' => Variant,
        'replacement' => String,
        'str' => String,
        'option' => array(String, 'null_string')));

f('mb_ereg_search_getpos', Int32);

f('mb_ereg_search_getregs', Variant);

f('mb_ereg_search_init', Boolean,
  array('str' => String,
        'pattern' => array(String, 'null_string'),
        'option' => array(String, 'null_string')));

f('mb_ereg_search_pos', Variant,
  array('pattern' => array(String, 'null_string'),
        'option' => array(String, 'null_string')));

f('mb_ereg_search_regs', Variant,
  array('pattern' => array(String, 'null_string'),
        'option' => array(String, 'null_string')));

f('mb_ereg_search_setpos', Boolean,
  array('position' => Int32));

f('mb_ereg_search', Variant,
  array('pattern' => array(String, 'null_string'),
        'option' => array(String, 'null_string')));

f('mb_ereg', Variant,
  array('pattern' => Variant,
        'str' => String,
        'regs' => array(Variant | Reference, 'null')));

f('mb_eregi_replace', Variant,
  array('pattern' => Variant,
        'replacement' => String,
        'str' => String,
        'option' => array(String, 'null_string')));

f('mb_eregi', Variant,
  array('pattern' => Variant,
        'str' => String,
        'regs' => array(Variant | Reference, 'null')));

f('mb_get_info', Variant,
  array('type' => array(String, 'null_string')));

f('mb_http_input', Variant,
  array('type' => array(String, 'null_string')));

f('mb_http_output', Variant,
  array('encoding' => array(String, 'null_string')));

f('mb_internal_encoding', Variant,
  array('encoding' => array(String, 'null_string')));

f('mb_language', Variant,
  array('language' => array(String, 'null_string')));

f('mb_output_handler', String,
  array('contents' => String,
        'status' => Int32));

f('mb_parse_str', Boolean,
  array('encoded_string' => String,
        'result' => array(StringVec | Reference, 'null')));

f('mb_preferred_mime_name', Variant,
  array('encoding' => String));

f('mb_regex_encoding', Variant,
  array('encoding' => array(String, 'null_string')));

f('mb_regex_set_options', String,
  array('options' => array(String, 'null_string')));

f('mb_send_mail', Boolean,
  array('to' => String,
        'subject' => String,
        'message' => String,
        'headers' => array(String, 'null_string'),
        'extra_cmd' => array(String, 'null_string')));

f('mb_split', Variant,
  array('pattern' => String,
        'str' => String,
        'count' => array(Int32, '-1')));

f('mb_strcut', Variant,
  array('str' => String,
        'start' => Int32,
        'length' => array(Int32, '0x7FFFFFFF'),
        'encoding' => array(String, 'null_string')));

f('mb_strimwidth', Variant,
  array('str' => String,
        'start' => Int32,
        'width' => Int32,
        'trimmarker' => array(String, 'null_string'),
        'encoding' => array(String, 'null_string')));

f('mb_stripos', Variant,
  array('haystack' => String,
        'needle' => String,
        'offset' => array(Int32, '0'),
        'encoding' => array(String, 'null_string')));

f('mb_stristr', Variant,
  array('haystack' => String,
        'needle' => String,
        'part' => array(Boolean, 'false'),
        'encoding' => array(String, 'null_string')));

f('mb_strlen', Variant,
  array('str' => String,
        'encoding' => array(String, 'null_string')));

f('mb_strpos', Variant,
  array('haystack' => String,
        'needle' => String,
        'offset' => array(Int32, '0'),
        'encoding' => array(String, 'null_string')));

f('mb_strrchr', Variant,
  array('haystack' => String,
        'needle' => String,
        'part' => array(Boolean, 'false'),
        'encoding' => array(String, 'null_string')));

f('mb_strrichr', Variant,
  array('haystack' => String,
        'needle' => String,
        'part' => array(Boolean, 'false'),
        'encoding' => array(String, 'null_string')));

f('mb_strripos', Variant,
  array('haystack' => String,
        'needle' => String,
        'offset' => array(Int32, '0'),
        'encoding' => array(String, 'null_string')));

f('mb_strrpos', Variant,
  array('haystack' => String,
        'needle' => String,
        'offset' => array(Variant, '0LL'),
        'encoding' => array(String, 'null_string')));

f('mb_strstr', Variant,
  array('haystack' => String,
        'needle' => String,
        'part' => array(Boolean, 'false'),
        'encoding' => array(String, 'null_string')));

f('mb_strtolower', Variant,
  array('str' => String,
        'encoding' => array(String, 'null_string')));

f('mb_strtoupper', Variant,
  array('str' => String,
        'encoding' => array(String, 'null_string')));

f('mb_strwidth', Variant,
  array('str' => String,
        'encoding' => array(String, 'null_string')));

f('mb_substitute_character', Variant,
  array('substrchar' => array(Variant, 'null_variant')));

f('mb_substr_count', Variant,
  array('haystack' => String,
        'needle' => String,
        'encoding' => array(String, 'null_string')));

f('mb_substr', Variant,
  array('str' => String,
        'start' => Int32,
        'length' => array(Int32, '0x7FFFFFFF'),
        'encoding' => array(String, 'null_string')));
