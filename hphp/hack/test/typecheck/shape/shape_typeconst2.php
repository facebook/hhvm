<?hh

class C {
  const FOO_KEY = 'FOO';
  const type TShape = shape(
    C::FOO_KEY => string,
    'bar' => int,
    'baz' => bool,
  );
}
