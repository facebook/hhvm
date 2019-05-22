<?hh

<<__EntryPoint>>
function main(): void {
  $d = shape('str' => 3);
  f<shape('str' => _)>($d);
}

function f<reify T>($a): void {
  $a as T;
}
