<?hh

function test_splice($input) :mixed{
  var_dump(array_splice(inout $input, 1));
  var_dump($input);
}
<<__EntryPoint>> function main(): void {
test_splice(dict[]);
test_splice(dict['foo' => 0, 'bar' => 1, 'baz' => 2, 'baz' => 3]);
test_splice(dict['bing' => 42]);
}
