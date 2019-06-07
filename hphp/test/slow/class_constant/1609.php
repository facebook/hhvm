<?hh

interface X {
  const A=1;
}
class Y {
  const B = 2;
}
class Z extends Y implements X {
  function x() {
    print self::A;
    print self::B;
    print Z::A;
    print Z::B;
    print X::A;
    print Y::B;
  }
}

<<__EntryPoint>>
function main_1609() {
$z = new Z;
$z->x();
}
