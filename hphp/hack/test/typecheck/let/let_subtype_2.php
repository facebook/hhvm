<?hh // experimental

class Base {}
class Subclass extends Base {}

function expect_base(Base $base): void {}
function expect_subclass(Subclass $sub): void {}

function foo(): void {
  let base : Base = new Subclass();
  let subclass : Subclass = new Subclass();
  expect_base(base);
  expect_subclass(subclass);
  expect_base(subclass);
}
