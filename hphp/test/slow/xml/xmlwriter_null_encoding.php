<?hh

function main() :mixed{
  $x = new XMLWriter();
  $x->openMemory();
  $x->startDocument('1.0', null);
  $x->writeElement('root');
  $x->endDocument();
  var_dump($x->outputMemory(true));
}


<<__EntryPoint>>
function main_xmlwriter_null_encoding() :mixed{
main();
}
