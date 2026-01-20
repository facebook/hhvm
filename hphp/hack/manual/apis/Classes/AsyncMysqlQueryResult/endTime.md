
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The end time when the successful query began, in seconds since epoch




``` Hack
public function endTime(): float;
```




## Returns




+ ` float ` - the end time as `` float `` seconds since epoch.




## Examples




Every successful query has a result. And one of the methods on an [` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/) is [` endTime() `](/apis/Classes/AsyncMysqlQueryResult/endTime/), which tells you the time when we finally got our result.




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
  // What time was it when we finally got this result?
  \var_dump($result->endTime());
  $conn->close();
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
float(%f)
int(%d)
```.example.hhvm.out
float(17309.077334)
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
