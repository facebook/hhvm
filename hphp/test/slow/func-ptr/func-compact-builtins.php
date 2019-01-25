<?hh

function foo() {}
function bar() {}

<<__EntryPoint>>
function main() {
  var_dump(array_column(
    [[fun('foo') => 't1'], ['foo' => 't2'], ['bar' => 't3']],
    fun('foo')));
  var_dump(array_count_values([fun('foo'), 'foo', 1]));
  var_dump(array_key_exists(fun('foo'), ['foo' => 1]));
  var_dump(array_key_exists(fun('foo'), [fun('foo') => 1]));
  var_dump(array_replace([fun('foo') => 1], [fun('foo') => 2]));
  var_dump(array_flip([fun('foo') => fun('bar')]));
}
