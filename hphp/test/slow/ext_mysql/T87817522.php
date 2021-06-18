<?hh

function test_mysql_ssl_connect() {
  $conn = mysql_connect_with_ssl("a", "b", "c", "d", 0, 0, 0, new stdClass, dict[]);
}

<<__EntryPoint>>
function main(): void {
  $tests = vec[
    fun('test_mysql_ssl_connect'),
  ];
  foreach ($tests as $t) {
    try {
      $t();
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}
