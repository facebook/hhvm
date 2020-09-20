<?hh
/* Prototype  : int mail(string to, string subject, string message [, string additional_headers [, string additional_parameters]])
 * Description: Send an email message 
 * Source code: ext/standard/mail.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mail() : error conditions ***\n";


//Test mail with one more than the expected number of arguments
echo "\n-- Testing mail() function with more than expected no. of arguments --\n";
$to = 'string_val';
$subject = 'string_val';
$message = 'string_val';
$additional_headers = 'string_val';
$additional_parameters = 'string_val';
$extra_arg = 10;
try { var_dump( mail($to, $subject, $message, $additional_headers, $additional_parameters, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing mail with one less than the expected number of arguments
echo "\n-- Testing mail() function with less than expected no. of arguments --\n";
$to = 'string_val';
$subject = 'string_val';
try { var_dump( mail($to, $subject) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
