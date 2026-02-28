
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The total time for the successful query to occur, in microseconds




``` Hack
public function elapsedMicros(): int;
```




## Returns




+ ` int ` - the total successful result producing time as `` int `` microseconds.




## Examples




Every successful query has a result. And one of the methods on an [` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/) is [` elapsedMicros() `](/apis/Classes/AsyncMysqlQueryResult/elapsedMicros/), which tells you how long it took to perform the query and get the result.




Note that




```
  elapsedMicros() ~== endTime() - startTime()
```




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
  $result = await $conn->query('SELECT name FROM test_table WHERE userID = 1');
  // How long did it take to get this result?
  \var_dump($result->elapsedMicros());
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
int(286)
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
