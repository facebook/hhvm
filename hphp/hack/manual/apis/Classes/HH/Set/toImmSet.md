
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable (` ImmSet `), deep copy of the current `` Set ``




``` Hack
public function toImmSet(): ImmSet<Tv>;
```




This method is interchangeable with [` immutable() `](/apis/Classes/HH/Set/immutable/).




## Returns




+ [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - an `` ImmSet `` that is a deep copy of the current ``` Set ```.




## Examples




``` basic-usage.hack
function expects_immutable(ImmSet<string> $is): void {
  \var_dump($is);
}

<<__EntryPoint>>
function basic_usage_main(): void {
  $s = Set {'red', 'green', 'blue', 'yellow'};

  // Get a deep, immutable copy of $s
  $immutable_s = $s->toImmSet();

  expects_immutable($immutable_s);
}
```
<!-- HHAPIDOC -->
