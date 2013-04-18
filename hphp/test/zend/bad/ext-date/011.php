<?php
date_default_timezone_set('UTC');

var_dump(timezone_name_from_abbr());
var_dump(timezone_name_from_abbr("CET"));
var_dump(timezone_name_from_abbr("AEST"));
var_dump(timezone_name_from_abbr("", 3600));
var_dump(timezone_name_from_abbr("", 3600, 0));

echo "Done\n";
?>