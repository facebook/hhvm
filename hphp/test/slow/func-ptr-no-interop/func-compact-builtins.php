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
  W(() ==> array_column(vec[dict[foo<> => 't1'], dict['foo' => 't2'], dict['bar' => 't3']], foo<>));
  W(() ==> array_count_values(vec[foo<>, 'foo', 1]));
  W(() ==> array_key_exists(foo<>, dict['foo' => 1]));
  W(() ==> array_key_exists(foo<>, dict[foo<> => 1]));
  W(() ==> array_replace(dict[foo<> => 1], dict[foo<> => 2]));
  W(() ==> array_flip(dict[foo<> => bar<>]));
}
