<?php

$xml = '<?xml version="1.0" encoding="UTF-8"?><response><t>6</t></response>';
$sxml = simplexml_load_string($xml);
function convert_simplexml_to_array($sxml) {
  if ($sxml) {
    foreach ($sxml as $k => $v) {
      var_dump($k, (string)$v);
      convert_simplexml_to_array($v);
    }
  }
}
convert_simplexml_to_array($sxml);
