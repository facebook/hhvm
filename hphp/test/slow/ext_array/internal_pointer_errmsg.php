<?hh // strict

function run($input) {
  var_dump(each(inout $input));
  var_dump(current($input));
  var_dump(key($input));
  var_dump(next(inout $input));
  var_dump(prev(inout $input));
  var_dump(reset(inout $input));
  var_dump(end(inout $input));
}

<<__EntryPoint>>
function main() {
  run(varray[1, 2, 3, 4, 5]);
  run(vec[1, 2, 3, 4]);
  run(dict['foo' => 'bar', 'baz' => 'qux', 'zar' => 'doz']);
  run(0);
}
