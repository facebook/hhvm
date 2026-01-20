
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable copy (` ImmVector `) of the current `` Vector ``




``` Hack
public function toImmVector(): ImmVector<Tv>;
```




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - A `` Vector `` that is an immutable copy of the current ``` Vector ```.




## Examples




This example shows that ` toImmVector ` returns an immutable copy of the `` Vector ``. Mutating the original ``` Vector ``` doesn't affect the immutable copy.




``` basic-usage.hack
function expects_immutable(ImmVector<mixed> $iv): void {
  \var_dump($iv);
}

<<__EntryPoint>>
function basic_usage_main(): void {
  $v = Vector {'red', 'green', 'blue', 'yellow'};

  // Get a deep, immutable copy of $v
  $immutable_v = $v->immutable();

  // Add a color to the original Vector $v
  $v->add('purple');

  expects_immutable($immutable_v);
}
```
<!-- HHAPIDOC -->
