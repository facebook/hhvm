<?php

$xml = '<?xml version="1.0"?><dependencies><dependency dependency_id="0" dependent_id="1"/><dependency dependency_id="4" dependent_id="5"/><dependency dependency_id="5" dependent_id="6"/><dependency dependency_id="9" dependent_id="8"/><dependency dependency_id="10" dependent_id="8"/><dependency dependency_id="12" dependent_id="13"/><dependency dependency_id="12" dependent_id="14"/></dependencies>';
$dom = new domDocument;
$dom->loadxml($xml);
$xpath = new DOMXPath($dom);
$node_list = $xpath->query('//dependencies/dependency[@dependent_id = 8]');
foreach ($node_list as $node) {
  var_dump($node->getAttribute($attribute));
}
