<?php

var_dump(max());
var_dump(max(1));
var_dump(max(array()));
var_dump(max(new stdclass));
var_dump(max(2,1,2));
var_dump(max(2.1,2.11,2.09));
var_dump(max("", "t", "b"));
var_dump(max(false, true, false));
var_dump(max(true, false, true));
var_dump(max(1, true, false, true));
var_dump(max(0, true, false, true));

echo "Done\n";
?>