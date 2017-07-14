<?php

namespace Foo;
function test(?\stdClass $param) {}
test(new \stdClass);

?>
===DONE===
