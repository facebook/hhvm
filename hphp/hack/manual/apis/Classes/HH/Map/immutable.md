
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a deep, immutable copy ([` ImmMap `](/apis/Classes/HH/ImmMap/)) of this [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function immutable(): ImmMap<Tk, Tv>;
```




This method is interchangeable with [` toImmMap() `](/apis/Classes/HH/Map/toImmMap/).




## Returns




+ [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - an [` ImmMap `](/apis/Classes/HH/ImmMap/) that is a deep copy of this [` Map `](/apis/Classes/HH/Map/).




## Examples




``` basic-usage.hack
function expects_immutable(ImmMap<string, string> $iv): void {
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

  // Get a deep, immutable copy of $m
  $immutable_map = $m->immutable();

  expects_immutable($immutable_map);
}
```
<!-- HHAPIDOC -->
