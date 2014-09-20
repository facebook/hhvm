<?php

namespace {
    const foo = 'foo.const';
    function foo() {
        return 'foo.function';
    }
}

namespace x {
    use const foo as bar;
    use function foo as bar;
    var_dump(bar);
    var_dump(bar());
}

?>
