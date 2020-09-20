<?hh
/* Prototype  : string stripslashes ( string $str )
 * Description: Un-quotes a quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Testing stripslashes() with strings
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing stripslashes() : basic functionality ***\n";

// Initialize all required variables
$str_array = varray[ "\\",
                    "ends with slash\\",
                  ];

// Calling striplashes() with all arguments
foreach( $str_array as $str ) {
  $str_stripslashes = stripslashes($str);
  echo "$str:\n";
  $char = str_split($str_stripslashes);
  foreach ($char as $c) {
    print(sprintf("%d\n", ord($c)));
  }
}

echo "Done\n";
}
