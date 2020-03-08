<?hh // strict

namespace NS_abstract_constants;

interface I1 {
  abstract const int CI;
  abstract const int CI2;
}

interface I2 extends I1 {
  abstract const int CI;
  const int CI2 = 50;
}

abstract class C1 implements I2 {
  const int CI = 10;
  abstract const int C3;
}

class C2 implements I2 {
  const int CI = 20;
  const int C3 = 55;
}

class C3 extends C1 {
  const int CI = 99;
  const int C3 = 66;
}

function main(): void {
  var_dump(new C2());
  var_dump(new C3());
}

/* HH_FIXME[1002] call to main in strict*/
main();
