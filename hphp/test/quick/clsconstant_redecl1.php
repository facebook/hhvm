<?hh

interface A {
  const FOO = 'FOO';
}

class B implements A {
  const FOO = 'BAR';
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
