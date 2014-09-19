<?php

namespace foo {
    function baz() {
        return 'foo.baz';
    }
}

namespace bar {
    function baz() {
        return 'bar.baz';
    }
}

namespace {
    use function foo\baz, bar\baz;
    echo "Done\n";
}

?>
