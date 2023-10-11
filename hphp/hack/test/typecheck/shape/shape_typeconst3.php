<?hh

class C {
  const FOO_KEY = 'foo';
  const type TShape = shape(
    C::FOO_KEY => string,
    C::MISSING_CONST => null,
  );
}
