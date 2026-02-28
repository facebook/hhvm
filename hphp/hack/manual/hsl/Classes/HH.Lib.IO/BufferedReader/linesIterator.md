
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Iterate over all lines in the file




``` Hack
public function linesIterator(): AsyncIterator<string>;
```




Usage:




```
foreach ($reader->linesIterator() await as $line) {
  do_stuff($line);
}
```




## Returns




+ [` AsyncIterator<string> `](/apis/Interfaces/HH/AsyncIterator/)
<!-- HHAPIDOC -->
