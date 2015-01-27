<?php

namespace foo {
    const bar = 42;
}

namespace {
    const bar = 43;
}

namespace {
    use function foo\bar;
    var_dump(bar);
    echo "Done\n";
}

?>
