<?hh // experimental

class Base {}
class Subclass extends Base {}

function foo(): Subclass {
  let sub : Base = new Subclass();
  return sub;
}
