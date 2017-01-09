<?hh
function foo() {
  return <foo bar = "blah">#<!--comment
  comment--><abc/>body{$x}#
  body</foo>;
}
class :c {
  category %x, %y, ;
  children (foo+ | def, %bar*, :blah-blah?)*;
}
function bar() {
  return $a ? :x:y::Z : $b;
}
