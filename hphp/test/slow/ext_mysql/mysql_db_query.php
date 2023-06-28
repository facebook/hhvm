<?hh

<<__EntryPoint>>
function main_mysql_db_query() :mixed{
try {
  mysql_db_query('', '');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
}
