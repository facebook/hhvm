<?php
ini_set('mail.add_x_header ',  Off);

ini_set('sendmail_path', tee mailBasic.out >/dev/null);

/* Prototype  : int mail(string to, string subject, string message [, string additional_headers [, string additional_parameters]])
 * Description: Send an email message 
 * Source code: ext/standard/mail.c
 * Alias to functions: 
 */

echo "*** Testing mail() : basic functionality ***\n";


// Initialise all required variables
$to = 'user@company.com';
$subject = 'Test Subject';
$message = 'A Message';
$additional_headers = 'KHeaders';
$outFile = "mailBasic.out";
@unlink($outFile);

echo "-- All Mail Content Parameters --\n";
// Calling mail() with all additional headers
var_dump( mail($to, $subject, $message, $additional_headers) );
echo file_get_contents($outFile);
unlink($outFile);

echo "\n-- Mandatory Parameters --\n";
// Calling mail() with mandatory arguments
var_dump( mail($to, $subject, $message) );
echo file_get_contents($outFile);
unlink($outFile);

?>
===DONE===