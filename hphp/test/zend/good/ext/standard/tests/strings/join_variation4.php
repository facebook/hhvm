<?php
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/

/*
 * test join() by passing different glue arguments
*/

echo "*** Testing join() : usage variations ***\n";

$glues = array (
  "TRUE",
  true,
  false,
  array("key1", "key2"),
  "",
  " ",
  "string\x00between",
  -1.566599999999999,
  NULL,
  -0,
  '\0'
);

$pieces = array (
  2,
  0,
  -639,
  -1.3444,
  true,
  "PHP",
  false,
  NULL,
  "",
  " ",
  6999.99999999,
  "string\x00with\x00...\0"
);
/* loop through  each element of $glues and call join */
$counter = 1;
for($index = 0; $index < count($glues); $index ++) {
  echo "-- Iteration $counter --\n";
  var_dump( join($glues[$index], $pieces) );
  $counter++;
}

echo "Done\n";
?>