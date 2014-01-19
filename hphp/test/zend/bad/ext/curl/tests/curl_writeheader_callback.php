<?php

function curl_header_callback($curl_handle, $data)
{
	if (strtolower(substr($data,0, 4)) == 'http')
		echo $data;
}

$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');

$ch = curl_init();
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($ch, CURLOPT_HEADERFUNCTION, 'curl_header_callback');
curl_setopt($ch, CURLOPT_URL, $host);
curl_exec($ch);
curl_close($ch);

?>