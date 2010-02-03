<?php

$t = mktime(0,0,0, 6, 27, 2006);
///var_dump(localtime(1,1,1));

///var_dump(localtime());
var_dump(localtime($t));
var_dump(localtime($t, true));
var_dump(localtime($t, false));

echo "Done\n";
