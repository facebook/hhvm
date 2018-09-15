<?hh
$tmp_sqllite = tempnam(sys_get_temp_dir(), 'vmpdotest');
$source = "sqlite:$tmp_sqllite";
$db = new PDO($source);
$rows = $db->query('SELECT LENGTH("123456") as col;')->fetchObject();
var_dump($rows);
