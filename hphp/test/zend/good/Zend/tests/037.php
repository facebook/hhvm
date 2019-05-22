<?php

namespace closure;

class closure { static $x = 1;}
<<__EntryPoint>> function main() {
$x = __NAMESPACE__;
\var_dump(closure::$x);

\var_dump($x::$x);
}
