<?hh

class Bar {}

type MyShape = shape(
  'first' => ?float,
  'second' => ?Bar,
  'third' => ?string,
);

<<__Memoize>>
function some_function(MyShape $i): void {}
