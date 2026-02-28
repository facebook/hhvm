<?hh

function foo(HH\FormatString<PlainSprintf> $fooString) :mixed{
  var_dump($fooString);
}
<<__EntryPoint>> function main(): void {
foo("Hello, World!");
foo(new stdClass);
}
