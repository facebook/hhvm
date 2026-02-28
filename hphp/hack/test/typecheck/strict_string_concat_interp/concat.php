<?hh

class Foo {}

// These should all have typing errors with the stricter rules.
function bad_concat(dynamic $a): void {
  "test" . false;
  null . "test";
  "test" . 123.0;
  "test" . new Foo();
  $a . null;
}

// These should typecheck.
function ok_concat(
  arraykey $a,
  arraykey $b,
  \HH\FormatString<\PlainSprintf> $c,
  dynamic $d,
): void {
  1 . 2;
  "test" . 2;
  "test" . "hello";
  $a . $b;
  $c . "other string";
  $d . $d;
  "some string" . $d;
}
