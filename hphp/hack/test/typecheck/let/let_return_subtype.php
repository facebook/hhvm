<?hh // strict

class Base {}
class Subclass extends Base {}

function foo(): Base {
  let sub = new Subclass();
  return sub;
}
