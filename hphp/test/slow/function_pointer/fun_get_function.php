<?hh

function rfunc<reify T>(): void {}
function rfunc2<reify T, Ta>(): void {}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\fun_get_function(rfunc<int>));
  var_dump(HH\fun_get_function(rfunc2<int, _>));
  var_dump(HH\fun_get_function(rfunc2<int, string>));
}
