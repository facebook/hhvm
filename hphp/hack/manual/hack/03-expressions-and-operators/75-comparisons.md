Hack provides comparison operators. They operate on two operands and
return `bool`.

* `<`, which represents *less-than*
* `>`, which represents *greater-than*
* `<=`, which represents *less-than-or-equal-to*
* `>=`, which represents *greater-than-or-equal-to*

The type of the result is `bool`.

Comparison operators are typically used with numbers:

``` Hack
1 < 2;     // true
1.0 <= 1.5; // true
```

However, comparisons also work on strings. This uses a lexicographic ordering
unless both strings are numeric, in which case the numeric values are
used:

``` Hack
"a" < "b";   // true
"ab" < "b";  // true

"01" < "1";  // false (1 == 1)
```

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
