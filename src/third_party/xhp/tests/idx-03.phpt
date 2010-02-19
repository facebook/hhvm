--TEST--
XHP idx Expression 03
--FILE--
<?php
$array = array();
$array[0] = 'zero';
$array['key'] = 'value';

@var_dump(__xhp_idx($array, 0));
@var_dump(__xhp_idx($array, '0'));
@var_dump(__xhp_idx($array, NULL));
@var_dump(__xhp_idx($array, true));
@var_dump(__xhp_idx($array, false));
@var_dump(__xhp_idx($array, 'key'));
@var_dump(__xhp_idx($array, 'other_key'));
--EXPECT--
string(4) "zero"
string(4) "zero"
NULL
NULL
string(4) "zero"
string(5) "value"
NULL
