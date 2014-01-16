<?php
ini_set('allow_url_fopen', 1);


$urls = array();
$urls[] = "data://text/plain,foo\r\nbar\r\n";
$urls[] = "data://text/plain,\r\nfoo\r\nbar\r\n";
$urls[] = "data://text/plain,foo\r\nbar";

foreach($urls as $url) {
	echo strtr($url, array("\r" => "\\r", "\n" => "\\n")) . "\n";
	var_dump(file($url, FILE_IGNORE_NEW_LINES));
}
?>