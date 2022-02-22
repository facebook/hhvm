<?hh

function expect_kc(KeyedContainer<string, Stringish> $s) : void {}

function expect_stringish(Stringish $s) : void {}

function f(dict<string, dict<arraykey, string>> $d) : void {
  $x = \HH\Lib\Dict\flatten($d)
    |> \HH\Lib\Dict\map_keys($$, ($key) ==> $key as string);
  expect_kc(\HH\Lib\Dict\flip($x));
}

