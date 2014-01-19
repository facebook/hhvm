<?php

$xml = '<?xml version="1.0" encoding="UTF-8"?><response>test</response>';
$sxml = simplexml_load_string($xml);
foreach ($sxml as $k => $v) {
  var_dump($k, (string)$v);
}
