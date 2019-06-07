<?hh

<<__EntryPoint>>
function main_mysql_create_db() {
try {
  mysql_create_db('');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
}
