<?hh // strict

class Foo {}

// These should all have typing errors with the stricter rules.
function bad_concat(): void {
  "test" . false;
  null . "test";
  "test" . 123.0;
  "test" . new Foo();
}

// These should typecheck.
function ok_concat(
  arraykey $a,
  arraykey $b,
  \HH\FormatString<\PlainSprintf> $c
): void {
  1 . 2;
  "test" . 2;
  "test" . "hello";
  $a . $b;
  $c . "other string";
}
