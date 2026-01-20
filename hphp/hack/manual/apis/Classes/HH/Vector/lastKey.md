
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last key in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function lastKey(): ?int;
```




## Returns




+ ` ?int ` - The last key (an integer) in the current [` Vector `](/apis/Classes/HH/Vector/), or `` null `` if
  the [` Vector `](/apis/Classes/HH/Vector/) is empty.




## Examples




This example shows how [` lastKey() `](/apis/Classes/HH/Vector/lastKey/) can be used even when a [` Vector `](/apis/Classes/HH/Vector/) may be empty:




``` basic-usage.hack
function echoLastKey(Vector<string> $v): void {
  $last_key = $v->lastKey();
  if ($last_key !== null) {
    echo 'Last key: '.$last_key."\n";
  } else {
    echo 'No last key (Vector is empty)'."\n";
  }
}

<<__EntryPoint>>
function basic_usage_main(): void {
  // Will print "Last key: 3"
  echoLastKey(Vector {'red', 'green', 'blue', 'yellow'});

  // Will print "No last key (Vector is empty)"
  echoLastKey(Vector {});
}
```
<!-- HHAPIDOC -->
