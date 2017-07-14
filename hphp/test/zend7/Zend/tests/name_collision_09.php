<?php

namespace Foo {
    const BAR = 42;
}

namespace Bazzle {
    use const Foo\BAR;
    const BAR = 24;
}
