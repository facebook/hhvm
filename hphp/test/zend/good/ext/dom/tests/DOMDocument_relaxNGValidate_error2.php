<?hh <<__EntryPoint>> function main(): void {
$rng = dirname(__FILE__).'/foo.rng';
$xml = <<< XML
<?xml version="1.0"?>
<apple>
  <pear>Pear</pear>
  <pear>Pear</pear>
</apple>
XML;
$doc = new DOMDocument();
$doc->loadXML($xml);
$result = $doc->relaxNGValidate($rng);
var_dump($result);
}
