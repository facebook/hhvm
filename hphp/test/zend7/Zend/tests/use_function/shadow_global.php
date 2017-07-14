<?php

namespace {
    require 'includes/global_bar.php';
    require 'includes/foo_bar.php';
}

namespace {
    var_dump(bar());
}

namespace {
    use function foo\bar;
    var_dump(bar());
    echo "Done\n";
}

?>
