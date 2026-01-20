
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable vector (` ImmVector `) with the values of the current
[` Set `](/apis/Classes/HH/Set/)




``` Hack
public function toImmVector(): ImmVector<Tv>;
```




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - an `` ImmVector `` (integer-indexed) with the values of the current
  [` Set `](/apis/Classes/HH/Set/).




## Examples




This example shows that ` toImmVector ` returns an `` ImmVector `` containing the [` Set `](/apis/Classes/HH/Set/)'s values. Mutating the original [` Set `](/apis/Classes/HH/Set/) doesn't affect the `` ImmVector ``.




``` basic-usage.hack
function expects_immutable(ImmVector<string> $iv): void {
  \var_dump($iv);
}

<<__EntryPoint>>
function basic_usage_main(): void {
  $s = Set {'red', 'green', 'blue', 'yellow'};

  // Get an immutable Vector $v of the values in Set $s
  $immutable_v = $s->toImmVector();

  // Add a color to the original Set $s
  $s->add('purple');

  expects_immutable($immutable_v);
}
```
<!-- HHAPIDOC -->
