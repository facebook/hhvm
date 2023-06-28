<?hh

<<__EntryPoint>>
function main() :mixed{
  foo(darray[]);
  foo(null);
}

function foo(?darray $arr): void {
  var_dump(Shapes::idx($arr, 'x', 'default'));
}
