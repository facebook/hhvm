
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) with the values being the keys of the current
[` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function keys(): ImmVector<int>;
```




This method will return an [` ImmVector `](/apis/Classes/HH/ImmVector/) with keys 0 and 1, and values 0 and
1, since the keys of a [` Pair `](/apis/Classes/HH/Pair/) are 0 and 1.




## Returns




+ [` ImmVector<int> `](/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the integer keys of the current
  [` Pair `](/apis/Classes/HH/Pair/) as values.




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};
\var_dump($p->keys());
```
<!-- HHAPIDOC -->
