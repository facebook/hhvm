<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('get_defined_functions', VariantMap);

f('function_exists', Boolean,
  array('function_name' => String));

f('is_callable', Boolean,
  array('v' => Any,
        'syntax' => array(Boolean, 'false'),
        'name' => array(String | Reference, 'null')),
  DefaultFlags);

f('call_user_func_array', Variant,
  array('function' => Variant,
        'params' => VariantVec), DefaultFlags);

f('call_user_func', Variant,
  array('function' => Variant),
  MixedVariableArguments, 'hphp_opt_call_user_func');

f('call_user_func_array_async', Object,
  array('function' => Variant,
        'params' => VariantVec));

f('call_user_func_async', Object,
  array('function' => Variant),
  MixedVariableArguments);

f('end_user_func_async', Variant,
  array('handle' => Object,
        'default_strategy' => array(Int32, 'k_GLOBAL_STATE_IGNORE'),
        'additional_strategies' => array(Variant, 'null')));

f('forward_static_call_array', Variant,
  array('function' => Variant,
        'params' => VariantVec));

f('forward_static_call', Variant,
  array('function' => Variant),
  MixedVariableArguments);

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
