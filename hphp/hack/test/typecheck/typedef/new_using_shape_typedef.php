<?hh

type S = shape(
  'a' => int,
);

function f(): void {
  $f = new S(42);
}
