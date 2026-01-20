
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a deep, immutable copy (` ImmMap `) of the current `` Map ``




``` Hack
public function toImmMap(): ImmMap<Tk, Tv>;
```




## Returns




+ [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - an ``` ImmMap ``` that is a copy of this ```` Map ````.




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
  $immutable_map = $m->toImmMap();

  expects_immutable($immutable_map);
}
```
<!-- HHAPIDOC -->
