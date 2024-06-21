<?hh

function expect_string(string $s): void {}
class C {}
function f<reify T as C>(): void {
  expect_string(T::class);
  expect_string(nameof T);
}
