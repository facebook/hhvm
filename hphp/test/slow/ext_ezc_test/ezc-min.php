<?php
var_dump(ezc_min());
var_dump(ezc_min(1));
var_dump(ezc_min(2,1,2));
var_dump(ezc_min(2.1,2.11,2.09));
var_dump(ezc_min("", "t", "b"));
var_dump(ezc_min(false, true, false));
var_dump(ezc_min(true, false, true));
var_dump(ezc_min(1, true, false, true));
var_dump(ezc_min(0, true, false, true));

echo "Done\n";
?>
