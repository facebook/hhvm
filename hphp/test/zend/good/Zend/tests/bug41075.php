<?php

function err($errno, $errstr, $errfile, $errline)
{
	    throw new Exception($errstr);
}

set_error_handler("err");

class test {
    function foo() {
        $var = $this->blah->prop = "string";
        var_dump($this->blah);
    }
}

$t = new test;
try {
    $t->foo();
} catch (Exception $e) {
    var_dump($e->getMessage());
}

echo "Done\n";
?>