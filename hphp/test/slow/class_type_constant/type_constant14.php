<?hh

interface I {
  const type T = int;
}

class C implements I {
  const type T = string;
}

interface I2 extends I {
  const type T = string;
}

class C2 implements I2 {}

<<__EntryPoint>>
function main(): void {
  new C();
  new C2();
  echo "Done.\n";
}
