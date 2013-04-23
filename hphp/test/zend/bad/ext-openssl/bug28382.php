<?php
$cert = file_get_contents(dirname(__FILE__) . "/bug28382cert.txt");
$ext = openssl_x509_parse($cert);
var_dump($ext['extensions']);
/* openssl 1.0 prepends the string "Full Name:" to the crlDistributionPoints array key.
	For now, as this is the one difference only between 0.9.x and 1.x, it's handled with
	placeholders to not to duplicate the test. When more diffs come, a duplication would
	be probably a better solution.
*/
?>