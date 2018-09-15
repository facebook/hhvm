<?php

// based on zend/ext/filter/010.php

<<__EntryPoint>>
function main_array() {
var_dump(
  filter_var(array(array()), FILTER_VALIDATE_FLOAT, FILTER_REQUIRE_ARRAY)
);
}
