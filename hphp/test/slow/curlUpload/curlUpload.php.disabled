<?php
$fp = fopen(__DIR__.'/curlUpload.png', 'rb');

$ch = curl_init('http://www.facebook.com/status.php');
curl_setopt($ch, CURLOPT_USERAGENT, 'GGGGG');
curl_setopt($ch, CURLOPT_UPLOAD, TRUE);
curl_setopt($ch, CURLOPT_INFILE, $fp);
curl_setopt($ch, CURLOPT_INFILESIZE,
  filesize(__DIR__.'/curlUpload.png'));
curl_setopt($ch, CURLOPT_READFUNCTION, '_read_cb');

$data = curl_exec($ch);
var_dump($data);
var_dump(curl_errno($ch));
var_dump(curl_error($ch));

function _read_cb($ch, $fd, $length) {
        $data = fread($fd, $length);
        return $data;
}

