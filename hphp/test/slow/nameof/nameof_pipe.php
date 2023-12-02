<?hh

function s(string $s): void { var_dump($s); }

<<__EntryPoint>>
function f(): void {
  nameof C |> s($$);
}
