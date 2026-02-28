<?hh

class Foo {}

function returns_readonly_fn(): (function(): readonly Foo) {
  return () ==> new Foo();
}

function takes_callback(
  <<__IgnoreReadonlyError>> (function(): (function(): (function(): Foo))) $f,
): void {}

function test(): void {
  // Nested lambda with direct function call - needs bidirectional fix
  takes_callback(() ==> () ==> returns_readonly_fn());
}
