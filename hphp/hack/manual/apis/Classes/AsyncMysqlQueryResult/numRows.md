
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The number of rows in the current result




``` Hack
public function numRows(): int;
```




This is particularly useful for ` SELECT ` statements.




This is complementary to [` numRowsAffected() `](/apis/Classes/AsyncMysqlQueryResult/numRowsAffected/) as they might be the same
value, but if this was an `` INSERT `` query, for example, then this might be
0, while [` numRowsAffected() `](/apis/Classes/AsyncMysqlQueryResult/numRowsAffected/) could be non-zero.




See the MySQL's [mysql_num_rows()](<http://goo.gl/Rv5NaL>) documentation for
more information.




## Returns




+ ` int ` - The number of rows in the current result as an `` int ``.




## Examples




This example shows how to determine the number of rows returned from a given query using [` AsyncMysqlQueryResult::numRows `](/apis/Classes/AsyncMysqlQueryResult/numRows/).




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
  $result = await $conn->query('SELECT name FROM test_table WHERE userID < 50');
  $conn->close();
  // How many rows did this query return?
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
