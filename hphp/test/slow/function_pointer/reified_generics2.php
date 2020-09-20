<?hh

function g<reify T>() {
  echo "done\n";
}

function f<reify T>() {
  $g = g<T>;
  $g();
}
<<__EntryPoint>> function main(): void {
  $f = f<int>;
  $f();
}
