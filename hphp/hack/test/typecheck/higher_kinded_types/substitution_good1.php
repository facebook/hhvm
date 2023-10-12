//// file1.php
<?hh // strict

type ID<T> = T;

type Apply1<TF<TP>, TA> = TF<TA>;

newtype Apply2<TF<TP>, TA> as TF<TA> = TF<TA>;

newtype Apply3<TF<TP>, TA> = TF<TA>;

function test1(Apply1<ID,int> $x) : int {
  return $x + $x;
}

function test2(Apply2<ID,int> $x) : int {
  return $x + $x;
}

function test3(Apply3<ID,int> $x) : int {
  return $x + $x;
}

//// file2.php
<?hh // strict

function test4(Apply2<ID,int> $x) : int {
  return $x + $x;
}
