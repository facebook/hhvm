<?hh

function test_splice($input) {
  var_dump(array_splice(&$input, 1));
  var_dump($input);
}

test_splice(dict[]);
test_splice(dict['foo' => 0, 'bar' => 1, 'baz' => 2, 'baz' => 3]);
test_splice(dict['bing' => 42]);
