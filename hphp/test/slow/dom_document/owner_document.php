<?hh

function main() :mixed{
  $doc = new DOMDocument();
  $root = $doc->createElement('root');
  var_dump($doc);
  var_dump($root->ownerDocument);
  var_dump($root->ownerDocument === $doc);
}


<<__EntryPoint>>
function main_owner_document() :mixed{
main();
}
