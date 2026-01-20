
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the iterator over the fields in the current row




``` Hack
public function getIterator(): KeyedIterator<string, mixed>;
```




## Returns




+ [` KeyedIterator<string, `](/apis/Interfaces/HH/KeyedIterator/)`` mixed> `` - An [` AsyncMysqlRowIterator `](/apis/Classes/AsyncMysqlRowIterator/) to iterate over the current row.




## Examples




The following example shows you how to get an iterator of an [` AsyncMysqlRow `](/apis/Classes/AsyncMysqlRow/) via [` getIterator() `](/apis/Classes/AsyncMysqlRow/getIterator/). Getting an iterator of an [` AsyncMysqlRow `](/apis/Classes/AsyncMysqlRow/) gives you an [` AsyncMysqlRowIterator `](/apis/Classes/AsyncMysqlRowIterator/), where each key of that iterator is an `` int `` representing the key to the field of the [` AsyncMysqlRow `](/apis/Classes/AsyncMysqlRow/), and each value from `` current() `` is the value of the field of that [` AsyncMysqlRow `](/apis/Classes/AsyncMysqlRow/).




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
  // A call to $result->rowBlocks() actually pops the first element of the
  // row block Vector. So the call actually mutates the Vector.
  $row_blocks = $result->rowBlocks();
  if (!$row_blocks->isEmpty()) {
    // An AsyncMysqlRowBlock
    $row_block = $row_blocks[0];
    if (!$row_block->isEmpty()) {
      // An AsyncMysqlRow
      $row = $row_block->getRow(0);
      // An AsyncMysqlRowIterator
      $it = $row->getIterator();
      while ($it->valid()) {
        // current() will give you a string value of the field in the row
        if ($it->key() > 0 && \is_numeric($it->current())) {
          return \intval($it->current());
        }
        $it->next();
      }
    }
    return -1;
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
int(42)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
