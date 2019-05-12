<?hh

function f(?int $x): void {
  if ($x <> null) {
    echo $x;
  }
}
<<__EntryPoint>> function main(): void {
f(1);
}
