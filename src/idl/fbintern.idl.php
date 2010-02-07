<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// fb extension

f('fb_set_opcode', NULL,
  array('opcode' => Int32,
        'callback' => String));

f('fb_reset_opcode', NULL,
  array('opcode' => Int32));

f('fb_config_coredump', NULL,
  array('enabled' => Boolean,
        'limit' => Int32));

f('fb_debug_rlog', Boolean,
  array('err' => Variant,
        'include_flags' => array(Int32, '-1'),
        'type' => array(Int32, '-1'),
        'headers' => array(Variant, 'null_variant')));

f('fb_backtrace', String,
  array('exception' => array(Variant, 'null_variant')));

f('fb_render_wrapped', String,
  array('text' => String,
        'linelen' => array(Int32, '26'),
        'head' => array(String, '"<span>"'),
        'tail' => array(String, '"</span><wbr></wbr><span class=\"word_break\"></span>"')));

f('fb_add_included_file', NULL,
  array('filepath' => String));

f('fb_request_timers', Variant);

f('fb_get_ape_version', String);

f('fb_get_derived_classes', StringVec,
  array('filename' => String,
        'basecls' => String));

///////////////////////////////////////////////////////////////////////////////
// hotprofiler

f('hotprofiler_enable',  NULL,
  array('level' => Int32));

f('hotprofiler_disable', Variant);

f('phprof_enable',  NULL,
  array('flags' => array(Int32, '0')));

f('phprof_disable', Variant);

///////////////////////////////////////////////////////////////////////////////
// FQL parser

f('fql_set_static_data_10', NULL,
  array('tables' => StringVec,
        'functions' => VariantMap));

f('fql_static_data_set_10', Boolean);

f('fql_parse_10', VariantMap,
  array('query' => String));

f('fql_multiparse_10', VariantMap,
  array('query' => StringMap));

///////////////////////////////////////////////////////////////////////////////
// xhp functions

f('xhp_preprocess_code', VariantMap,
  array('code' => String));
