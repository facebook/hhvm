<?php

class X { static $y = 2; }
function k() { var_dump(is_int(X::$y)); }

<<__EntryPoint>>
function main_public_static_props_011() {
$z = 'asd';
X::$y =& $z;
k();
}
