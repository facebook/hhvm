<?hh
/* Prototype:
     string strtoupper ( string $string );
   Description:
     Returns string with all alphabetic characters converted to uppercase */
<<__EntryPoint>> function main(): void {
if( substr(PHP_OS, 0, 3) == 'WIN') {
  setlocale(LC_ALL, 'C');
} else {
  setlocale(LC_ALL, 'en_US.UTF-8');
}

echo "*** Testing strtoupper() with 128 chars ***\n";
for ($i=0; $i<=127; $i++){
  $char = chr($i);
  print(bin2hex($char))." => ".(bin2hex(strtoupper("$char")))."\n";
}

echo "\n*** Testing strtoupper() with basic strings ***\n";
$str = "Mary Had A liTTle LAmb and ShE loveD IT So\n";
var_dump(strtoupper($str));

echo "\n*** Testing strtoupper() with various strings ***";
/* strings to pass strtoupper() */
$strings = varray [
  "",
  "string",
  "stRINg0234",
  "1.233.344StrinG12333",
  "$$$$$$!!!!@@@@@@@ ABCDEF !!!***",
  "ABCD\0abcdABCD",
  NULL,
  TRUE,
  FALSE,
  varray[]
];

$count = 0;
/* loop through to check possible variations */
foreach ($strings as $string) {
  echo "\n-- Iteration $count --\n";
  try { var_dump( strtoupper($string) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count++;
}

echo "\n*** Testing strtoupper() with two different case strings ***\n";
if (strtoupper("HeLLo woRLd") === strtoupper("hEllo WORLD"))
  echo "strings are same, with Case Insensitive\n";
else
  echo "strings are not same\n";

echo "\n*** Testing error conditions ***";
try { var_dump( strtoupper() ); /* Zero arguments */ } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( strtoupper("a", "b") ); /* Arguments > Expected */ } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "*** Done ***";
}
