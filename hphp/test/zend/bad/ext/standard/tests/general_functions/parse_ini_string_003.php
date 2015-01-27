<?php

$contents = <<<EOS
foo = 1
bar = 1.3
baz = null
qux[] = false
qux[] = off
qux[] = something
qux[] = "something else"
EOS;

var_dump(parse_ini_string($contents, false, INI_SCANNER_TYPED));

?>
Done
