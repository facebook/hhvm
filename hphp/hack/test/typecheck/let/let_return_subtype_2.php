<?hh // experimental

class Base {}
class Subclass extends Base {}

function foo(): Base {
  let sub : Base = new Subclass();
  return sub;
}

function bar(): Base {
  let sub : Subclass = new Subclass();
  return sub;
}
