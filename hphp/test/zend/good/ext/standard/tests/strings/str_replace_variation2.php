<?hh
<<__EntryPoint>>
function entrypoint_str_replace_variation2(): void {
  /*
    Prototype: mixed str_replace(mixed $search, mixed $replace,
                                 mixed $subject [, int &$count]);
    Description: Replace all occurrences of the search string with
                 the replacement string
  */

  echo "\n*** Testing str_replace() with various subjects ***";
  $subject = "Hello, world,0120333.3445-1.234567          NULL TRUE FALSE\000
 	    \x000\x5ACD\0abcd \xXYZ\tabcd $$@#%^&*!~,.:;?: !!Hello, World 
	    ?Hello, World chr(0).chr(128).chr(234).chr(65).chr(255).chr(256)";

  /* needles in an array to be compared in the string $string */
  $search_str = vec[
    "Hello, World",
    'Hello, World',
    '!!Hello, World',
    "??Hello, World",
    "$@#%^&*!~,.:;?",
    "123",
    123,
    "-1.2345",
    -1.2344,
    "abcd",
    'XYZ',
    NULL,
    "NULL",
    "0",
    0,
    "",
    " ",
    "\0",
    "\x000",
    "\x5AC",
    "\0000",
    ".3",
    TRUE,
    "TRUE",
    "1",
    1,
    FALSE,
    "FALSE",
    " ",
    "          ",
    'b',
    '\t',
    "\t",
    chr(128).chr(234).chr(65).chr(255).chr(256),
    $subject
  ];

  /* loop through to get the  $string */
  for( $i = 0; $i < count($search_str); $i++ ) {
    echo "\n--- Iteration $i ---";
    echo "\n-- String after replacing the search value is => --\n";
    $count = 0;
    var_dump( str_replace_with_count($search_str[$i], "FOUND", $subject, inout $count) );
    echo "-- search string has found '$count' times\n";
  }

  echo "===DONE===\n";
}
