<?hh


<<__EntryPoint>>
function main_1641() {
$x = new SimpleXMLElement('<foo/>');
$x->addChild('bar', 'whoops');
var_dump((string)$x);
}
