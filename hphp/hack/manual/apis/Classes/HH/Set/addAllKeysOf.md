
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Adds the keys of the specified container to the current [` Set `](/apis/Classes/HH/Set/) as new
values




``` Hack
public function addAllKeysOf(
  ?KeyedContainer<Tv, mixed> $container,
): Set<Tv>;
```




Future changes made to the current [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Parameters




+ ` ? `[` KeyedContainer<Tv, `](/apis/Interfaces/HH/KeyedContainer/)`` mixed> $container `` - The container with the new keys to add.




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.




## Examples




This example adds ` string ` keys from a [` Map `](/apis/Classes/HH/Map/) to a [` Set `](/apis/Classes/HH/Set/) as its values:




``` string-keys.hack
$fruit_calories = Map {
  'apple' => 95,
  'orange' => 45,
};

$vegetable_calories = darray[
  'cabbage' => 176,
  'potato' => 163,
];

$food_names = Set {};

// Add the keys from a Map
$food_names->addAllKeysOf($fruit_calories);

// Add the keys from an associative array
$food_names->addAllKeysOf($vegetable_calories);

\var_dump($food_names);
```




This example adds ` int ` keys from a [` Map `](/apis/Classes/HH/Map/) to a [` Set `](/apis/Classes/HH/Set/) as its values:




``` int-keys.hack
$uploaders_by_id = Map {
  4993063 => 'Amy Smith',
  9361760 => 'John Doe',
};

$commenters_by_id = darray[
  4993063 => 'Amy Smith',
  7424854 => 'Jane Roe',
  5740542 => 'Joe Bloggs',
];

$all_ids = Set {};

// Add the keys from a Map
$all_ids->addAllKeysOf($uploaders_by_id);

// Add the keys from an associative array
$all_ids->addAllKeysOf($commenters_by_id);

\var_dump($all_ids);
```
<!-- HHAPIDOC -->
