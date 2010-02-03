<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// preg

f('preg_grep', Variant,
  array('pattern' => String,
        'input' => VariantMap,
        'flags' => array(Int32, '0')));

f('preg_match', Variant,
  array('pattern' => String,
        'subject' => String,
        'matches' => array(StringVec | Reference, 'null'),
        'flags' => array(Int32, '0'),
        'offset' => array(Int32, '0')));

f('preg_match_all', Variant,
  array('pattern' => String,
        'subject' => String,
        'matches' => StringVec | Reference,
        'flags' => array(Int32, '0'),
        'offset' => array(Int32, '0')));

f('preg_replace', Variant,
  array('pattern' => Variant,
        'replacement' => Variant,
        'subject' => Variant,
        'limit' => array(Int32, '-1'),
        'count' => array(Int32 | Reference, 'null')));

f('preg_replace_callback', Variant,
  array('pattern' => Variant,
        'callback' => Variant,
        'subject' => Variant,
        'limit' => array(Int32, '-1'),
        'count' => array(Int32 | Reference, 'null')));

f('preg_split', Variant,
  array('pattern' => Variant,
        'subject' => Variant,
        'limit' => array(Int32, '-1'),
        'flags' => array(Int32, '0')));

f('preg_quote', String,
  array('str' => String,
        'delimiter' => array(String, 'null_string')));

f('preg_last_error', Int32);

///////////////////////////////////////////////////////////////////////////////
// ereg

f('ereg_replace', String,
  array('pattern' => String,
        'replacement' => String,
        'str' => String));

f('eregi_replace', String,
  array('pattern' => String,
        'replacement' => String,
        'str' => String));

f('ereg', Variant,
  array('pattern' => String,
        'str' => String,
        'regs' => array(StringVec | Reference, 'null')));

f('eregi', Variant,
  array('pattern' => String,
        'str' => String,
        'regs' => array(StringVec | Reference, 'null')));

f('split', Variant,
  array('pattern' => String,
        'str' => String,
        'limit' => array(Int32, '-1')));

f('spliti', Variant,
  array('pattern' => String,
        'str' => String,
        'limit' => array(Int32, '-1')));

f('sql_regcase', String,
  array('str' => String));
