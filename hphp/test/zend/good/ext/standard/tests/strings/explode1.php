<?hh
/* Prototype: array explode ( string $delimiter, string $string [, int $limit] );
   Description: Returns an array of strings, each of which is a substring of string
                formed by splitting it on boundaries formed by the string delimiter.
                If limit is set, the returned array will contain a maximum of limit
                elements with the last element containing the rest of string.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing explode() for basic operations ***\n";
$delimiters = vec[
  "",  // len=0
  NULL,
  "abcd",  // string
  0,  // zero
  "0",
  TRUE,  // boolean value
  FALSE,
  -1,  // negative integer
  -11.23,  // double
  4,  // positive integer
  "%",
];
$string = "1234NULL23abcd00000TRUEFALSE-11.234444true-11.24%PHP%ZEND";
/* loop prints an array of strings, each of which is a substring of $string
   formed by splitting it on boundaries formed by the string $delimiter.
 */
$counter = 1;
foreach($delimiters as $delimiter) {
  echo "-- Iteration $counter --\n";
  try { var_dump( explode($delimiter, $string, -1) ); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump( explode($delimiter, $string, 0) ); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump( explode($delimiter, $string, 1) ); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump( explode($delimiter, $string, 2) ); } catch (Exception $e) { var_dump($e->getMessage()); }
  $counter++;
}

echo "\n*** Testing explode() with miscelleneous input arguments ***\n";

echo "\n-- Passing positive values of Limit to explode() --\n";
/* LIMIT=2 */
var_dump( explode("::", "mon::tues::wed::thurs::fri::sat::sun", 2) );

/* checking for LIMIT =0,1 */
echo "\n-- Passing limit values 0 and 1 to explode() --\n";
var_dump( explode(":", "Name:Phone:Address:City:State", 0) );
var_dump( explode(":", "Name:Phone:Address:City:State", 1) );

/* to check the maximum limit of string that can be given with limit<=0,
   default size is 50 but increases dynamically */
echo "\n*** Testing explode() for maximum limit of string with Limit = -1 ***\n";
var_dump( explode(":", "1:2:3:4:5:6:7:7:5:6:7:3:4:5:2:8:9:0:5:5:5:5:5:5:5:5:5:5:5:5:55:5:5:5%:%:%:%:5:5:5:%:%:5:5:5:5:5%:%:%:55:1:1", -1) );

echo "\n*** Testing explode() with string variations as input argument ***\n";
/* String with escape characters */
echo "\n-- Testing string with escape characters --\n";
var_dump( explode("\t\n", "1234\t\n5678\n\t9100") );
var_dump( explode("\r", "1234\rabcd\r5678\rrstu") );

/* String with embedded NULL */
echo "\n-- Testing string with embedded NULL --\n";
var_dump( explode("\x00", "abcd\x0n1234\x0005678\x0000efgh\xijkl") );
var_dump( explode("\0", "abcd\0efgh\0ijkl\x00mnop\x000qrst\00uvwx\000yz") );

echo "\n*** Testing error conditions ***\n";
/* checking for arguments <2 and >3 */
try { var_dump( explode(":", "array1:array2:array3", -1, -33) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( explode(":") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
