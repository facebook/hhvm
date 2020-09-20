<?hh


<<__EntryPoint>>
function main_1646() {
$xml = '<?xml version="1.0" encoding="UTF-8"?><root><invalidations><invalidation id="12345"/></invalidations></root>';
$dom = new SimpleXMLElement($xml);
$invalidations = $dom->invalidations;
var_dump((string)$invalidations->invalidation->offsetGet("id"));
foreach ($invalidations as $node) {
  var_dump((string)$node->invalidation->offsetGet("id"));
}
}
