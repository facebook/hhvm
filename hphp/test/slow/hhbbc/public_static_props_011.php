<?php

class X { static $y = 2; }
$z = 'asd';
X::$y =& $z;
function k() { var_dump(is_int(X::$y)); }
k();
