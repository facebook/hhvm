<?hh
<<__EntryPoint>> function main(): void {
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

// --------------------------------
// Connection string tests.
// --------------------------------

// Inline port.
try {
  $dbh = new PDO(
    'mysql:host='.$host.':'.$port.';'.
    'dbname='.$db,
    $user,
    $passwd
  );
  var_dump(true);
} catch (Exception $ex) { }

// Separate port.
try {
  $dbh = new PDO(
    'mysql:host='.$host.';'.
    'port='.$port.';'.
    'dbname='.$db,
    $user,
    $passwd
  );
  var_dump(true);
} catch (Exception $ex) { }

try {
  // Mixed, explicit override.
  $dbh = new PDO(
    'mysql:host='.$host.':'.$port.';'.
    'port='.$port.';'.
    'dbname='.$db,
    $user,
    $passwd
  );
  var_dump(true);
} catch (Exception $ex) { }

try {
  // IPv6 with port.
  $dbh = new PDO(
    'mysql:host=[::1]:'.$port.';'.
    'dbname='.$db,
    $user,
    $passwd
  );
  var_dump(true);
} catch (Exception $ex) { }

try {
  // IPv6 with override port.
  $dbh = new PDO(
    'mysql:host=[::1]:'.$port.';'.
    'port='.$port.';'.
    'dbname='.$db,
    $user,
    $passwd
  );
  var_dump(true);
} catch (Exception $ex) { }

try {
  // IPv6 explicit port.
  $dbh = new PDO(
    'mysql:host=[::1];'.
    'port='.$port.';'.
    'dbname='.$db,
    $user,
    $passwd
  );
  var_dump(true);
} catch (Exception $ex) { }
}
