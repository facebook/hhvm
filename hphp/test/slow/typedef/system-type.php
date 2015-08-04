<?hh

function foo(HH\FormatString<PlainSprintf> $fooString) {
  var_dump($fooString);
}

foo("Hello, World!");
foo(new stdClass);
