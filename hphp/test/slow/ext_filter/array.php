<?hh

// based on zend/ext/filter/010.php

<<__EntryPoint>>
function main_array() {
var_dump(
  filter_var(varray[varray[]], FILTER_VALIDATE_FLOAT, FILTER_REQUIRE_ARRAY)
);
}
