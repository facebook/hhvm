<?php

$html = <<<HTML
<div id="test"><span>hi there</span></div>
HTML;

$text = '<p>hello world &trade;</p>';

$dom = new DOMDocument('1.0', 'UTF-8');
$dom->loadHTML($html);

$node = $dom->getElementById('test');
var_dump($node->textContent);
$node->textContent = $text;
var_dump($node->textContent == $text);

var_dump($dom->saveHTML($node));

?>
