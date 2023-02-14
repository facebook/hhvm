<?hh

function test_connect() {
  AsyncMysqlClient::connect("a", 123, "db", "a", "a", 0, new stdClass, 0, "x");
}

function test_connect_and_query() {
   AsyncMysqlClient::connectAndQuery(darray(vec[]), "x",1,"x","y","z", new stdClass, dict[]);
}

function test_connect_with_opts() {
  AsyncMysqlClient::connectWithOpts("a",1,"a","a","a", new stdClass);
}

function test_pool_connect_with_opts() {
  $a = new AsyncMysqlConnectionPool(dict(vec[]));
  $a->connectWithOpts("a", 123, "x","y","z", new stdClass, "extra");
}

<<__EntryPoint>>
function main(): void {
  $tests = vec[
      test_connect<>,
      test_connect_and_query<>,
      test_connect_with_opts<>,
      test_pool_connect_with_opts<>
  ];
  foreach ($tests as $t) {
    try {
      $t();
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}
