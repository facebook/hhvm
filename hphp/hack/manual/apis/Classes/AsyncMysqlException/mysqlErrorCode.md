
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the MySQL error number for that caused the current exception




``` Hack
public function mysqlErrorCode(): int;
```




See MySQL's
[mysql_errno()](<http://dev.mysql.com/doc/refman/5.0/en/mysql-errno.html>)
for information on the error numbers.




## Returns




+ ` int ` - The error number as an `` int ``.




## Examples




The following example shows how to use [` AsyncMysqlException::mysqlErrorCode `](/apis/Classes/AsyncMysqlException/mysqlErrorCode/) to get the raw MySQL error code associated with this exception. The most likely error code for this example will be `` 1045 `` for access denied.




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
    "thisIsNotThePassword",
  );
}
async function simple_query(): Awaitable<?string> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = null;
  $ret = null;
  try {
    $conn = await connect($pool);
    $result = await $conn->query(
      'SELECT name FROM test_table WHERE userID = 1',
    );
    $conn->close();
    return $result->vectorRows()[0][0];
  } catch (\AsyncMysqlConnectException $ex) { // implicitly constructed
    $ret = "Connection Exception";
    \var_dump($ex->mysqlErrorCode());
  } catch (\AsyncMysqlQueryException $ex) { // implicitly constructed
    $ret = "Query Exception";
  } catch (\AsyncMysqlException $ex) {
    $ret = null;
  } finally {
    if ($conn) {
      $conn->close();
    }
  }
  return $ret;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
int(%d)
string(20) "Connection Exception"
```.example.hhvm.out
int(1045)
string(20) "Connection Exception"
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
