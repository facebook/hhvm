<?hh

<<__EntryPoint>>
function main() :mixed{
  foo(dict[]);
  foo(null);
}

function foo(?darray $arr): void {
  var_dump(Shapes::idx($arr, 'x', 'default'));
}
