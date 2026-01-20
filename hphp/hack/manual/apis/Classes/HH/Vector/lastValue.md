
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last value in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function lastValue(): ?Tv;
```




## Returns




+ ` ?Tv ` - The last value in the current [` Vector `](/apis/Classes/HH/Vector/), or `` null `` if the current
  [` Vector `](/apis/Classes/HH/Vector/) is empty.




## Examples




This example shows how [` lastValue() `](/apis/Classes/HH/Vector/lastValue/) can be used even when a [` Vector `](/apis/Classes/HH/Vector/) may be empty:




``` basic-usage.hack
function echoLastValue(Vector<string> $v): void {
  $last_value = $v->lastValue();
  if ($last_value !== null) {
    echo 'Last value: '.$last_value."\n";
  } else {
    echo 'No last value (Vector is empty)'."\n";
  }
}

<<__EntryPoint>>
function basic_usage_main(): void {
  // Will print "Last value: yellow"
  echoLastValue(Vector {'red', 'green', 'blue', 'yellow'});

  // Will print "No last value (Vector is empty)"
  echoLastValue(Vector {});
}
```
<!-- HHAPIDOC -->
