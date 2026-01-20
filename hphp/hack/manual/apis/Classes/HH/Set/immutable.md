
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable ([` ImmSet `](/apis/Classes/HH/ImmSet/)), deep copy of the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function immutable(): ImmSet<Tv>;
```




This method is interchangeable with [` toImmSet() `](/apis/Classes/HH/Set/toImmSet/).




## Returns




+ [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - an [` ImmSet `](/apis/Classes/HH/ImmSet/) that is a deep copy of the current [` Set `](/apis/Classes/HH/Set/).




## Examples




``` basic-usage.hack
function expects_immutable(ImmSet<string> $is): void {
  \var_dump($is);
}

<<__EntryPoint>>
function basic_usage_main(): void {
  $s = Set {'red', 'green', 'blue', 'yellow'};

  // Get a deep, immutable copy of $s
  $immutable_s = $s->immutable();

  expects_immutable($immutable_s);
}
```
<!-- HHAPIDOC -->
