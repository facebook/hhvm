<?php

$ai = new ArrayIterator(array(new stdClass(), new stdClass()));
$ci = new CachingIterator($ai);
var_dump(
$ci->__toString() // if conversion to string is done by echo, for example, an exeption is thrown. Invoking __toString explicitly covers different code.
);
?>