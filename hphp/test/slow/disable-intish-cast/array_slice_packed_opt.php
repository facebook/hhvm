<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(array_slice(vec[0, 1, 2, 3, 4, 5], 3, 5, /* preserve keys */ false));
  var_dump(array_slice(vec[0, 1, 2, 3, 4, 5], 3, 5, /* preserve keys */ true));
}
