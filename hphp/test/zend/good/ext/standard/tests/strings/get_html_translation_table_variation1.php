<?hh
/* Prototype  : array get_html_translation_table ( [int $table [, int $quote_style [, string charset_hint]]] )
 * Description: Returns the internal translation table used by htmlspecialchars and htmlentities
 * Source code: ext/standard/html.c
*/

/*
 * test get_html_translation_table() with unexpected value for argument $table
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing get_html_translation_table() : usage variations ***\n";
// initialize all required variables
$quote_style = ENT_COMPAT;


// a resource variable
$fp = fopen(__FILE__, "r");

// array with different values
$values =  vec[

  // array values
  vec[],
  vec[0],
  vec[1],
  vec[1, 2],
  dict['color' => 'red', 'item' => 'pen'],

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // string values
  "string",
  'string',

  // objects
  new stdClass(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,

  // resource var
  $fp,
];


// loop through each element of the array and check the working of get_html_translation_table()
// when $table argument is supplied with different values
echo "\n--- Testing get_html_translation_table() by supplying different values for 'table' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $table = $values [$index];

  $v = null;
  try { $v = get_html_translation_table($table, ENT_COMPAT, "UTF-8"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  if (is_array($v) && count($v) > 100)
    var_dump(count($v));
   else if (is_array($v)) {
    asort(inout $v);
    var_dump($v);
   } else {
    var_dump($v);
   }

  $v = null;
  try { $v = get_html_translation_table($table, $quote_style, "UTF-8"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  if (is_array($v) && count($v) > 100)
    var_dump(count($v));
   else if (is_array($v)) {
    asort(inout $v);
    var_dump($v);
   } else {
    var_dump($v);
   }

  $counter ++;
}

// close resource
fclose($fp);

echo "Done\n";
}
