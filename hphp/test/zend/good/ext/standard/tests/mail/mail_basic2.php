<?hh
/* Prototype  : int mail(string to, string subject, string message [, string additional_headers [, string additional_parameters]])
 * Description: Send an email message 
 * Source code: ext/standard/mail.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mail() : basic functionality ***\n";


// Initialise all required variables
$to = 'user@example.com';
$subject = 'Test Subject';
$message = 'A Message';
$additional_headers = 'KHeaders';
$additional_parameters = "-n";
$outFile = "/tmp/php_test_mailBasic2.out";
@unlink($outFile);

echo "-- extra parameters --\n";
// Calling mail() with all possible arguments
var_dump( mail($to, $subject, $message, $additional_headers, $additional_parameters) );

echo file_get_contents($outFile);
unlink($outFile);
echo "===DONE===\n";
}
