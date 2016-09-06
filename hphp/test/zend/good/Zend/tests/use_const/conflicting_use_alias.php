<?php

namespace {
    const foo = 'foo';
}

namespace x {
    use foo as bar;
    use const foo as bar;
    var_dump(bar);
}

?>
