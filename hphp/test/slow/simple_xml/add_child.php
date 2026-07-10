<?hh


<<__EntryPoint>>
function main_add_child() :mixed{
$x = new SimpleXMLElement('<foo/>');
$x->addChild('foobar');
var_dump($x->addChild('hello', 'world')->__toString());
var_dump($x->__toString());
var_dump($x);
foreach ($x->children() as $child) {
  var_dump($child->__toString());
}
}
