
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The last ID inserted, if one existed, for the query that produced the
current result




``` Hack
public function lastInsertId(): int;
```




See the MySQL's [mysql_insert_id()](<http://goo.gl/qxIcPz>) documentation for
more information.




## Returns




+ ` int ` - The last insert id, or 0 if none existed.




## Examples




This example shows how to use the [` AsyncMysqlQueryResult::lastInsertId `](/apis/Classes/AsyncMysqlQueryResult/lastInsertId/) method to get the last primary id inserted into a table, if one exists. This will return `` 0 `` if your query did not actually insert an id, for example in a ``` SELECT ``` statement.




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
  $id = \rand(100, 60000); // userID is a SMALLINT
  $name = \str_shuffle("ABCDEFGHIJ");
  $query = 'INSERT INTO test_table (userID, name) VALUES ('.
    $id.
    ', "'.
    $name.
    '")';
  try {
    $result = await $conn->query($query);
    // What was the last primary id we inserted into the table?
    \var_dump($result->lastInsertId());
  } catch (\AsyncMysqlQueryException $ex) {
    // this could happen if we try to insert duplicate user id
    // But to keep test output consistent, just var dump a positive number
    \var_dump(\PHP_INT_MAX);
    $conn->close();
    return 0;
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
int(%d)
int(%d)
```.example.hhvm.out
int(21301)
int(0)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
