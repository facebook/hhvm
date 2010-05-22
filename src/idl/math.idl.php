<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// mathematical operations

f('pi',           Double);
f('min',          Variant,     array('value' => Variant), VariableArguments);
f('max',          Variant,     array('value' => Variant), VariableArguments);
f('abs',          Variant, array('number' => Variant));
f('is_finite',    Boolean, array('val' => Double));
f('is_infinite',  Boolean, array('val' => Double));
f('is_nan',       Boolean, array('val' => Double));

f('ceil',    Double,  array('value' => Double));
f('floor',   Double,  array('value' => Double));
f('round',   Double,  array('val' => Variant,
                            'precision' => array(Int64, '0')));
f('deg2rad', Double,  array('number' => Double));
f('rad2deg', Double,  array('number' => Double));
f('decbin',  String,  array('number' => Int64));
f('dechex',  String,  array('number' => Int64));
f('decoct',  String,  array('number' => Int64));
f('bindec',  Variant,   array('binary_string' => String));
f('hexdec',  Variant,   array('hex_string'    => String));
f('octdec',  Variant,   array('octal_string'  => String));

f('base_convert', Variant,
  array('number' => String,
        'frombase' => Int64,
        'tobase' => Int64));

f('pow',     Numeric, array('base' => Variant, 'exp' => Variant));
f('exp',     Double,  array('arg' => Double));
f('expm1',   Double,  array('arg' => Double));
f('log10',   Double,  array('arg' => Double));
f('log1p',   Double,  array('number' => Double));
f('log',     Double,  array('arg' => Double, 'base' => array(Double, '0')));

f('cos',     Double,  array('arg' => Double));
f('cosh',    Double,  array('arg' => Double));
f('sin',     Double,  array('arg' => Double));
f('sinh',    Double,  array('arg' => Double));
f('tan',     Double,  array('arg' => Double));
f('tanh',    Double,  array('arg' => Double));
f('acos',    Double,  array('arg' => Double));
f('acosh',   Double,  array('arg' => Double));
f('asin',    Double,  array('arg' => Double));
f('asinh',   Double,  array('arg' => Double));
f('atan',    Double,  array('arg' => Double));
f('atanh',   Double,  array('arg' => Double));
f('atan2',   Double,  array('y'   => Double, 'x' => Double));
f('hypot',   Double,  array('x'   => Double, 'y' => Double));
f('fmod',    Double,  array('x'   => Double, 'y' => Double));
f('sqrt',    Double,  array('arg' => Double));

///////////////////////////////////////////////////////////////////////////////
// randomization

f('getrandmax', Int64);

f('srand', Null,
  array('seed' => array(Variant, 'null_variant')));

f('rand', Int64,
  array('min' => array(Int64, '0'),
        'max' => array(Int64, 'RAND_MAX')));

f('mt_getrandmax', Int64);

f('mt_srand', Null,
  array('seed' => array(Variant, 'null_variant')));

f('mt_rand', Int64,
  array('min' => array(Int64, '0'),
        'max' => array(Int64, 'RAND_MAX')));

f('lcg_value', Double);
