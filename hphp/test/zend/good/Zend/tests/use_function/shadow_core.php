<?php

require 'includes/foo_strlen.php';

use function foo\strlen;

var_dump(strlen('foo bar baz'));
echo "Done\n";

?>
