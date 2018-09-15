<?hh


<<__EntryPoint>>
function main_connect_and_query() {
try {
  $options = new AsyncMysqlConnectionOptions();
  $options->setTotalTimeout(10);
  $options->setQueryTimeout(10);
  HH\Asio\join(
    AsyncMysqlClient::connectAndQuery(
      Vector{"SELECT id FROM `lolwhut` WHERE id IN (1)"},
      "127.0.0.1",
      80,
      "fakedb",
      "fakeuser",
      "",
      $options
    )
  );
  echo "connectAndQuery unexpectedly succeeded\n";
} catch (Exception $e) {
  echo "connectAndQuery threw an exception\n";
}
// walk the heap to make sure we haven't corrupted it
heapgraph_create();
}
