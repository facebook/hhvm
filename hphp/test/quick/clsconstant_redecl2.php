<?hh

interface A {
  const FOO = 'FOO';
}

interface B extends A {
  const BAR = 'BAR';
}

class C implements B {
  const FOO = 'DOH';
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
