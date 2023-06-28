<?hh

function g<reify T>() :mixed{
  echo "done\n";
}

function f<reify T>() :mixed{
  $g = g<T>;
  $g();
}
<<__EntryPoint>> function main(): void {
  $f = f<int>;
  $f();
}
