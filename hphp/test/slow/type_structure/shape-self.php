<?hh

class A {
  const C1 = 'C1';
  const C2 = 'C2';
  const C3 = 'C3';
  const C4 = 'C4';
  const C5 = 'C5';
  const C6 = 'C6';

  const type T = shape(
    self::C1 => int,
    self::C2 => int,
    self::C3 => int,
    self::C4 => int,
    self::C5 => int,
    self::C6 => bool,
  );

  public static function foo() :mixed{
    return type_structure(self::class, 'T');
  }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::foo());
}
