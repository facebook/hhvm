<?php

$xml = '<?xml version="1.0" encoding="UTF-8"?><root><invalidations><invalidation id="12345"/></invalidations></root>';
$dom = new SimpleXMLElement($xml);
$invalidations = $dom->invalidations;
var_dump((string)$invalidations->invalidation["id"]);
foreach ($invalidations as $node) {
  var_dump((string)$node->invalidation["id"]);
}
