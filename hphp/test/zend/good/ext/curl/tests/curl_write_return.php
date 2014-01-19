<?php

$log_file = tempnam(sys_get_temp_dir(), 'php-curl-test');

$fp = fopen($log_file, 'w+');
fwrite($fp, "test");
fclose($fp);

$ch = curl_init();
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($ch, CURLOPT_URL, 'file://' . $log_file);
$result = curl_exec($ch);
curl_close($ch);

echo $result;

// cleanup
unlink($log_file);

?>