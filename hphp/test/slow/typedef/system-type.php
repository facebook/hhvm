<?hh

function foo(HH\FormatString<PlainSprintf> $fooString) {
  var_dump($fooString);
}
<<__EntryPoint>> function main(): void {
foo("Hello, World!");
foo(new stdClass);
}
