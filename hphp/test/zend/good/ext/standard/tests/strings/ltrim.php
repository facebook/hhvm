<?hh
/*  Testing for Error conditions  */

/*  Invalid Number of Arguments */

<<__EntryPoint>> function main(): void {
echo "\n *** Output for Error Conditions ***\n";

echo "\n *** Output for zero argument ***\n";
try { var_dump( ltrim() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n *** Output for more than valid number of arguments (Valid are 1 or 2 arguments) ***\n";
try { var_dump( ltrim("", " ", 1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* heredoc string */
$str = <<<EOD
us
ing heredoc string
EOD;

echo "\n *** Using heredoc string ***\n";
var_dump( ltrim($str, "\nusi") );

/* Testing the Normal behaviour of ltrim() function */

 echo "\n *** Output for Normal Behaviour ***\n";
 var_dump ( ltrim(" \t\0    ltrim test") );                      /* without second Argument */
 var_dump ( ltrim("   ltrim test" , "") );                       /* no characters in second Argument */
 var_dump ( ltrim("        ltrim test", '1') );                  /* with 1 as second Argument */
 var_dump ( ltrim("        ltrim test", " ") );                  /* with single space as second Argument */
 var_dump ( ltrim("\t\n\r\0\x0B ltrim test", "\t\n\r\0\x0B") );  /* with multiple escape sequences as second Argument */
 var_dump ( ltrim("ABCXYZltrim test", "A..Z") );                 /* with characters range as second Argument */
 var_dump ( ltrim("0123456789ltrim test", "0..9") );             /* with numbers range as second Argument */
 var_dump ( ltrim("@$#ltrim test", "#@$") );                     /* with some special characters as second Argument */

echo "\nDone\n";
}
