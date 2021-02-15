<?hh

class :div extends XHPTest {}

function foo(): mixed {
  return <div foo="a" foo="b">stuff</div>;
}
