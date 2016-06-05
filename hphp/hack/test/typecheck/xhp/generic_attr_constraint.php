<?hh // strict

class :my-xhp<T as num> {
  attribute T foo @required;
}

function test(): void {
  <my-xhp foo="1" />;
}
