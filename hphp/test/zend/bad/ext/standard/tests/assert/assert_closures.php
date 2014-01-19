<?php
assert_options(ASSERT_CALLBACK, function () { echo "Hello World!\n"; });
assert(0);
?>