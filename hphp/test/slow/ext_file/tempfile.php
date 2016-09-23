<?php
$x = fopen("php://temp", "w+");
// ensure that ftell returns 0 and not false
var_dump(ftell($x));
