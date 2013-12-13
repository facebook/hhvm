<?hh

$s1 = Set {
}
;
$s2 = Set {
}
;
var_dump($s1 == $s2);
$s1->add('a');
var_dump($s1 == $s2);
$s2[] = 'b';
var_dump($s1 == $s2);
$s1[] = 'b';
var_dump($s1 == $s2);
$s2->add('a');
var_dump($s1 == $s2);
$s1[] = 'c';
var_dump($s1 == $s2);
echo "============\n";
$s1 = Set {
'a', 'b', 'c', 'd'}
;
$s1->remove('a');
$s1->remove('c');
$s2 = Set {
'b', 'd'}
;
var_dump($s1 == $s2);
$s1->remove('d');
var_dump($s1 == $s2);
$s2->remove('d');
var_dump($s1 == $s2);
$s1->add('d');
var_dump($s1 == $s2);
$s2->add('d');
var_dump($s1 == $s2);
echo "============\n";
$m = Set {
}
;
var_dump($m == null);
var_dump($m == false);
var_dump($m == true);
var_dump($m == 1);
var_dump($m == "Set");
echo "============\n";
$m = Set {
7}
;
var_dump($m == null);
var_dump($m == false);
var_dump($m == true);
var_dump($m == 1);
var_dump($m == "Set");

