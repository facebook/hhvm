
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Average loop time of the MySQL client event, in microseconds




``` Hack
public function ioEventLoopMicrosAvg(): float;
```




An event can include a connection, an error condition, a query, etc.




This returns an exponentially-smoothed average.




## Returns




+ ` float ` - A `` float `` representing the average for an event to happen on this
  MySQL client.




## Examples




The following example describes how to get the average loop time of this SQL client's event handling (in this particular case, performing the connection) via [` AsyncMysqlClientStats::ioEventLoopMicrosAvg `](/apis/Classes/AsyncMysqlClientStats/ioEventLoopMicrosAvg/).




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
async function get_loop_info(): Awaitable<?float> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $loop = $conn->connectResult()?->clientStats()?->ioEventLoopMicrosAvg();
  $conn->close();
  return $loop;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $l = await get_loop_info();
  \var_dump($l);
}
```.hhvm.expectf
float(%f)
```.example.hhvm.out
float(3.536136)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
