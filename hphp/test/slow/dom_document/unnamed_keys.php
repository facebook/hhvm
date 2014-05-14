<?php

$dom = new DOMDocument('1.0','UTF-8');
$dom->loadXML('<div><a/><a/></div>');

// $dom->documentElement->childNodes is a DOMNodeList

foreach($dom->documentElement->childNodes as $k => $v){
  var_dump($k);
}
