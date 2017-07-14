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
    use function foo\baz as foo_baz,
                 bar\baz as bar_baz;
    var_dump(foo_baz());
    var_dump(bar_baz());
    echo "Done\n";
}

?>
