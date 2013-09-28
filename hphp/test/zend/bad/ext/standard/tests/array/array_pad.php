<?php

var_dump(array_pad());
var_dump(array_pad(array()));
var_dump(array_pad(array(), 1));
var_dump(array_pad(array(), 1, 0));

var_dump(array_pad(array(), 0, 0));
var_dump(array_pad(array(), -1, 0));
var_dump(array_pad(array("", -1, 2.0), 5, 0));
var_dump(array_pad(array("", -1, 2.0), 5, array()));
var_dump(array_pad(array("", -1, 2.0), 2, array()));
var_dump(array_pad(array("", -1, 2.0), -3, array()));
var_dump(array_pad(array("", -1, 2.0), -4, array()));
var_dump(array_pad(array("", -1, 2.0), 2000000, 0));
var_dump(array_pad("", 2000000, 0));

echo "Done\n";
?>