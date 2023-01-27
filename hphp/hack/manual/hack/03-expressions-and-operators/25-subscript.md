The subscript operator, `[...]` is used to designate an element of a string, a vec, a dict, or a keyset. The element key is
designated by the expression contained inside the brackets. For a string or vec, the key must have type `int`, while dict and
keyset also allow `string`. The type and value of the result is the type and value of the designated element. For example:

```Hack
$text = "Hello";
$e = $text[4];    // designates the element with key 4 value "o"
$text[1] = "?";   // changes the element with key 1 from "e" to "?"

$v = vec[10, 25, -6];
$e = $v[1];     // designates the element with key 1 value 25
$v[2] = 44;     // changes the element with key 2 from -6 to 44

$d = dict["red" => 4, "white" =>12, "blue" => 3];
$e = $d["white"]; // designates the element with key "white" value 12
$d["red"] = 9;    // changes the element with key "red" from 4 to 9
```

For a vec, the brackets can be empty, provided the subscript expression is the destination of an assignment.  This results in a
new element being inserted at the right-hand end. The type and value of the result is the type and value of the new element. For example:

```Hack
$v = vec[10, 25, -6];
$v[] = 99;    // creates new element with key 3, value 99
```
