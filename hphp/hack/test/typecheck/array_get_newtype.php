//// file1.php
<?hh

newtype MyArray as darray<mixed, mixed> = darray<string, string>;

//// file2.php
<?hh
function f(MyArray $a): MyArray {
  $a['x'] = 4;
  return $a;
}
