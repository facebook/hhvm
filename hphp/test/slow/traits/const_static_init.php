<?hh

trait T2 {
  const TWENTY_FOUR = 24;
}

trait T1 {
  use T2;
  const TWENTY_FOUR1 = T2::TWENTY_FOUR;
}

interface I2 {
  const THIRTY_TWO = 32;
}

interface I extends I2 {
  const FORTY_EIGHT = 48;
  const THIRTY_TWO1 = I2::THIRTY_TWO;
}

trait T implements I {
  use T1;
  const FORTY_EIGHT1 = I::FORTY_EIGHT;
}

class C {
  use T;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C::TWENTY_FOUR1);  // trait -> trait
  var_dump(C::FORTY_EIGHT1);  // trait -> interface
  var_dump(C::THIRTY_TWO1);   // interface -> interface
}
