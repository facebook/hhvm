<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('session_set_cookie_params', NULL,
  array('lifetime' => Int64,
        'path' => array(String, 'null_string'),
        'domain' => array(String, 'null_string'),
        'secure' => array(Variant, 'null'),
        'httponly' => array(Variant, 'null')));

f('session_get_cookie_params', StringMap);

f('session_name', String,
  array('newname' => array(String, 'null_string')));

f('session_module_name', Variant,
  array('newname' => array(String, 'null_string')));

f('session_set_save_handler', Boolean,
  array('open' => String,
        'close' => String,
        'read' => String,
        'write' => String,
        'destroy' => String,
        'gc' => String));

f('session_save_path', String,
  array('newname' => array(String, 'null_string')));

f('session_id', String,
  array('newid' => array(String, 'null_string')));

f('session_regenerate_id', Boolean,
  array('delete_old_session' => array(Boolean, 'false')));

f('session_cache_limiter', String,
  array('new_cache_limiter' => array(String, 'null_string')));

f('session_cache_expire', Int64,
  array('new_cache_expire' => array(String, 'null_string')));

f('session_encode', Variant);

f('session_decode', Boolean,
  array('data' => String));

f('session_start', Boolean);

f('session_destroy', Boolean);

f('session_unset', Variant);

f('session_commit');

f('session_write_close');

f('session_register', Boolean,
  array('var_names' => Variant),
  VariableArguments);

f('session_unregister', Boolean,
  array('varname' => String));

f('session_is_registered', Boolean,
  array('varname' => String));
