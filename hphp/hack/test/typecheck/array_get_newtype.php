//// file1.php
<?hh // partial

newtype MyArray as array<mixed, mixed> = array<string, string>;

//// file2.php
<?hh // partial
function f(MyArray $a): MyArray {
  $a['x'] = 4;
  return $a;
}
