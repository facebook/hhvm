<?hh

class C {}
class D {}
type X = C;
type Y = D;

function expect(typename<C> $t): void {}
function f(): void {
  expect(X::class);
  expect(nameof X);
  expect(Y::class);
  expect(nameof Y);
}
