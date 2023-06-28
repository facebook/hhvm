<?hh

enum MyEnum : string as string {
  A = '1';
  B = '2';
  C = '3';
  D = '4';
  E = '5';
  F = '6';
  G = '7';
  H = '8';
  I = '9';
  J = '10';
  K = '11';
  L = '12';
}

class A {
  const dict<string, MyEnum> MAP = dict[
    "A" => MyEnum::A,
    "B" => MyEnum::B,
    "C" => MyEnum::C,
    "D" => MyEnum::D,
    "E" => MyEnum::E,
    "F" => MyEnum::F,
    "G" => MyEnum::G,
    "H" => MyEnum::H,
    "J" => MyEnum::J,
    "K" => MyEnum::A,
    "L" => MyEnum::B,
    "M" => MyEnum::C,
    "N" => MyEnum::D,
    "O" => MyEnum::E,
    "P" => MyEnum::F,
    "Q" => MyEnum::G,
    "R" => MyEnum::H,
    "S" => MyEnum::I,
    "T" => MyEnum::J,
    "A1" => MyEnum::A,
    "B1" => MyEnum::B,
    "C1" => MyEnum::C,
    "D1" => MyEnum::D,
    "E1" => MyEnum::E,
    "F1" => MyEnum::F,
    "G1" => MyEnum::G,
    "H1" => MyEnum::H,
    "I1" => MyEnum::I,
    "J1" => MyEnum::J,
    "K1" => MyEnum::A,
    "L1" => MyEnum::B,
    "M1" => MyEnum::C,
    "N1" => MyEnum::D,
    "O1" => MyEnum::E,
    "P1" => MyEnum::F,
    "Q1" => MyEnum::G,
    "R1" => MyEnum::H,
    "S1" => MyEnum::I,
    "T1" => MyEnum::J,
  ];
}

<<__EntryPoint>>
function main() :mixed{
  // Set timer to a value such that it interrupts resolving A::MAP on line 69
  $t1 = new IntervalTimer(0.0, 0.035, () ==> {A::MAP;});
  $t1->start();

  A::MAP;

  $t1->stop();
  echo "OK\n";
}
