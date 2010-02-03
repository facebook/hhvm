<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('assert_options', Variant,
  array('what' => Int32,
        'value' => array(Variant, 'null_variant')));

f('assert', Variant,
  array('assertion' => Variant));

f('dl', Int32,
  array('library' => String));

f('extension_loaded', Boolean,
  array('name' => String));

f('get_loaded_extensions', StringVec,
  array('zend_extensions' => array(Boolean, 'false')));

f('get_extension_funcs', VariantMap,
  array('module_name' => String));

f('get_cfg_var', String,
  array('option' => String));

f('get_current_user', String);

f('get_defined_constants', VariantMap,
  array('categorize' => array(Variant, 'null_variant')));

f('get_include_path', String);

f('restore_include_path');

f('set_include_path', String,
  array('new_include_path' => String));

f('get_included_files', StringVec);

f('get_magic_quotes_gpc', Int32);

f('get_magic_quotes_runtime', Int32);

f('get_required_files', StringVec);

f('getenv', Variant,
  array('varname' => String));

f('getlastmod', Int32);

f('getmygid', Int32);

f('getmyinode', Int32);

f('getmypid', Int32);

f('getmyuid', Int32);

f('getopt', VariantMap,
  array('options' => String,
        'longopts' => array(Variant, 'null_variant')));

f('getrusage', VariantMap,
  array('who' => array(Int32, '0')));

f('clock_getres', Boolean,
  array('clk_id' => Int32,
        'sec' => Int64 | Reference,
        'nsec' => Int64 | Reference));

f('clock_gettime', Boolean,
  array('clk_id' => Int32,
        'sec' => Int64 | Reference,
        'nsec' => Int64 | Reference));

f('clock_settime', Boolean,
  array('clk_id' => Int32,
        'sec' => Int64,
        'nsec' => Int64));

f('ini_alter', String,
  array('varname' => String,
        'newvalue' => String));

f('ini_get_all', VariantMap,
  array('extension' => array(String, 'null_string')));

f('ini_get', String,
  array('varname' => String));

f('ini_restore', NULL,
  array('varname' => String));

f('ini_set', String,
  array('varname' => String,
        'newvalue' => String));

f('memory_get_peak_usage', Int64,
  array('real_usage' => array(Boolean, 'false')));

f('memory_get_usage', Int64,
  array('real_usage' => array(Boolean, 'false')));

f('php_ini_scanned_files', String);

f('php_logo_guid', String);

f('php_sapi_name', String);

f('php_uname', String,
  array('mode' => array(String, 'null_string')));

f('phpcredits', Boolean,
  array('flag' => array(Int32, '0')));

f('phpinfo', Boolean,
  array('what' => array(Int32, '0')));

f('phpversion', String,
  array('extension' => array(String, 'null_string')));

f('putenv', Boolean,
  array('setting' => String));

f('set_magic_quotes_runtime', Boolean,
  array('new_setting' => Boolean));

f('set_time_limit', NULL,
  array('seconds' => Int32));

f('sys_get_temp_dir', String);

f('version_compare', Variant,
  array('version1' => String,
        'version2' => String,
        'sop' => array(String, 'null_string')));

f('zend_logo_guid', String);

f('zend_thread_id', Int32);

f('zend_version', String);
