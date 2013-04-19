<?php
$curl = curl_init();

$fd = tmpfile();
curl_setopt($curl, CURLOPT_FILE, $fd);

curl_copy_handle($curl);

echo 'DONE!';
?>