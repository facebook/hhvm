<?hh

class C {}

function f(): void {
  (string)C::class;
}
