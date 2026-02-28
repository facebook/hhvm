<?hh

function bar() :mixed{
  echo "bar()\n";
  return "World";
}

function foo($a, $b = bar(), $c = 123) :mixed{
  echo "foo(\n";
  echo "  "; var_dump($a);
  echo "  "; var_dump($b);
  echo "  "; var_dump($c);
  echo ");\n";
}


<<__EntryPoint>>
function main_non_static_default() :mixed{
foo("Hello");
foo("Hello");
foo("Goodbye", "Land");
}
