<?php

namespace Foo;
class Bar {}

$bar = "Bar";
assert(new \stdClass instanceof $bar);
assert(new \stdClass instanceof Bar);
assert(new \stdClass instanceof \Foo\Bar);
?>
