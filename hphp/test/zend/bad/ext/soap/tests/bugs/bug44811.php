<?php
try {
    $x = new SoapClient('http://slashdot.org');
} catch (SoapFault $e) {
    echo $e->getMessage() . PHP_EOL;
}
die('ok');
?>