<?php

$dom = new DOMDocument;
$dom->loadHTML('<span title=""y">x</span><span title=""z">x</span>');
$html = simplexml_import_dom($dom);

var_dump($html->body->span);

foreach ($html->body->span as $obj) {
	var_dump((string)$obj->title);
}

?>