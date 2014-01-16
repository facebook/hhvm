<?hh

$v = new Vector;
$v[] = 7;
$v[] = 'foo';
var_dump(serialize($v));
$v2 = unserialize(serialize($v));
echo "------------------------\n";
var_dump($v);
print_r($v);
echo json_encode($v) . "\n";
var_export($v);
 echo "\n";
var_dump($v2);
print_r($v2);
echo json_encode($v2) . "\n";
var_export($v2);
 echo "\n";
