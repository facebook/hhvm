<?php
class Cls {
    function __call($name, $arg) {
    }
}

$cls = new Cls();
$cls->{0}();
$cls->{1.0}();
$cls->{true}();
$cls->{false}();
$cls->{null}();
echo "ok\n";