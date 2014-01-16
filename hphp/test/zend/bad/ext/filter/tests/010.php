<?php

var_dump(filter_var(array(1,"1","", "-23234", "text", "asdf234asdfgs", array()), FILTER_VALIDATE_INT, FILTER_REQUIRE_ARRAY));
var_dump(filter_var(array(1.2,"1.7","", "-23234.123", "text", "asdf234.2asdfgs", array()), FILTER_VALIDATE_FLOAT, FILTER_REQUIRE_ARRAY));
var_dump(filter_var(1, array()));
var_dump(filter_var(1, FILTER_SANITIZE_STRING, 1));
var_dump(filter_var(1, FILTER_SANITIZE_STRING, 0));
var_dump(filter_var(1, FILTER_SANITIZE_STRING, array()));
var_dump(filter_var(1, -1, array(123)));
var_dump(filter_var(1, 0, array()));

echo "Done\n";
?>