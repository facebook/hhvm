<?hh
<<__EntryPoint>> function main(): void {
// Initialise all required variables
$to = 'user@example.com';
$subject = 'Test Subject';
$message = 'A Message';
$additional_headers = "KHeaders\n\n\n\n\n";
$outFile = sys_get_temp_dir().'/'.'mail_bug51604.out';

// Calling mail() with all additional headers
var_dump( mail($to, $subject, $message, $additional_headers) );
echo file_get_contents($outFile);
unlink($outFile);

echo "===DONE===\n";
}
