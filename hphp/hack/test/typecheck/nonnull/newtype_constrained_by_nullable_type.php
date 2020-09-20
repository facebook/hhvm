//// file1.php
<?hh // strict

newtype t as ?string = string;

//// file2.php
<?hh // strict

function f(t $x): nonnull {
  return $x;
}
