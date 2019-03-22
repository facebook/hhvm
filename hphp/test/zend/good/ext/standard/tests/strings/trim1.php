<?php

/* Prototype: string trim( string str [,string charlist] )
 * Strip whitespace (or other characters) from the beginning and end of a string.
 */

/* trim with unset/null/boolean variable - retuns an empty string */
echo "\n";
$null_var = NULL;
var_dump( trim($null_var) );
$null_var = "";
var_dump( trim($null_var) );
$null_var = 0;
var_dump( trim($null_var) );
$bool_val = true;
var_dump( trim($null_var) );

/* second argument charlist as null - does not trim any white spaces */
var_dump( trim("\ttesting trim", "") );
var_dump( trim("  \ttesting trim  ", NULL) );
var_dump( trim("\ttesting trim  ", true) );

/* Testing error conditions */
echo "\n*** Testing error conditions ***\n";

//Zero arguments
try { var_dump( trim() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
// More than expected number of args */
try { var_dump( trim("\tstring\n", "\t\n", $null_var) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( trim(NULL, "", NULL ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


/* Use of class and objects */
echo "\n*** Testing with OBJECTS ***\n";
class string1 
{
  public function __toString() {
    return "Object";
  }
}
$obj = new string1;
var_dump( trim($obj, "Ot") );

/* String with embedded NULL */
echo "\n*** Testing with String with embedded NULL ***\n";
var_dump( trim("\x0n1234\x0005678\x0000efgh\xijkl\x0n1", "\x0n1") );

/* heredoc string */
$str = <<<EOD
us
ing heredoc string
EOD;

echo "\n*** Testing with heredoc string ***\n";
var_dump( trim($str, "us\ning") );

echo "\nDone";
