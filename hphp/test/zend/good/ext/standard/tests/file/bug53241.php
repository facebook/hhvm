<?php
$fn = __DIR__ . "/test.tmp";
@unlink($fn);
$fh = fopen($fn, 'xb');
$ch = curl_init('http://www.yahoo.com/');
var_dump(curl_setopt($ch, CURLOPT_FILE, $fh));
echo "Done.\n";
?>
<?php error_reporting(0); ?>
<?php
$fn = __DIR__ . "/test.tmp";
@unlink($fn);
?>