<?hh

function foo() {}
function bar() {}

function W($f) {
  try {
    var_dump($f());
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main() {
  W(() ==> array_column(varray[darray[fun('foo') => 't1'], darray['foo' => 't2'], darray['bar' => 't3']], fun('foo')));
  W(() ==> array_count_values(varray[fun('foo'), 'foo', 1]));
  W(() ==> array_key_exists(fun('foo'), darray['foo' => 1]));
  W(() ==> array_key_exists(fun('foo'), darray[fun('foo') => 1]));
  W(() ==> array_replace(darray[fun('foo') => 1], darray[fun('foo') => 2]));
  W(() ==> array_flip(darray[fun('foo') => fun('bar')]));
}
