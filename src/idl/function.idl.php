<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('get_defined_functions', VariantMap);

f('function_exists', Boolean,
  array('function_name' => String));

f('is_callable', Boolean,
  array('v' => Any,
        'syntax' => array(Boolean, 'false'),
        'name' => array(String | Reference, 'null')));

f('call_user_func_array', Variant,
  array('function' => Variant,
        'params' => VariantVec));

f('call_user_func', Variant,
  array('function' => Variant),
  VariableArguments);

f('create_function', String,
  array('args' => String,
        'code' => String));

///////////////////////////////////////////////////////////////////////////////

f('func_get_arg', Variant,
  array('arg_num' => Int32));

f('func_get_args', VariantVec);

f('func_num_args', Int32);

///////////////////////////////////////////////////////////////////////////////

f('register_postsend_function', NULL,
  array('function' => Variant),
  VariableArguments);

f('register_shutdown_function', NULL,
  array('function' => Variant),
  VariableArguments);

f('register_cleanup_function', NULL,
  array('function' => Variant),
  VariableArguments);

f('register_tick_function', Boolean,
  array('function' => Variant),
  VariableArguments);

f('unregister_tick_function', NULL,
  array('function_name' => Variant));
