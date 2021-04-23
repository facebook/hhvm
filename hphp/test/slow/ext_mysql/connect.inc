<?hh

function create_test_table(string $db, string $suffix = ''): bool {
  $name = "test_$suffix";
  mysql_select_db($db);
  mysql_query("drop table $name");
  $success = (bool)mysql_query(
    "create table $name (id int not null auto_increment,".
    " name varchar(255) not null, primary key (id)) ".
    "engine=innodb"
  );
  if (!$success) {
    var_dump(mysql_error());
  }
  return $success;
}

/*
Default values are "localhost", "root", database "test" and empty password.
Change the MYSQL_TEST_* environment values if you want to use another configuration.
*/
function connection_settings(): (string, string, string, string) {
  $host   = getenv("MYSQL_TEST_HOST") ?: "localhost";
  $user   = getenv("MYSQL_TEST_USER") ?: "root";
  $passwd = getenv("MYSQL_TEST_PASSWD") ?: "";
  $db     = getenv("MYSQL_TEST_DB") ?: "test";

  $socket = getenv("MYSQL_TEST_SOCKET") ?: null;
  if ($socket) {
    ini_set('mysql.default_socket', $socket);
  }

  return tuple($host, $user, $passwd, $db);
}
