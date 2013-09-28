<?php

$xml = '<?xml version="1.0" encoding="UTF-8"?><response><t>6</t><t>7</t><t>8</t></response>';
$sxml = simplexml_load_string($xml);
foreach ($sxml as $k => $v) {
  var_dump($k, (int)$v);
}
