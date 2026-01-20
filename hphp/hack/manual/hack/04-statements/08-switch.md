# Switch

A `switch` statement typically consists of a controlling expression, some case labels, and optionally a default label.  Based on the
value of the controlling expression, execution passes to one of the case labels, the default label, or to the statement immediately
following the switch statement.  For example:

```hack no-extract
enum Bank: int {
  DEPOSIT = 1;
  WITHDRAWAL = 2;
  TRANSFER = 3;
}

function processTransaction(Transaction $t): void {
  $trType = ...;     // get the transaction type as an enum value
  switch ($trType) {

  case Bank::TRANSFER:
    // ...
    break;
  case Bank::DEPOSIT:
    // ...
    break;
  case Bank::WITHDRAWAL:
    // ...
    break;
  }
}
```

The `switch` has case labels for each of the possible value of `enum Bank`, where a case label is the keyword `case` followed by an
expression, followed by `:`, followed by zero or more statements.  Note that the so-called body of a case label need *not* be made a
compound statement; the set of statements associated with a particular case label ends with a `break` statement or the closing brace (`}`)
of the `switch` statement.

A default label is used as a catch-all.  In the enumerated-type example above, assuming all possible values of type `enum Bank` have
corresponding case labels, no default label is needed, as no other values can exist!  However, that is not true when the switch controlling
expression has type `int`. Here, we're unlikely to have case labels cover the complete range of `int` values, so a default label might be
necessary.  For example:

```hack
$v = 100;

switch ($v) {
  case 20:
    // ...
    break;
  case 10:
    // ...
    break;
  case 30:
    // ...
    break;
  default:
    // ...
    break;
}
```

A default label is the keyword `default`, followed by `:`, followed by zero or more statements.  Note that the so-called body of a default
label need *not* be made a compound statement; the set of statements associated with a particular default label ends with a `break`
statement or the closing brace (`}`) of the `switch` statement.

If a `switch` contains more than one case label whose values compare equal to the controlling expression, the first in lexical order is
considered the match.

An arbitrary number of statements can be associated with any case or default label. In the absence of a `break` statement at the end of
a set of such statements, control drops through into any following case or default label. Thus, if all cases and the default end in `break`
and there are no duplicate-valued case labels, the order of case and default labels is insignificant.

If no `break` statement is seen for a case or default before a subsequent case label, default label, or the switch-terminating `}` is
encountered, an implementation might issue a warning. However, such a warning can be suppressed by placing a source line containing the
special comment `// FALLTHROUGH`, at the end of that case or default statement group.

```hack
$v = 10;
switch ($v) {
  case 10:
    // ...
    // FALLTHROUGH
  case 30:
    // ...         // Handle 10 or 30
    break;
  default:
    // ...
    break;
}
```

Case-label values can be runtime expressions, and the types of sibling case-label values need not be the same.

**Note switch uses `==` equality for comparing the value with the
different cases.**. See [equality](/hack/expressions-and-operators/equality) for more details.

```hack
$v = 30;
switch ($v) {
  case 30.0:  // <===== this case matches with 30
    // ...
    break;
  default:
    // ...
    break;
}
```
