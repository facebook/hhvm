//// file1.php
<?hh

case type A = shape('a' => ?B);

//// file2.php
<?hh

newtype B = shape('b' => A);
