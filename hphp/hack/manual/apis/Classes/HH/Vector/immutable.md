
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable copy ([` ImmVector `](/apis/Classes/HH/ImmVector/)) of the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function immutable(): ImmVector<Tv>;
```




This method is interchangeable with [` toImmVector() `](/apis/Classes/HH/Vector/toImmVector/).




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) copy of the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




``` basic-usage.hack
function expects_immutable(ImmVector<mixed> $iv): void {
  \var_dump($iv);
}

<<__EntryPoint>>
function basic_usage_main(): void {
  $v = Vector {'red', 'green', 'blue', 'yellow'};

  // Get a deep, immutable copy of $v
  $immutable_v = $v->immutable();

  expects_immutable($immutable_v);
}
```
<!-- HHAPIDOC -->
