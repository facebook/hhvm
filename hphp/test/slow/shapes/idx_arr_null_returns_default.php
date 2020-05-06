<?hh

<<__EntryPoint>>
function main() {
  foo(darray[]);
  foo(null);
}

function foo(?darray $arr): void {
  var_dump(Shapes::idx($arr, 'x', 'default'));
}
