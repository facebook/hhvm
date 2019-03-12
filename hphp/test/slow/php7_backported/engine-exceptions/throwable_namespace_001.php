<?php

namespace Foo;

<<__EntryPoint>>
function main_throwable_namespace_001() {
$y = 'Throwable';

$x = new \Exception();
\var_dump($x instanceof \Throwable);
\var_dump($x instanceof Throwable);
\var_dump($x instanceof $y);
}
