
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) representing all row blocks returned by the successful
query




``` Hack
public function rowBlocks(): Vector<AsyncMysqlRowBlock>;
```




A row block can be the full result of the query (if there is only one
row block), or it can be the partial result of the query (if there are
more than one row block). The total number of row blocks makes up the
entire result of the successful query.




Usually, there will be only one row block in the vector because the
query completed in full in one attempt. However, if, for example, the
query represented something that exceeded some network parameter, the
result could come back in multiple blocks.




## Returns




+ [` Vector<AsyncMysqlRowBlock> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) of [` AsyncMysqlRowBlock `](/apis/Classes/AsyncMysqlRowBlock/) objects, the total number
  of which represent the full result of the query.




## Examples




The following example shows how a call to [` AsyncMysqlQueryResult::rowBlocks `](/apis/Classes/AsyncMysqlQueryResult/rowBlocks/) gets you a [` Vector `](/apis/Classes/HH/Vector/) of [` AsyncMysqlRowBlock `](/apis/Classes/AsyncMysqlRowBlock/) objects. Each object can then be queried for statistical data on that row, such as the number of fields that came back with the result, etc.




**NOTE**: A call to [` rowBlocks() `](/apis/Classes/AsyncMysqlQueryResult/rowBlocks/) actually pops the first element of that [` Vector `](/apis/Classes/HH/Vector/). So, for example, if you have the following:




```
object(HH\Vector)#9 (1) {
  [0]=>
  object(AsyncMysqlRowBlock)#10 (0) {
  }
}
```




a call to [` rowBlocks() `](/apis/Classes/AsyncMysqlQueryResult/rowBlocks/) will make it so that you know have:




```
object(HH\Vector)#9 (0) {
}
```




and thus a subsequent call to [` rowBlocks() `](/apis/Classes/AsyncMysqlQueryResult/rowBlocks/) will return an empty [` Vector `](/apis/Classes/HH/Vector/).




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
async function simple_query(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT * FROM test_table WHERE userID < 50');
  // A call to $result->rowBlocks() actually pops the first element of the
  // row block Vector. So it mutates it.
  $row_blocks = $result->rowBlocks();
  if ($row_blocks->count() > 0) {
    // An AsyncMysqlRowBlock
    $row_block = $row_blocks[0];
    \var_dump($row_block->fieldName(2)); // string
  } else {
    \var_dump('nothing');
  }
  $conn->close();
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
string(%d) "%s"
int(%d)
```.example.hhvm.out
string(3) "age"
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
