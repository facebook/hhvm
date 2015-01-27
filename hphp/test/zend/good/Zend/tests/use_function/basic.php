<?php

namespace foo\bar {
    function baz() {
        return 'foo.bar.baz';
    }
    function qux() {
        return baz();
    }
}

namespace {
    use function foo\bar\baz, foo\bar\qux;
    var_dump(baz());
    var_dump(qux());
    echo "Done\n";
}

?>
