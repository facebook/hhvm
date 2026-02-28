---
title: AsyncMysqlClientStats
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Provides timing statistics about the MySQL client




This class provides round-trip and callback timing information for various
operations on the MySQL client.




This information can be used to know how the performance of the MySQL client
may have affected a given result.




For example, if you have a [` AsyncMysqlConnection `](/apis/Classes/AsyncMysqlConnection/), you can call:




` $conn->connectResult()->clientStats()->ioEventLoopMicrosAvg() `




to get round-trip timing information on the connection event itself.




Basically any concrete implementation of [` AsyncMysqlResult `](/apis/Classes/AsyncMysqlResult/) can provide
these type of statistics by calling its `` clientStats() `` method and a method
on this class.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
class AsyncMysqlClientStats {...}
```




### Public Methods




* [` ->callbackDelayMicrosAvg(): float `](/apis/Classes/AsyncMysqlClientStats/callbackDelayMicrosAvg/)\
  Average delay between when a callback is scheduled in the MySQL client
  and when it's actually ran, in microseconds
* [` ->ioEventLoopMicrosAvg(): float `](/apis/Classes/AsyncMysqlClientStats/ioEventLoopMicrosAvg/)\
  Average loop time of the MySQL client event, in microseconds
* [` ->ioThreadBusyMicrosAvg(): float `](/apis/Classes/AsyncMysqlClientStats/ioThreadBusyMicrosAvg/)\
  Average of reported busy time in the client's IO thread
* [` ->ioThreadIdleMicrosAvg(): float `](/apis/Classes/AsyncMysqlClientStats/ioThreadIdleMicrosAvg/)\
  Average of reported idle time in the client's IO thread
* [` ->notificationQueueSize(): int `](/apis/Classes/AsyncMysqlClientStats/notificationQueueSize/)\
  Size of this client's event base notification queue
<!-- HHAPIDOC -->
