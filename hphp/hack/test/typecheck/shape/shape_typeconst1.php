<?hh

class C {
  const BAZ_KEY = '';
  const type TShape = shape(
    'foo' => int,
    'bar' => string,
    C::BAZ_KEY => bool,
  );
}
