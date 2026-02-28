<?hh

class Something {
  const ctx C = [];
}

function f(
  (function()[_]: void) $x1,
  (function()[_]: void) $x2,
)[rx, ctx $x1, ctx $x2] :mixed{}

function g(Something $x1, Something $x2)[$x1::C, $x2::C] :mixed{}

class C {
public function f(
    Something $x1,
    (function()[_]: void) $x2,
)[$x1::C, ctx $x2, this::C, IO] :mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  echo "ok\n";
}
