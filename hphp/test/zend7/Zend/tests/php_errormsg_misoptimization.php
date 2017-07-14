<?php

function test() {
    $php_errormsg = 1;
    echo $undef;
    var_dump($php_errormsg + 1);
}
test();

?>
