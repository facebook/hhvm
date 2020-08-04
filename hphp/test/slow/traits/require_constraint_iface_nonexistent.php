<?hh

interface I1 {
  require extends NonExistent;
}

class X implements I1 {
}

<<__EntryPoint>>
function main(): void {
  $x = new X();
}
