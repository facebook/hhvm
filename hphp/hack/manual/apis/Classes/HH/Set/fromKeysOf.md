
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates a [` Set `](/apis/Classes/HH/Set/) from the keys of the specified container




``` Hack
public static function fromKeysOf<Tk as arraykey>(
  ?KeyedContainer<Tk, mixed> $container,
): Set<Tk>;
```




The keys of the container will be the values of the [` Set `](/apis/Classes/HH/Set/).




## Parameters




+ ` ? `[` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` mixed> $container `` - The container with the keys used to create the [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<Tk> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) built from the keys of the specified container.




## Examples




This example creates new [` Set `](/apis/Classes/HH/Set/)s from a string-keyed [` Map `](/apis/Classes/HH/Map/) and associative array:




``` string-keys.hack
$fruit_calories = Map {
  'apple' => 95,
  'orange' => 45,
};

$vegetable_calories = darray[
  'cabbage' => 176,
  'potato' => 163,
];

// Create a Set from the keys of a Map
$fruit_names = Set::fromKeysOf($fruit_calories);
\var_dump($fruit_names);

// Create a Set from the keys of an associative array
$vegetable_names = Set::fromKeysOf($vegetable_calories);
\var_dump($vegetable_names);
```




This example creates new [` Set `](/apis/Classes/HH/Set/)s from an int-keyed [` Map `](/apis/Classes/HH/Map/) and associative array:




``` int-keys.hack
$uploaders_by_id = Map {
  4993063 => 'Amy Smith',
  9361760 => 'John Doe',
};

$commenters_by_id = darray[
  7424854 => 'Jane Roe',
  5740542 => 'Joe Bloggs',
];

// Create a Set from the integer keys of a Map
$uploader_ids = Set::fromKeysOf($uploaders_by_id);
\var_dump($uploader_ids); // $uploader_ids contains 4993063, 9361760

// Create a Set from the integer keys of an associative array
$commenter_ids = Set::fromKeysOf($commenters_by_id);
\var_dump($commenter_ids); // $commenter_ids contains 7424854, 5740542
```
<!-- HHAPIDOC -->
