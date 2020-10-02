//// file1.php
<?hh // partial

newtype MyArray as darray<mixed, mixed> = darray<string, string>;

//// file2.php
<?hh // partial
function f(MyArray $a): MyArray {
  $a['x'] = 4;
  return $a;
}
