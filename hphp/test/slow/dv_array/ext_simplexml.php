<?hh

function takes_and_dumps_varray(varray<mixed> $v): void {
  var_dump($v);
}
function takes_and_dumps_darray(darray<arraykey, mixed> $d): void {
  var_dump($d);
}

<<__EntryPoint>>
function main_xpath_clone() :mixed{
  $doc1 = new SimpleXMLElement('<test><a>content</a></test>');
  takes_and_dumps_varray($doc1->xpath("a"));
  takes_and_dumps_darray($doc1->getNamespaces());
  takes_and_dumps_darray($doc1->getDocNamespaces());
}
