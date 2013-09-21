<?php

var_dump(get_included_files());

include(dirname(__FILE__)."/014.inc");
var_dump(get_included_files());

var_dump(get_included_files(1,1));

include_once(dirname(__FILE__)."/014.inc");
var_dump(get_included_files());

var_dump(get_included_files(1));

include(dirname(__FILE__)."/014.inc");
var_dump(get_included_files());

echo "Done\n";
?>