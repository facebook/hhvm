//// file1.php
<?hh

newtype MyArray as array<mixed, mixed> = array<string, string>;

//// file2.php
<?hh
function f(MyArray $a): MyArray {
  $a['x'] = 4;
  return $a;
}
