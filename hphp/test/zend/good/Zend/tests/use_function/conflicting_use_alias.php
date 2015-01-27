<?php

namespace {
    function foo() {
        return 'foo';
    }
}

namespace x {
    use foo as bar;
    use function foo as bar;
    var_dump(bar());
}

?>
