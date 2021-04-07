<?hh

function pure()[] {}

class C {
  function f<reify T>(mixed $x, mixed $y, ...$z)[rx, ctx $x, ctx $y] {
    echo "ok\n";
  }
}

<<__EntryPoint>>
function main()[] {
  (new C)->f<int>(pure<>, pure<>);
}
