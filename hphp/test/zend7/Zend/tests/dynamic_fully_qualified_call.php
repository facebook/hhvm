<?php

namespace Foo;
try {
    ('\bar')();
} catch (\Error $e) {
    echo $e->getMessage(), "\n";
}

?>
