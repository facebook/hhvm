<?hh

function pure()[] :mixed{}

class C {
  function f<reify T>(mixed $x, mixed $y, ...$z)[rx, ctx $x, ctx $y] :mixed{
    echo "ok\n";
  }
}

<<__EntryPoint>>
function main()[] :mixed{
  (new C)->f<int>(pure<>, pure<>);
}
