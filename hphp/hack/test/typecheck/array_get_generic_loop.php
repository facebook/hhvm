<?hh // strict

function f<T1 as array<int> as T2, T2 as T1> (T1 $x): void {
  $x[5]=5;
}

function g<T1 as array<int> as string>(T1 $x) : void {
  $x[4] = 3;
}
