<?hh

<<__EntryPoint>>
function main () {
  $db = new PDO('sqlite:/tmp/foo');
  var_dump($db->query('SELECT 1')->fetch(2));
}
