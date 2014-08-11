<?php
$html = '<html><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>This is the title</title></head></html>';

$htmldoc = new DOMDocument();
$htmldoc->loadHTML($html);
$htmldoc->formatOutput = true;
echo $htmldoc->saveHTML();
?>
