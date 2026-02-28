<?hh


<<__EntryPoint>>
function main_1652() :mixed{
$doc = simplexml_load_string('<?xml version="1.0"?><lists><list path="svn+ssh"><entry kind="dir"></entry><entry kind="file"></entry></list></lists>');
 foreach ($doc->list->offsetGet(0)->entry as $r) {
 var_dump($r->attributes());
}
}
