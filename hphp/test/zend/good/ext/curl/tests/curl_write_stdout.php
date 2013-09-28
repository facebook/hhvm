<?php

$log_file = tempnam(sys_get_temp_dir(), 'php-curl-test');

$fp = fopen($log_file, 'w+');
fwrite($fp, "test");
fclose($fp);

$ch = curl_init();
curl_setopt($ch, CURLOPT_FILE, STDOUT);
curl_setopt($ch, CURLOPT_URL, 'file://' . $log_file);
curl_exec($ch);
curl_close($ch);

// cleanup
unlink($log_file);

?>