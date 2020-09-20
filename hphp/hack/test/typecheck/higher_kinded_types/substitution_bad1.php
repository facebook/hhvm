//// file1.php
<?hh // strict

type ID<T> = T;

newtype Apply<TF<TP>, TA> = TF<TA>;


//// file2.php
<?hh // strict

// bad, can't see through definition of Apply here
function test4(Apply<ID,int> $x) : int {
  return $x;
}
