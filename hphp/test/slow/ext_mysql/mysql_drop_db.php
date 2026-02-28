<?hh

<<__EntryPoint>>
function main_mysql_drop_db() :mixed{
try {
  mysql_drop_db('');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
}
