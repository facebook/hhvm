<?hh

const int FOO = 1;
const int BAR = A::BAR;
const int QUX = BAZ;

class A {
  const int BAR = 2;
}

<<__EntryPoint>>
function main(): void {
  var_dump(FOO);
  var_dump(BAR);
  var_dump(BAZ);
  var_dump(QUX);
}
