<?php

namespace foo {
    const baz = 42;
}

namespace bar {
    const baz = 42;
}

namespace {
    use const foo\baz, bar\baz;
    echo "Done\n";
}

?>
