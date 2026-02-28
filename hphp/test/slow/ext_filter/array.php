<?hh

// based on zend/ext/filter/010.php

<<__EntryPoint>>
function main_array() :mixed{
var_dump(
  filter_var(vec[vec[]], FILTER_VALIDATE_FLOAT, FILTER_REQUIRE_ARRAY)
);
}
