<?hh

<<__EntryPoint>>
function main_node_identity() :mixed{
$dom = new \DOMDocument('1.0', 'UTF-8');
$root = $dom->createElement('root');
$dom->appendChild($root);

// expected to be TRUE, actually FALSE
var_dump($root === $dom->documentElement);
}
