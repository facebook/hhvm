<?hh

class C {}

function f(): void {
  $c = new C();
  HH\FIXME\UNSAFE_CAST<mixed, int>($c);
}
