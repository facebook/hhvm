
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the underlying [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) associated with the current
exception




``` Hack
public function getResult(): AsyncMysqlErrorResult;
```




## Returns




+ [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) - The [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) that underlies the current exception.




## Examples




The following example shows how to use [` AsyncMysqlException::getResult `](/apis/Classes/AsyncMysqlException/getResult/) to get the [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) object associated with this exception. Particularly, we call `` elapsedMicros() `` on that object.




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
    \var_dump($ex->getResult()->elapsedMicros());
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
int(4710)
string(20) "Connection Exception"
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
