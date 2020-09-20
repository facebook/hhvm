<?hh

function main() {
  $x = new SimpleXMLElement(
    '<a><b/><b/><c/></a>'
  );
  var_dump($x->children()->count());
}

<<__EntryPoint>>
function main_count_same() {
main();
}
