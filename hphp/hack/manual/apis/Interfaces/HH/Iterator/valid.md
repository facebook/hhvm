
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from http://php.net/manual/en/iterator.valid.php )




``` Hack
public function valid(): bool;
```




This method is called after Iterator::rewind() and Iterator::next() to
check if the current position is valid.




## Returns




+ ` mixed ` - The return value will be casted to boolean and then
  evaluated. Returns TRUE on success or FALSE on
  failure.
<!-- HHAPIDOC -->
