<?hh

class :div {}

function foo(): mixed {
  return <div foo="a" foo="b">stuff</div>;
}
