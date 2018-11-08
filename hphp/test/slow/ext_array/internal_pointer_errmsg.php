<?hh // strict

function run($input) {
  var_dump(each(&$input));
  var_dump(current(&$input));
  var_dump(key(&$input));
  var_dump(next(&$input));
  var_dump(prev(&$input));
  var_dump(reset(&$input));
  var_dump(end(&$input));
}

<<__EntryPoint>>
function main() {
  run([1, 2, 3, 4, 5]);
  run(vec[1, 2, 3, 4]);
  run(dict['foo' => 'bar', 'baz' => 'qux', 'zar' => 'doz']);
  run(0);
}
