# Subscript

The subscript operator, `[...]` is used to designate an element of a string, a vec, a dict, or a keyset. The element key is
designated by the expression contained inside the brackets. For a string or vec, the key must have type `int`, while dict and
keyset also allow `string`. The type and value of the result is the type and value of the designated element. For example:

```hack
$text = "Hello";
$e = $text[4];    // designates the element with key 4 value "o"
$text[1] = "?";   // changes the element with key 1 from "e" to "?"

$v = vec[10, 25, -6];
$e = $v[1];     // designates the element with key 1 value 25
$v[2] = 44;     // changes the element with key 2 from -6 to 44

$d = dict["red" => 4, "white" =>12, "blue" => 3];
$e = $d["white"]; // designates the element with key "white" value 12
$d["red"] = 9;    // changes the element with key "red" from 4 to 9

$k = keyset["red", "blue", "green"];
$e = $k["blue"];  // designates the element with key "blue" value "blue"
```

In a keyset, keys and values are the same, so `$k["blue"]` evaluates to `"blue"`. Reading from a keyset this way is rarely useful in practice — it exists for consistency with the other container types. Typically, you would use `C\contains_key($k, "blue")` to test for membership instead.

## Append vs Update

For a vec, empty brackets append a new element at the end.

```hack
$v = vec[10, 25, -6];
$v[] = 99;    // appends: $v is now vec[10, 25, -6, 99]
$v[0] = 77;   // updates existing element at key 0
```

For a dict, assigning to a key that doesn't exist inserts a new element at the end; assigning to an existing key updates it.

```hack
$d = dict["a" => 1];
$d["b"] = 2;  // inserts new key "b"
$d["a"] = 9;  // updates existing key "a"
```

For a keyset, only empty brackets are supported. This adds the value if it is not already present.

```hack
$k = keyset[1, 2, 3];
$k[] = 4;     // adds 4: $k is now keyset[1, 2, 3, 4]
$k[] = 2;     // no change: 2 is already present
```

Strings do not support appending. Only existing character positions can be updated.

## Out-of-Bounds Access

Accessing an element with a key that doesn't exist throws an `OutOfBoundsException` at runtime.

```hack
$v = vec[10, 20];
$v[5];             // OutOfBoundsException

$d = dict["a" => 1];
$d["missing"];     // OutOfBoundsException

$k = keyset["a"];
$k["missing"];     // OutOfBoundsException
```

Use `idx` to safely access elements that may not exist, or `C\contains_key` to check for the presence of a key.

```hack
$d = dict["a" => 1, "b" => 2];
$val = idx($d, "missing");      // null
$val = idx($d, "missing", 0);   // 0 (default)
```
