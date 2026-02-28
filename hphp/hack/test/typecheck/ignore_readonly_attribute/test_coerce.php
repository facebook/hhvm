<?hh

class Foo {}

function returns_readonly_fn(): (function(): readonly Foo) {
  return () ==> new Foo();
}

function takes_callback(
  <<__IgnoreReadonlyError>> (function(): (function(): Foo)) $f,
): void {}

function test(): void {
  // Assign to a local variable first
  $fn = returns_readonly_fn();
  // Return the variable - the coercion should be the primary check point
  takes_callback(() ==> $fn);
}
