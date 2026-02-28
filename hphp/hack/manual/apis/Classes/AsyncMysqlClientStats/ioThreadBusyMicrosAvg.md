
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Average of reported busy time in the client's IO thread




``` Hack
public function ioThreadBusyMicrosAvg(): float;
```




This returns an exponentially-smoothed average.




## Returns




+ ` float ` - A `` float `` representing the average busy time of this
  MySQL client's IO Thread.
<!-- HHAPIDOC -->
