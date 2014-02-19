<?hh

function asd() { return 12; }
function bsd() { return mt_rand() ? 12.0 : 42.0; }
function lol(double $x) { echo "heh\n"; }

/*
 * This is a case where global dce can change the type of locals
 * across blocks.
 */
function foo() {
  $x = asd(); // This store can be eliminated, but $x :: Int only if
              // it happened.
  for ($i = 0; $i < 10; ++$i) {
    echo $i . "\n";
  }
  $x = bsd($x);
  lol($x);
}

foo();
