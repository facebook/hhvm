//// file1.php
<?hh // strict

newtype alias_keyset_int as keyset<int> = keyset<int>;

//// file2.php
<?hh // strict

function foo(alias_keyset_int $k) : void {
  unset($k[42]);
}
