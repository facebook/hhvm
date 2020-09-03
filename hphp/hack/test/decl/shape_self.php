<?hh // strict

class TestClass {
  const string KEY = 'key';

  const type TClassType = shape(
    self::KEY => int,
  );
}
