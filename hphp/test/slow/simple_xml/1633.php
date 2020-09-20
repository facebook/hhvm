<?hh


<<__EntryPoint>>
function main_1633() {
$x = new SimpleXMLElement('<foo/>');
 $x->addAttribute('attr', 'one');
 $x->offsetSet('attr', 'two');
 var_dump((string)$x->offsetGet('attr'));
 var_dump($x->asXML());
}
