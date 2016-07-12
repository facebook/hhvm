<?hh
function foo() {
  return <foo bar = "blah"><!--comment
  comment--><abc/>body{$x}
  body</foo>;
}
