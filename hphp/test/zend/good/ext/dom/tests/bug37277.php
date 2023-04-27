<?hh <<__EntryPoint>> function main(): void {
$dom1 = new DOMDocument('1.0', 'UTF-8');

$xml = '<foo />';
$dom1->loadXML($xml);

$node = clone $dom1->documentElement;

$dom2 = new DOMDocument('1.0', 'UTF-8');
$dom2->appendChild($dom2->importNode($node->cloneNode(true), TRUE));

print $dom2->saveXML();
}
