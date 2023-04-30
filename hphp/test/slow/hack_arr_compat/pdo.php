<?hh

<<__EntryPoint>>
function main () {
  $db = new PDO('sqlite:'.sys_get_temp_dir().'/foo');
  var_dump($db->query('SELECT 1')->fetch(2));
}
