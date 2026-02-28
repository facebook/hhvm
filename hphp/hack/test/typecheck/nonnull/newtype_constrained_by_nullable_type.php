//// file1.php
<?hh

newtype t as ?string = string;

//// file2.php
<?hh

function f(t $x): nonnull {
  return $x;
}
