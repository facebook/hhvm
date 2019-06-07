<?hh

<<__EntryPoint>>
function main_mysql_drop_db() {
try {
  mysql_drop_db('');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
}
