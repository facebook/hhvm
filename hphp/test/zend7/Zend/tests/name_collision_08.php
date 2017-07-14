<?php

namespace Foo {
    function bar() {}
}

namespace Bazzle {
    use function Foo\bar;
    function bar() {}
}
