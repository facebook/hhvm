
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get a certain row in the current row block




``` Hack
public function getRow(
  int $row,
): AsyncMysqlRow;
```




## Parameters




+ ` int $row ` - the row index.




## Returns




* [` AsyncMysqlRow `](/apis/Classes/AsyncMysqlRow/) - The [` AsyncMysqlRow `](/apis/Classes/AsyncMysqlRow/) representing one specific row in the current
  row block.




## Examples




The following example shows you how to get a row from a row block via [` AsyncMysqlRowBlock::getRow `](/apis/Classes/AsyncMysqlRowBlock/getRow/). You pass an `` int `` specifying the row of the row block you want.




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
async function simple_query(): Awaitable<string> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT * FROM test_table WHERE userID < 50');
  $conn->close();
  // A call to $result->rowBlocks() actually pops the first element of the
  // row block Vector. So the call actually mutates the Vector.
  $row_blocks = $result->rowBlocks();
  if ($row_blocks->count() > 0) {
    // An AsyncMysqlRowBlock
    $row_block = $row_blocks[0];
    // The next two lines are similar to $row_block->getFieldAsString(0, 'name')
    $row = $row_block->getRow(0); // An AsyncMysqlRow
    return $row->getFieldAsString('name'); // string
  } else {
    return "nothing";
  }
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
string(%d) "%s"
```.example.hhvm.out
string(3) "Jan"
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
