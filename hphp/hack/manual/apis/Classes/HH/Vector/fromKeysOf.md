
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates a [` Vector `](/apis/Classes/HH/Vector/) from the keys of the specified container




``` Hack
public static function fromKeysOf<Tk as arraykey>(
  ?KeyedContainer<Tk, mixed> $container,
): Vector<Tk>;
```




Every key in the provided [` KeyedContainer `](/apis/Interfaces/HH/KeyedContainer/) will appear sequentially in the
returned [` Vector `](/apis/Classes/HH/Vector/), with the next available integer key assigned to each.




## Parameters




+ ` ? `[` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` mixed> $container `` - The container with the keys used to create the
  [` Vector `](/apis/Classes/HH/Vector/).




## Returns




* [` Vector<Tk> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) built from the keys of the specified container.




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

// Create a Vector from the keys of a Map
$fruit_names = Vector::fromKeysOf($fruit_calories);
\var_dump($fruit_names);

// Create a Vector from the keys of an associative array
$vegetable_names = Vector::fromKeysOf($vegetable_calories);
\var_dump($vegetable_names);
```




This example creates new [` Vector `](/apis/Classes/HH/Vector/)s from an int-keyed [` Map `](/apis/Classes/HH/Map/) and an associative array:




``` int-keys.hack
$uploaders_by_id = Map {
  4993063 => 'Amy Smith',
  9361760 => 'John Doe',
};

$commenters_by_id = darray[
  7424854 => 'Jane Roe',
  5740542 => 'Joe Bloggs',
];

// Create a Vector from the integer keys of a Map
$uploader_ids = Vector::fromKeysOf($uploaders_by_id);
\var_dump($uploader_ids); // $uploader_ids contains 4993063, 9361760

// Create a Vector from the integer keys of an associative array
$commenter_ids = Vector::fromKeysOf($commenters_by_id);
\var_dump($commenter_ids); // $commenter_ids contains 7424854, 5740542
```
<!-- HHAPIDOC -->
