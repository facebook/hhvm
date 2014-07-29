<?php
date_default_timezone_set("UTC");

$logfile = ini_get("mail.log");
if (file_exists($logfile)) {
	unlink($logfile);
}
touch($logfile);
clearstatcache();

$to = "test@example.com";
$subject = "mail.log test";
$message = "Testing mail.log";
$headers = "X-Test: 1";

var_dump(filesize($logfile) == 0);
clearstatcache();

var_dump(mail($to, $subject, $message, $headers));

var_dump(filesize($logfile) > 0);
clearstatcache();

echo file_get_contents($logfile);
?>
Done
<?php
unlink("/tmp/mail.log");
unlink("/tmp/mail.out");
?>