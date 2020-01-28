<?hh

function foo() {}
function bar() {}

<<__EntryPoint>>
function main() {
  var_dump(array_column(
    varray[darray[fun('foo') => 't1'], darray['foo' => 't2'], darray['bar' => 't3']],
    fun('foo')));
  var_dump(array_count_values(varray[fun('foo'), 'foo', 1]));
  var_dump(array_key_exists(fun('foo'), darray['foo' => 1]));
  var_dump(array_key_exists(fun('foo'), darray[fun('foo') => 1]));
  var_dump(array_replace(darray[fun('foo') => 1], darray[fun('foo') => 2]));
  var_dump(array_flip(darray[fun('foo') => fun('bar')]));
}
