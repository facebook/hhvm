<?hh

function asd() :mixed{ return 12; }
function bsd() :mixed{ return mt_rand() ? 12.0 : 42.0; }
function lol(float $x) :mixed{ echo "heh\n"; }

/*
 * This is a case where global dce can change the type of locals
 * across blocks.
 */
function foo() :mixed{
  $x = asd(); // This store can be eliminated, but $x :: Int only if
              // it happened.
  for ($i = 0; $i < 10; ++$i) {
    echo $i . "\n";
  }
  $x = bsd($x);
  lol($x);
}


<<__EntryPoint>>
function main_dce_001() :mixed{
foo();
}
