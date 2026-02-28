<?hh

function test_splice($input) :mixed{
  var_dump(array_splice(inout $input, 1));
  var_dump($input);
}
<<__EntryPoint>> function main(): void {
test_splice(vec[]);
test_splice(vec['foo', 'bar', 'baz', 'baz']);
test_splice(vec['bing']);
}
