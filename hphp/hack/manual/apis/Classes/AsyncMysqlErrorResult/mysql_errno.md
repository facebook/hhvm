
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the MySQL error number for this result




``` Hack
public function mysql_errno(): int;
```




See MySQL's
[mysql_errno()](<http://dev.mysql.com/doc/refman/5.0/en/mysql-errno.html>)
for information on the error numbers.




## Returns




+ ` int ` - The error number as an `` int ``.




## Examples




When an error occurs when establishing a connection or on a query, and you catch the exception that is thrown, you will get an [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/). And one of the methods on an [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) is [` mysql_errno() `](/apis/Classes/AsyncMysqlErrorResult/mysql_errno/), which gives you the MySQL error number. In this case, the error number is 1146, which represents a table not existing.




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
async function simple_query_error(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  try {
    $result = await $conn->query('SELECT bogus FROM bogus WHERE bogus = 1');
  } catch (\AsyncMysqlQueryException $ex) {
    $qr = $ex->getResult();
    // Actually `AsyncMysqlQueryErrorResult`
    \var_dump($qr is \AsyncMysqlErrorResult);
    // Error should be 1146 .. Table doesn't exist
    \var_dump($qr->mysql_errno());
    $conn->close();
    return 0;
  }
  $conn->close();
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query_error();
  \var_dump($r);
}
```.hhvm.expectf
bool(true)
int(1146)
int(%d)
```.example.hhvm.out
bool(true)
int(1146)
int(0)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
