<?hh

newtype T = (int, float);

function f(): void {
  $f = new T(42);
}
