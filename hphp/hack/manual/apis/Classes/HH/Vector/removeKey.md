
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the key/value pair with the specified key from the current
[` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function removeKey(
  int $key,
): Vector<Tv>;
```




This will cause elements with higher keys to be assigned a new key that is
one less than their previous key.  That is, values with keys ` $k + 1 ` to
`` n - 1 `` will be given new keys ``` $k ``` to ```` n - 2 ````, where n is the length of
the current [` Vector `](/apis/Classes/HH/Vector/) before the call to [` removeKey() `](/apis/Classes/HH/Vector/removeKey/).




If ` $k ` is negative, or `` $k `` is greater than the largest key in the current
[` Vector `](/apis/Classes/HH/Vector/), no changes are made.




Future changes made to the current [` Vector `](/apis/Classes/HH/Vector/) ARE reflected in the
returned [` Vector `](/apis/Classes/HH/Vector/), and vice-versa.




## Parameters




+ ` int $key `




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - Returns itself.




## Examples




Since [` Vector::removeKey() `](/apis/Classes/HH/Vector/removeKey/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $v ` itself, you can chain a bunch of [` removeKey() `](/apis/Classes/HH/Vector/removeKey/) calls together.




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Remove 'blue' at index 2
$v->removeKey(2);
\var_dump($v);

// Remove 'red' and then remove 'green'
$v->removeKey(0)->removeKey(0);
\var_dump($v);
```
<!-- HHAPIDOC -->
