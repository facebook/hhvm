<?php
$data = "\xFE\xFF\x00\x3C\x00\x66\x00\x6F\x00\x6F\x00\x2F\x00\x3E";

$dom = new DOMDocument();
$dom->loadXML($data);
echo $dom->saveXML();

?>
