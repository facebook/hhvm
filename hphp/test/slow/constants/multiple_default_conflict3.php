<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

interface I1 {
  abstract const int X = 3;
}
interface I2 {
  abstract const int X = 4;
}

// error, must redeclare to choose default or as concrete
abstract class A implements I1, I2 {}

<<__EntryPoint>>
function main(): void {
  echo "TODO(T89444033) should be error\n";
}
