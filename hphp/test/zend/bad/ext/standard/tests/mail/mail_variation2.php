<?php
ini_set('mail.add_x_header ',  Off);

ini_set('mail.force_extra_parameters', "-n");

ini_set('sendmail_path', "cat > /tmp/php_test_mailVariation2.out");

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
$outFile = "/tmp/php_test_mailVariation2.out";
@unlink($outFile);

var_dump( mail($to, $subject, $message) );
echo file_get_contents($outFile);
unlink($outFile);

?>
===DONE===