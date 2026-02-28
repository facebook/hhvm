
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last "key" in the current [` MutableSet `](/apis/Interfaces/MutableSet/)




``` Hack
public function lastKey(): ?arraykey;
```




Since sets do not have keys, it returns the last value.




This method is interchangeable with [` lastValue() `](/apis/Interfaces/MutableSet/lastValue/).




## Returns




+ ` ?arraykey ` - The last value in the current [` MutableSet `](/apis/Interfaces/MutableSet/), or `` null `` if the
  current [` MutableSet `](/apis/Interfaces/MutableSet/) is empty.
<!-- HHAPIDOC -->
