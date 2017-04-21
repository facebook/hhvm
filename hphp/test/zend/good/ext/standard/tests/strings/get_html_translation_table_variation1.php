<?php
/* Prototype  : array get_html_translation_table ( [int $table [, int $quote_style [, string charset_hint]]] )
 * Description: Returns the internal translation table used by htmlspecialchars and htmlentities
 * Source code: ext/standard/html.c
*/

/*
 * test get_html_translation_table() with unexpected value for argument $table 
*/

echo "*** Testing get_html_translation_table() : usage variations ***\n";
// initialize all required variables
$quote_style = ENT_COMPAT;

// get an unset variable
$unset_var = 10;
unset($unset_var);

// a resource variable 
$fp = fopen(__FILE__, "r");

// array with different values
$values =  array (

  // array values
  array(),
  array(0),
  array(1),
  array(1, 2),
  array('color' => 'red', 'item' => 'pen'),

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // string values
  "string",
  'string',

  // objects
  new stdclass(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,

  // resource var
  $fp,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);


// loop through each element of the array and check the working of get_html_translation_table()
// when $table argument is supplied with different values
echo "\n--- Testing get_html_translation_table() by supplying different values for 'table' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $table = $values [$index];

  $v = get_html_translation_table($table, ENT_COMPAT, "UTF-8");
  if (is_array($v) && count($v) > 100)
    var_dump(count($v));
   elseif (is_array($v)) {
    asort($v);
    var_dump($v);
   } else {
    var_dump($v);
   }
   
  $v = get_html_translation_table($table, $quote_style, "UTF-8");
  if (is_array($v) && count($v) > 100)
    var_dump(count($v));
   elseif (is_array($v)) {
    asort($v);
    var_dump($v);
   } else {
    var_dump($v);
   }

  $counter ++;
}

// close resource
fclose($fp);

echo "Done\n";
?>
