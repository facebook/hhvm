<?php
/* Prototype: bool fnmatch ( string $pattern, string $string [, int $flags] )
   Description: fnmatch() checks if the passed string would match 
     the given shell wildcard pattern. 
*/

echo "*** Testing fnmatch() with file and various patterns ***\n";
$file_name = dirname(__FILE__)."/match.tmp";

/* avoid using \, it breaks the pattern */
if (substr(PHP_OS, 0, 3) == 'WIN') {
    $file_name = str_replace('\\','/', $file_name);
}

fopen($file_name, "w");

$pattern_arr = array(
0 => "*.tmp",
1 => "match*",
2 => "mat*",
3 => "mat*tmp",
4 => "m*t",
5 => "ma[pt]ch*",
6 => "*.t*",
7 => "***.tmp",
8 => "match**",
9 => "*.t*p",
10 => "",
11 => "match",
12 => ".tmp",
13 => "?match",
14 => "match?tmp",
15 => "?tmp",
16 => "match?",
17 => "?match?",
18 => "match.tmp",
19 => "/match.tmp",
20 => "/match.tmp/", 
21 => 'match.tmp',
22 => 'match.tmp\0',
23 => "match.tmp\0",
24 => "match\0.tmp",
25 => chr(109).chr(97)."tch.tmp",
26 => chr(109).chr(97).chr(116).chr(99).chr(104).".tmp",
27 => chr(109).chr(97).chr(116).chr(99).chr(104).chr(46).chr(116).chr(120).chr(116),
28 => chr(109).chr(97).chr(116).chr(99).chr(104).".".chr(116).chr(120).chr(116),
29 => "MATCH.TMP",
30 => "MATCH*",
31 => $file_name,

/* binary inputs */
32 => b"match*",
33 => b"*.tmp",
34 => b"mat*",
35 => b"mat*tmp",
36 => b"m*t",
);

for( $i = 0; $i<count($pattern_arr); $i++ ) {
  echo "-- Iteration $i --\n";
  var_dump( fnmatch($pattern_arr[$i], $file_name) );
}
unlink($file_name);


echo "\n*** Testing fnmatch() with other types other than files ***";

/* defining a common function */
function match( $pattern, $string ) {
  for( $i = 0; $i<count($pattern); $i++ ) {
    echo "-- Iteration $i --\n";
    for( $j = 0; $j<count($string); $j++ ) {
    var_dump( fnmatch($pattern[$i], $string[$j]) );
    }
  }
}

echo "\n--- With Integers ---\n";
$int_arr = array(
  16,
  16.00,
  020,
  020.00,
  0xF,
  0xF0000
);
match($int_arr, $int_arr);

echo "\n--- With Strings ---\n";
$str_arr = array(
  "string",
  "string\0",
  'string',
  "str\0ing",
  "stringstring",

  /* binary input */
  b"string"
);
match($str_arr, $str_arr);

echo "\n--- With booleans ---\n";
$bool_arr = array(
  TRUE,
  true,
  1,
  10,
  FALSE,
  false,
  0,
  "",
  "string"
);
match($bool_arr, $bool_arr);

echo "\n--- With NULL ---\n";
$null_arr = array(
  NULL,
  null,
  "",
  "\0",
  "string",
  0
);
match($null_arr, $null_arr);

echo "\n*** Done ***\n";
?>