//// file1.php
<?hh

newtype t as nonnull = string;

//// file2.php
<?hh

function f(t $x): nonnull {
  return $x;
}
