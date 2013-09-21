<?php

var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, array("options"=>array("regexp"=>'/.*/'))));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, array("options"=>array("regexp"=>'/^b(.*)/'))));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, array("options"=>array("regexp"=>'/^d(.*)/'))));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, array("options"=>array("regexp"=>'/blah/'))));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, array("options"=>array("regexp"=>'/\[/'))));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP));

echo "Done\n";
?>