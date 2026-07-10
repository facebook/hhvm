<?hh


<<__EntryPoint>>
function main_1646() :mixed{
$xml = '<?xml version="1.0" encoding="UTF-8"?><root><invalidations><invalidation id="12345"/></invalidations></root>';
$dom = new SimpleXMLElement($xml);
$invalidations = $dom->invalidations;
var_dump($invalidations->invalidation->offsetGet("id")->__toString());
foreach ($invalidations as $node) {
  var_dump($node->invalidation->offsetGet("id")->__toString());
}
}
