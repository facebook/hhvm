<?hh

/*  Testing for Error conditions  */

<<__EntryPoint>> function main(): void {
echo "\n *** Output for Error Conditions ***\n";
/*  Invalid Number of Arguments */
try { rtrim(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { rtrim("", " ", 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Testing the Normal behaviour of rtrim() function */

echo "\n *** Output for Normal Behaviour ***\n";
var_dump ( rtrim("rtrim test   \t\0 ") );                       /* without second Argument */
var_dump ( rtrim("rtrim test   " , "") );                       /* no characters in second Argument */
var_dump ( rtrim("rtrim test        ", " ") );                  /* with single space as second Argument */
var_dump ( rtrim("rtrim test \t\n\r\0\x0B", "\t\n\r\0\x0B") );  /* with multiple escape sequences as second Argument */
var_dump ( rtrim("rtrim testABCXYZ", "A..Z") );                 /* with characters range as second Argument */
var_dump ( rtrim("rtrim test0123456789", "0..9") );             /* with numbers range as second Argument */
var_dump ( rtrim("rtrim test$#@", "#@$") );                     /* with some special characters as second Argument */

/* String with embedded NULL */
echo "\n*** String with embedded NULL ***\n";
var_dump( rtrim("234\x0005678\x0000efgh\xijkl\x0n1", "\x0n1") );

/* heredoc string */
$str = <<<EOD
us
ing heredoc string
EOD;

echo "\n *** Using heredoc string ***\n";
var_dump( rtrim($str, "ing") );

echo "Done\n";
}
