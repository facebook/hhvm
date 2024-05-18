<?hh

trait FooTrait {}

class C{}

function foo<<<__Enforceable>> reify T>(mixed $in): void {
  $in as T;
}

function test(): void {
  foo<FooTrait>(42);
  foo<C>(42);
}
