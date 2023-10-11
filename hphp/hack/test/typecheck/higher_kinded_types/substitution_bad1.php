//// file1.php
<?hh

type ID<T> = T;

newtype Apply<TF<TP>, TA> = TF<TA>;


//// file2.php
<?hh

// bad, can't see through definition of Apply here
function test4(Apply<ID,int> $x) : int {
  return $x;
}
