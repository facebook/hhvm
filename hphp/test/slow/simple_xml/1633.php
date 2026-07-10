<?hh


<<__EntryPoint>>
function main_1633() :mixed{
$x = new SimpleXMLElement('<foo/>');
 $x->addAttribute('attr', 'one');
 $x->offsetSet('attr', 'two');
 var_dump($x->offsetGet('attr')->__toString());
 var_dump($x->asXML());
}
