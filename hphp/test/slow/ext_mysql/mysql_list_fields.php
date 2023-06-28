<?hh

<<__EntryPoint>>
function main_mysql_list_fields() :mixed{
try {
  mysql_list_fields('', '');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
}
