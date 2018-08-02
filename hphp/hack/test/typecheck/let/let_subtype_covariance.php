<?hh // experimental

class Box<+T> {}

class Base {}
class Subclass extends Base {}

function expect_base_box(Box<Base> $box): void {}
function expect_subclass_box(Box<Subclass> $box): void {}

function foo(): void {
  let box1 : Box<Base> = new Box<Subclass>();
  let box2 : Box<Subclass> = new Box<Subclass>();
  expect_base_box(box1);
  expect_base_box(box2);
  expect_subclass_box(box2);
}
