<?hh

$v1 = Vector {
5}
;
$v2 = Vector {
5}
;
var_dump($v1 === $v2);
var_dump($v1 == $v2);
$v1[] = "123";
$v2[] = 123;
var_dump($v1 == $v2);
$v1[] = 73;
var_dump($v1 == $v2);
$v2[] = 74;
var_dump($v1 == $v2);
echo "------------------------\n";
$v = Vector {
}
;
var_dump($v == null);
var_dump($v == false);
var_dump($v == true);
var_dump($v == 1);
var_dump($v == "Vector");
echo "------------------------\n";
$v = Vector {
7}
;
var_dump($v == null);
var_dump($v == false);
var_dump($v == true);
var_dump($v == 1);
var_dump($v == "Vector");
