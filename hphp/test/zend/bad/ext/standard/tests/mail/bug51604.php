<?php
ini_set('mail.add_x_header ',  Off);

ini_set('sendmail_path', tee mail_bug51604.out >/dev/null);

// Initialise all required variables
$to = 'user@company.com';
$subject = 'Test Subject';
$message = 'A Message';
$additional_headers = "KHeaders\n\n\n\n\n";
$outFile = "mail_bug51604.out";
@unlink($outFile);

// Calling mail() with all additional headers
var_dump( mail($to, $subject, $message, $additional_headers) );
echo file_get_contents($outFile);
unlink($outFile);

?>
===DONE===