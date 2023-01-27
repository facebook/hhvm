**Note: The semantics of operators `==` and `!=` were inherited from PHP, and are sometimes not
what is expected. As such, use `===` and `!==` instead.** See the discussion below.

The binary equality operators are, as follows:
* `==`, which represents *value-equality*
* `!=`, which represents *value-inequality*
* `===`, which represents *same-type-and-value-equality*
* `!==`, which represents *not-same-type-and-value-equality*.
* `<=>`, which indicates less-than, equal-to, or greater-than (see later below)

However, when comparing two objects, operator `===` represents *identity* and operator `!==` represents *non-identity*. Specifically,
in this context, these operators check to see if the two operands are the exact same object, not two different objects of the same type and value.

The type of the result of `==`, `!=`, `===`, and `!==`, is `bool`.

```Hack no-extract
null == 0;   // result has value true
null === 0;  // result has value false
true != 100;  // result has value false
true !== 100;  // result has value true

"10" != 10;  // result has value false
"10" !== 10; // result has value true

vec[10,20] == vec[10,20.0];  // result has value true
vec[10,20] === vec[10,20.0]; // result has value false
dict["red"=>0,"green"=>0] === dict["red"=>0,"green"=>0]; // result has value true
dict["red"=>0,"green"=>0] === dict["green"=>0,"red"=>0]; // result has value false
```

When using `==` and `!=` to compare strings that start with numeric digits, such strings
are converted to `int` or `float` and then compared numerically. As such,

```Hack
'0e789' == '0e123';   // True
'0e789' != '0e123';   // False
```

because both strings actually have the same numeric value, zero! (Zero to the power 789 is the
same as zero to the power 123.) However,

```Hack
'0e789' === '0e123';   // False
'0e789' !== '0e123';   // True
```

because there is no numeric conversion; the strings are compared character by character.

## The Spaceship Operator
Often referred to as the *spaceship operator*, the binary operator `<=>` compares the values of its operands and returns an `int`
result. If the left-hand value is less than the right-hand value, the result is some unspecified negative value; else, if the left-hand
value is greater than the right-hand value, the result is some unspecified positive value; otherwise, the values are equal and the result is zero. For example:

```Hack
1 <=> 1;         // 0; equal
1 <=> 2;         // negative; 1 < 2
2 <=> 1;         // positive; 2 > 1

"a" <=> "a";     // 0; same length and content
"a" <=> "b";     // negative; a is lower than b in the collating sequence
"b" <=> "a";     // positive; b is higher than a in the collating sequence
"a" <=> "A";     // positive; lowercase a is higher than uppercase A

"a" <=> "aa";    // negative; same leading part, but a is shorter than aa
"aa" <=> "a";    // positive; same leading part, but aa is longer than a
"aa" <=> "aa";   // 0; same length and content
```
