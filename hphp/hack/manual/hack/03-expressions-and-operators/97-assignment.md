# Assignment

The assignment operator `=` assigns the value of the right-hand operand to the left-hand operand.  For example:

```hack
$a = 10;
```

## Element Assignment

We can assign to array elements, as follows:

```hack
$v = vec[1, 2, 3];

$v[0] = 42; // $v is now vec[42, 2, 3]

$v = dict[0 => 10, 1 => 20, 2 => 30];
$v[1] = 22;     // change the value of the element with key 1
$v[-10] = 19;   // insert a new element with key -10
```

For `vec`, indexes must be within the range of the
existing values. Use `$v[] = new_value;` to append new values.

For `dict`, we can insert at arbitrary keys.

``` Hack
$d = dict['x' => 1];
$d['y'] = 42; // $d is now dict['x' => 1, 'y' => 42]
```

Strings can also be assigned like arrays. However, it is possible to
assign beyond the end of the string. The string will be extended with
spaces as necessary.

``` Hack
$s = "ab";
$s[0] = "x"; // in bounds
$s[3] = "y"; // $s is now "xb y"
```

## Compound Assignments

Infix operators in Hack have a corresponding compound assignment
operator. For example, `+` has compound assignment operator `+=`.

``` Hack no-extract
$x += 10;

// Equivalent to:
$tmp = $x + 10;
$x = $tmp;
```

The complete set of compound-assignment operators is: `**=`, `*=`, `/=`, `%=`, `+=`, `-=`, `.=`, `<<=`, `>>=`, `&=`, `^=`, `|=`, and
[`??=`](/hack/expressions-and-operators/coalesce#coalescing-assignment-operator).
