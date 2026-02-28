<?hh

function main() :mixed{
  $a = simplexml_load_string("<root />");
  var_dump($a->unknown);
  var_dump((bool) $a->unknown);
  var_dump((int) $a->unknown);
  var_dump((string) $a->unknown);
  var_dump((float)$a->unknown);
  var_dump(darray($a->unknown));
  var_dump($a->unknown == null);
  var_dump(null == $a->unknown);
}

<<__EntryPoint>>
function main_empty_node() :mixed{
main();
}
