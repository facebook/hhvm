<?hh // experimental

class Base {}
class Subclass extends Base {}

function expect_base(Base $base): void {}
function expect_subclass(Subclass $sub): void {}

function foo(): void {
  let subclass : Subclass = new Base();
  expect_subclass(subclass);
  expect_base(subclass);
}
