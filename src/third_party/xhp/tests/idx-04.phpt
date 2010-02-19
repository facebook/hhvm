--TEST--
XHP idx Expression 04
--FILE--
<?php
$string = 'foobar';

@var_dump(__xhp_idx($string, 0));
@var_dump(__xhp_idx($string, '0'));
@var_dump(__xhp_idx($string, null));
@var_dump(__xhp_idx($string, true));
@var_dump(__xhp_idx($string, false));
@var_dump(__xhp_idx($string, 'honk'));
--EXPECT--
string(1) "f"
string(1) "f"
string(1) "f"
string(1) "o"
string(1) "f"
string(1) "f"
