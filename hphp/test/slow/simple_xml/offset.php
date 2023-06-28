<?hh

function main() :mixed{
  $x = new SimpleXMLElement(
    '<a><b><c>d</c></b><b><c>d</c></b></a>'
  );
  foreach ($x as $child) {
    var_dump($child->offsetExists(0) && $child->offsetGet(0) !== null);
    var_dump($child->offsetExists(1) && $child->offsetGet(1) !== null);

    var_dump((string) $child->offsetGet(0));
    var_dump((string) $child->offsetGet(0)->c);
  }
}

<<__EntryPoint>>
function main_offset() :mixed{
main();
}
