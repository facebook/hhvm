
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable vector (` ImmVector `) with the values of the current
[` Map `](/apis/Classes/HH/Map/)




``` Hack
public function toImmVector(): ImmVector<Tv>;
```




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - an `` ImmVector `` that is an immutable copy of the current [` Map `](/apis/Classes/HH/Map/).




## Examples




This example shows that ` toImmVector ` returns an immutable copy of the [` Map `](/apis/Classes/HH/Map/)'s values. Mutating the `` Vector `` of values doesn't affect the original [` Map `](/apis/Classes/HH/Map/) and vice-versa.




``` basic-usage.hack
function expects_immutable(ImmVector<string> $iv): void {
  \var_dump($iv);
}

<<__EntryPoint>>
function basic_usage_main(): void {
  $m = Map {
    'red' => '#ff0000',
    'green' => '#00ff00',
    'blue' => '#0000ff',
    'yellow' => '#ffff00',
  };

  // Get an immutable Vector of $m's values
  $immutable_v = $m->toImmVector();

  // Add a color to the original Map $m
  $m->add(Pair {'purple', '#663399'});

  expects_immutable($immutable_v);
}
```
<!-- HHAPIDOC -->
