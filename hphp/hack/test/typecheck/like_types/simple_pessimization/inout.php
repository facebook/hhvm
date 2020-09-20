<?hh

function f<T>(inout T $i): void {}

function g(): void {
  $i = 3;
  f<int>(inout $i);
  hh_show($i);
}
