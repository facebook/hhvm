<?hh

<<__EntryPoint>> function main(): void {
  $tmp_sqlite = sys_get_temp_dir().'/'.'vmpdotest';
  $source = "sqlite:$tmp_sqlite";
  $db = new PDO($source);
  $rows = $db->query('SELECT LENGTH("123456") as col;')->fetchObject();
  var_dump($rows);
}
