<?php

/*  Testing for Error conditions  */

/*  Invalid Number of Arguments */

 echo "\n *** Output for Error Conditions ***\n";

 echo "\n *** Output for zero argument ***\n";
 var_dump( ltrim() );

 echo "\n *** Output for more than valid number of arguments (Valid are 1 or 2 arguments) ***\n";
 var_dump( ltrim("", " ", 1) );

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
 var_dump ( ltrim("        ltrim test", NULL) );                 /* with NULL as second Argument */
 var_dump ( ltrim("        ltrim test", true) );                 /* with boolean value as second Argument */
 var_dump ( ltrim("        ltrim test", " ") );                  /* with single space as second Argument */
 var_dump ( ltrim("\t\n\r\0\x0B ltrim test", "\t\n\r\0\x0B") );  /* with multiple escape sequences as second Argument */
 var_dump ( ltrim("ABCXYZltrim test", "A..Z") );                 /* with characters range as second Argument */
 var_dump ( ltrim("0123456789ltrim test", "0..9") );             /* with numbers range as second Argument */
 var_dump ( ltrim("@$#ltrim test", "#@$") );                     /* with some special characters as second Argument */
 

 echo "\n *** Output for  scalar argument) ***\n";
 var_dump( ltrim(  12345  ) );                                   /* Scalar argument */

 echo "\n *** Output for  NULL argument) ***\n";
 var_dump( ltrim(NULL) );                                        /* NULL Argument */

echo "\nDone\n";

?>