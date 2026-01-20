
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an alternative, normalized version of the error message provided by
mysql_error()




``` Hack
public function mysql_normalize_error(): string;
```




Sometimes the message is the same, depending on if there was an explicit
normalized string provided by the MySQL client.




## Returns




+ ` string ` - The normalized error string.




## Examples




When an error occurs when establishing a connection or on a query, and you catch the exception that is thrown, you will get an [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/). And one of the methods on an [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) is [` mysql_normalize_error() `](/apis/Classes/AsyncMysqlErrorResult/mysql_normalize_error/), which gives you possibly alternative, normalized version of the error provided by [` mysql_error() `](/apis/Classes/AsyncMysqlErrorResult/mysql_error/). Many times they are the same; it depends on how the client provides the error messages. In this case, the two errors are the same; the error string is letting us know that the `` bogus `` table does not exist.




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
    // Error should be "Table doesn't exist...""
    \var_dump($qr->mysql_normalize_error());
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
string(34) "Table 'testdb.bogus' doesn't exist"
int(%d)
```.example.hhvm.out
bool(true)
string(34) "Table 'testdb.bogus' doesn't exist"
int(0)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
