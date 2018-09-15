<?hh

function test_splice($input) {
  var_dump(array_splice(&$input, 1));
  var_dump($input);
}

test_splice(vec[]);
test_splice(vec['foo', 'bar', 'baz', 'baz']);
test_splice(vec['bing']);
