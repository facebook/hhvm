
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/apis/Classes/HH/Set/) containing the values after an operation has been applied
to each "key" and value in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function mapWithKey<Tu as arraykey>(
  (function(arraykey, Tv): Tu) $callback,
): Set<Tu>;
```




Since [` Set `](/apis/Classes/HH/Set/)s don't have keys, the callback uses the values as the keys
as well.




Every value in the current [` Set `](/apis/Classes/HH/Set/) is affected by a call to [` mapWithKey() `](/apis/Classes/HH/Set/mapWithKey/),
unlike [` filterWithKey() `](/apis/Classes/HH/Set/filterWithKey/) where only values that meet a certain criteria are
affected.




## Parameters




+ ` (function(arraykey, Tv): Tu) $callback `




## Returns




* [` Set<Tu> `](/apis/Classes/HH/Set/) - a [` Set `](/apis/Classes/HH/Set/) containing the values after a user-specified operation
  on the current [` Set `](/apis/Classes/HH/Set/)'s values is applied.
<!-- HHAPIDOC -->
