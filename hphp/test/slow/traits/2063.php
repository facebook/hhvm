<?hh

trait T2 {
  require implements I2;
}

trait T implements I2 {
  use T2;
  private static ?self::TVal $override = null;
}

interface I2 {
  abstract const type TVal;
}

interface I1 extends I2 {
  const type TVal = darray<string, string>;
}

trait T1 implements I1 {}

abstract final class C {
  use T;
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(type_structure(C::class, 'TVal'));
}
