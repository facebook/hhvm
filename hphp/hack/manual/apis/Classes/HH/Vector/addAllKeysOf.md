
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Adds the keys of the specified container to the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function addAllKeysOf(
  ?KeyedContainer<Tv, mixed> $container,
): Vector<Tv>;
```




For every key in the provided [` KeyedContainer `](/apis/Interfaces/HH/KeyedContainer/), append that key into
the current [` Vector `](/apis/Classes/HH/Vector/), assigning the next available integer key for each.




Future changes made to the current [` Vector `](/apis/Classes/HH/Vector/) ARE reflected in the
returned [` Vector `](/apis/Classes/HH/Vector/), and vice-versa.




## Parameters




+ ` ? `[` KeyedContainer<Tv, `](/apis/Interfaces/HH/KeyedContainer/)`` mixed> $container `` - The [` KeyedContainer `](/apis/Interfaces/HH/KeyedContainer/) with the new keys to add.




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - Returns itself.




## Examples




This example adds ` string ` keys from a [` Map `](/apis/Classes/HH/Map/) to a [` Vector `](/apis/Classes/HH/Vector/) as its values:




``` string-keys.hack
$fruit_calories = Map {
  'apple' => 95,
  'orange' => 45,
};

$vegetable_calories = darray[
  'cabbage' => 176,
  'potato' => 163,
];

$food_names = Vector {};

// Add the keys from a Map
$food_names->addAllKeysOf($fruit_calories);

// Add the keys from an associative array
$food_names->addAllKeysOf($vegetable_calories);

\var_dump($food_names);
```




This example adds ` int ` keys from a [` Map `](/apis/Classes/HH/Map/) to a [` Vector `](/apis/Classes/HH/Vector/) as its values:




``` int-keys.hack
$uploaders_by_id = Map {
  4993063 => 'Amy Smith',
  9361760 => 'John Doe',
};

$commenters_by_id = darray[
  7424854 => 'Jane Roe',
  5740542 => 'Joe Bloggs',
];

$all_ids = Vector {};

// Add the keys from a Map
$all_ids->addAllKeysOf($uploaders_by_id);

// Add the keys from an associative array
$all_ids->addAllKeysOf($commenters_by_id);

\var_dump($all_ids);
```
<!-- HHAPIDOC -->
