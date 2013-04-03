<?php

namespace closure;

class closure { static $x = 1;}

$x = __NAMESPACE__;
var_dump(closure::$x);

var_dump($x::$x);

?>