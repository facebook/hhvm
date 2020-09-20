<?hh

function foo<T>(): void {
  echo "This function has erased generics";
}

<<__EntryPoint>>
function main(): void {
  $f = foo<int>;
  $f();
}
