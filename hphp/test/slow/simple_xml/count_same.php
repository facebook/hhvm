<?hh

function main() :mixed{
  $x = new SimpleXMLElement(
    '<a><b/><b/><c/></a>'
  );
  var_dump($x->children()->count());
}

<<__EntryPoint>>
function main_count_same() :mixed{
main();
}
