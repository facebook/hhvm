<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// type testing

f('is_bool',     Boolean, array('var' => Any), NoEffect);
f('is_int',      Boolean, array('var' => Any), NoEffect);
f('is_integer',  Boolean, array('var' => Any), NoEffect);
f('is_long',     Boolean, array('var' => Any), NoEffect);
f('is_double',   Boolean, array('var' => Any), NoEffect);
f('is_float',    Boolean, array('var' => Any), NoEffect);
f('is_numeric',  Boolean, array('var' => Any), NoEffect);
f('is_real',     Boolean, array('var' => Any), NoEffect);
f('is_string',   Boolean, array('var' => Any), NoEffect);
f('is_scalar',   Boolean, array('var' => Any), NoEffect);
f('is_array',    Boolean, array('var' => Any), NoEffect);
f('is_object',   Boolean, array('var' => Any), NoEffect);
f('is_resource', Boolean, array('var' => Any), NoEffect);
f('is_null',     Boolean, array('var' => Any), NoEffect);

f('gettype', String, array('v' => Any));
f('get_resource_type', String, array('handle' => Resource));

///////////////////////////////////////////////////////////////////////////////
// type conversion

f('intval',    Int64,  array('v' => Any, 'base' => array(Int64, '10')));
f('doubleval', Double, array('v' => Any));
f('floatval',  Double, array('v' => Any));
f('strval',    String, array('v' => Any));

f('settype', Boolean,
  array('var' => Variant | Reference,
        'type' => String));

///////////////////////////////////////////////////////////////////////////////
// input/output

f('print_r', Variant,
  array('expression' => Any,
        'ret' => array(Boolean, 'false')));
f('var_export', Variant,
  array('expression' => Any,
        'ret' => array(Boolean, 'false')));
f('var_dump', NULL, array('expression' => Any), VariableArguments);
f('debug_zval_dump', NULL, array('variable' => Any));

f('serialize',   String,  array('value' => Any));
f('unserialize', Variant, array('str' => String));

///////////////////////////////////////////////////////////////////////////////
// variable table

f('get_defined_vars', VariantMap);

f('import_request_variables', Boolean,
  array('types' => String,
        'prefix' => array(String, '""')));

f('extract', Int32,
  array('var_array' => VariantMap,
        'extract_type' => array(Int32, 'EXTR_OVERWRITE'),
        'prefix' => array(String, '""')));
