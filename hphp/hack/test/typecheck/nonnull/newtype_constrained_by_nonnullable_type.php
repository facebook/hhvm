//// file1.php
<?hh // strict

newtype t as nonnull = string;

//// file2.php
<?hh // strict

function f(t $x): nonnull {
  return $x;
}
