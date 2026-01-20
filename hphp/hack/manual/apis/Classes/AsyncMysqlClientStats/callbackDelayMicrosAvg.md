
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Average delay between when a callback is scheduled in the MySQL client
and when it's actually ran, in microseconds




``` Hack
public function callbackDelayMicrosAvg(): float;
```




The callback can be from creating a connection, inducing an error
condition, executing a query, etc.




This returns an exponentially-smoothed average.




## Returns




+ ` float ` - A `` float `` representing the average callback delay on this
  MySQL client.




## Examples




The following example describes how to get the average delay time between when a callback is scheduled (in this case, performing the connection) and when the callback actual ran (in this case, when the connection was actually established) via [` AsyncMysqlClientStats::callbackDelayMicrosAvg `](/apis/Classes/AsyncMysqlClientStats/callbackDelayMicrosAvg/).




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
async function get_delay(): Awaitable<?float> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $delay = $conn->connectResult()?->clientStats()?->callbackDelayMicrosAvg();
  $conn->close();
  return $delay;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $d = await get_delay();
  \var_dump($d);
}
```.hhvm.expectf
float(%f)
```.example.hhvm.out
float(44.25)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
