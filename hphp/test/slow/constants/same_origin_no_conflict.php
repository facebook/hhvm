<?hh

interface IOrigin {
  abstract const type T = int;
}

interface I1 extends IOrigin {}
interface I2 extends IOrigin {}

abstract class A implements I1, I2 {}

<<__EntryPoint>>
function main(): void {
  echo "Ok\n";
}
