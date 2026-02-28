
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reset the iterator to the first row




``` Hack
public function rewind(): void;
```




## Returns




+ ` void `




## Examples




The following example shows how to use [` AsyncMysqlRowBlockIterator::rewind `](/apis/Classes/AsyncMysqlRowBlockIterator/rewind/) to get back to the beginning of the iterator as necessary.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

async function connect(
  \AsyncMysqlConnectionPool $pool,
): Awaitable<\AsyncMysqlConnection> {
  return await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
}
async function iterate(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT * FROM test_table WHERE userID < 50');
  $conn->close();
  $ret = -1;
  // A call to $result->rowBlocks() actually pops the first element of the
  // row block Vector. So the call actually mutates the Vector.
  $row_blocks = $result->rowBlocks();
  if ($row_blocks->count() > 0) {
    // An AsyncMysqlRowBlock
    $row_block = $row_blocks[0];
    // An AsyncMysqlRowBlockIterator
    $rbit = $row_block->getIterator();
    // Iterating through a row block iterator will have an int key and
    // an AsyncMysqlRow as its value
    while ($rbit->valid()) {
      // current() gets you an AsyncMysqlRow
      $row = $rbit->current();
      // getIterator() gets you an AsyncmysqlRowIterator
      $rit = $row->getIterator();
      while ($rit->valid()) {
        // current() will give you a string value of the field in the row
        if ($rit->key() > 0 && \is_numeric($rit->current())) {
          $ret = \intval($rit->current());
          break;
        }
        $rit->next(); // move to the next field
      }
      if ($ret > -1) {
        break;
      }
      $rbit->next(); // move to the next AsyncMysqlRow
    }

    if ($ret > -1) {
      $rbit->rewind(); // Go back to the beginning of AsyncMysqlBlockIterator
      while ($rbit->valid()) {
        if ($rbit->current() is \AsyncMysqlRow) {
          $ret++;
        }
        $rbit->next();
      }
    }
    return $ret;
  } else {
    return -1;
  }
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await iterate();
  \var_dump($r);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(43)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
