<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// arbitrary precision mathematics

f('bcscale', Boolean,
  array('scale' => Int64));

f('bcadd', String,
  array('left' => String,
        'right' => String,
        'scale' => array(Int64, '-1')));

f('bcsub', String,
  array('left' => String,
        'right' => String,
        'scale' => array(Int64, '-1')));

f('bccomp', Int64,
  array('left' => String,
        'right' => String,
        'scale' => array(Int64, '-1')));

f('bcmul', String,
  array('left' => String,
        'right' => String,
        'scale' => array(Int64, '-1')));

f('bcdiv', String,
  array('left' => String,
        'right' => String,
        'scale' => array(Int64, '-1')));

f('bcmod', String,
  array('left' => String,
        'right' => String));

f('bcpow', String,
  array('left' => String,
        'right' => String,
        'scale' => array(Int64, '-1')));

f('bcpowmod', Variant,
  array('left' => String,
        'right' => String,
        'modulus' => String,
        'scale' => array(Int64, '-1')));

f('bcsqrt', Variant,
  array('operand' => String,
        'scale' => array(Int64, '-1')));
