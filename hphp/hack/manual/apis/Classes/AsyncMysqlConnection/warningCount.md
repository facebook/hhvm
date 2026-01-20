
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The number of errors, warnings, and notes returned during execution of
the previous SQL statement




``` Hack
public function warningCount(): int;
```




## Returns




+ ` int ` - The `` int `` count of errors, warnings, etc.




## Examples




The following example shows how to get the number of errors or warnings on the last SQL query via [` AsyncMysqlConnection::warningCount `](/apis/Classes/AsyncMysqlConnection/warningCount/).




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
async function get_warning_count_on_query(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT name FROM test_table WHERE userID = 1');
  $wc = $conn->warningCount();
  $conn->close();
  return $wc;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $wc = await get_warning_count_on_query();
  \var_dump($wc);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(0)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
