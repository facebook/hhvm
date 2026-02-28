
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Alters the current [` Set `](/apis/Classes/HH/Set/) so that it only contains the values that meet a
supplied condition on its "keys" and values




``` Hack
public function retainWithKey(
  (function(arraykey, Tv): bool) $callback,
): Set<Tv>;
```




[` Set `](/apis/Classes/HH/Set/)s don't have keys, so the [` Set `](/apis/Classes/HH/Set/) values are used as the key in the
callback.




This method is like [` filterWithKey() `](/apis/Classes/HH/Set/filterWithKey/), but mutates the current [` Set `](/apis/Classes/HH/Set/) too
in addition to returning the current [` Set `](/apis/Classes/HH/Set/).




Future changes made to the current [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Parameters




+ ` (function(arraykey, Tv): bool) $callback `




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.
<!-- HHAPIDOC -->
