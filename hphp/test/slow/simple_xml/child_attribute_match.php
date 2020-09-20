<?hh


<<__EntryPoint>>
function main_child_attribute_match() {
$p = new SimpleXMLElement('<parent/>');
$c = $p->addChild('child', '123');
$c->addAttribute('attr', 'hi');

$cattrs = darray[];
foreach ($c->attributes() as $k => $v) {
  $cattrs[$k] = $v;
}

// $p->children()[0] should be the same node as $c
$pcattrs = darray[];
foreach ($p->children()->offsetGet(0)->attributes() as $k => $v) {
  $pcattrs[$k] = $v;
}

var_dump(count($cattrs) === count($pcattrs));
}
