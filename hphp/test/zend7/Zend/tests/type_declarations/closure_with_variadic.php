<?php
$f = function (stdClass ...$a) {
    var_dump($a);
};
$f(new stdClass);
?>
