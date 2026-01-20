
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable vector (` ImmVector `) converted from the current
[` Iterable `](/apis/Interfaces/HH/Iterable/)




``` Hack
public function toImmVector(): ImmVector<Tv>;
```




Any keys in the current [` Iterable `](/apis/Interfaces/HH/Iterable/) are discarded and replaced with integer
indices, starting with 0.




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - an `` ImmVector `` converted from the current [` Iterable `](/apis/Interfaces/HH/Iterable/).
<!-- HHAPIDOC -->
