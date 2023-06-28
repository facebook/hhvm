<?hh

function foo() :mixed{}
function bar() :mixed{}

function W($f) :mixed{
  try {
    var_dump($f());
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  W(() ==> array_column(varray[darray[foo<> => 't1'], darray['foo' => 't2'], darray['bar' => 't3']], foo<>));
  W(() ==> array_count_values(varray[foo<>, 'foo', 1]));
  W(() ==> array_key_exists(foo<>, darray['foo' => 1]));
  W(() ==> array_key_exists(foo<>, darray[foo<> => 1]));
  W(() ==> array_replace(darray[foo<> => 1], darray[foo<> => 2]));
  W(() ==> array_flip(darray[foo<> => bar<>]));
}
