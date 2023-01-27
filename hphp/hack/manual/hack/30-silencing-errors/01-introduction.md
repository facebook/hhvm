Errors reported by the Hack typechecker can be silenced with
`HH_FIXME` and `HH_IGNORE_ERROR` comments. Errors arising from type mismatches
on expressions may also be silenced using the `HH\FIXME\UNSAFE_CAST` function.

## Silencing Errors with `HH\FIXME\UNSAFE_CAST`

```no-extract
takes_int(HH\FIXME\UNSAFE_CAST<string,int>("foo",  "Your explanation here"));
```

To silence an error arising from a type mismatch on a particular expression,
add a call to `HH\FIXME\UNSAFE_CAST` with the expression as the first argument,
an optional (string literal) comment, and explicit type hints indicating the
actual type of the expression and the expected type.

The `UNSAFE_CAST` function **has no runtime effect**. However, in contrast
to `HH_FIXME` comments, the `UNSAFE_CAST` function _does_ change the type of the
expression.

### Silencing Errors per Expression

Whilst a single `HH_FIXME` comment will silence all related errors on the
proceeding line, the `UNSAFE_CAST` function must be applied to each
sub-expression that has a type mismatch.

```Hack file:takes_int.hack
function takes_int(int $i): int {
  return $i + 1;
}

function takes_float_with_fixme(float $i): float {
  /* HH_FIXME[4110] calls takes_int with wrong
     param type AND returns wrong type */
  return takes_int($i);
}
```

```Hack file:takes_int.hack
function takes_float_with_unsafe_cast(float $i): float {
  return HH\FIXME\UNSAFE_CAST<int, float>(
    takes_int(HH\FIXME\UNSAFE_CAST<float, int>($i, 'wrong param type')),
    'returns wrong type',
  );
}
```

## Silencing Errors with Comments

```Hack file:takes_int.hack
/* HH_FIXME[4110] Your explanation here. */
takes_int("foo");
```

To silence an error, place a comment on the immediately previous
line. The comment must use the `/*` syntax.

This syntax only affects error reporting. It does not change types,
so the typechecker will keep checking the rest of the file as before.

This syntax also **has no runtime effect**. The runtime will do its
best with badly typed code, but it may produce an error immediately,
produce an error later in the program, or coerce values to an unwanted
type.

The behavior of badly typed code may change between HHVM
releases. This will usually be noted in the changelog.

### `HH_FIXME` versus `HH_IGNORE_ERROR`

Both `HH_FIXME` and `HH_IGNORE_ERROR` have the same effect: they
suppress an error.

```Hack file:takes_int.hack
/* HH_FIXME[4110] An example fixme. */
takes_int("foo");

/* HH_IGNORE_ERROR[4110] This is equivalent to the HH_FIXME above. */
takes_int("foo");
```

You should generally use `HH_FIXME`. `HH_IGNORE_ERROR` is intended to
signal to the reader that the type checker is wrong and you are
deliberately suppressing the error. This should be very rare.

### Error Codes

Every Hack error has an associated error code. These are stable across
Hack releases, and new errors always have new error codes.

Hack will ignore error suppression comments that have no effect, to
help migration to newer Hack versions.

Error codes 1000 - 1999 are used for parsing errors. Whilst it is
possible to silence these, the runtime usually cannot run this code at
all.

Error codes 2000 - 3999 are used for naming errors. This includes
references to nonexistent names, as well as well-formedness checks
that don't require type information.

Error codes 4000 - 4999 are used for typing errors.

### Configuring Error Suppression

Hack error suppression can be configured in the `.hhconfig` file at the root of a project.
In hhvm version [4.62](https://hhvm.com/blog/2020/06/16/hhvm-4.62.html) and above, error suppression works on a whitelist system.
Older hhvm versions used a blacklisting system instead.

### How to whitelist suppression comments in hhvm 4.62 and above

By default Hack will not accept a suppression comment, if that specific error code is not mentioned in the `.hhconfig` file.
Attempting to suppress an unmentioned error will result in an extra error like this:

```
Typing[4110] You cannot use HH_FIXME or HH_IGNORE_ERROR comments to suppress error 4110
```

If the file in which this error resides is in **partial** mode, add `4110` to the `allowed_fixme_codes_partial` key in your `.hhconfig`.
If the file in which this error resides is in **strict** mode, add `4110` to the `allowed_fixme_codes_strict` key in your `.hhconfig`.

As described further in Best Practices, suppressing errors on declarations is generally a bad idea. However, some errors can only be suppressed at a declaration. When suppressing an error at a declaration, you'll get an error like this.

```
Typing[4047] You cannot use HH_FIXME or HH_IGNORE_ERROR comments to suppress error 4047 in declarations
```

In such cases, you'll have to add `4047` to the `allowed_decl_fixme_codes` key, as well as to the `allowed_fixme_codes_xxx` key.

An important note when using an external package. If a package uses a suppression comment and mentions this in its `.hhconfig`, this will not automatically update the `.hhconfig` settings for your project. In order to use this package, you'll need to add these codes to your own `.hhconfig`.

### Historic note for hhvm 4.61 and below

*If you are writing code on hhvm 4.62 or above, you may skip this section.*

Once you have removed all the occurrences of a specific error code,
you can ensure that no new errors are added.

You can use the `ignored_fixme_codes` option in `.hhconfig` to forbid
suppression of a specific error code.

```
ignored_fixme_codes = 1002, 4110
```

This forbids `/* HH_FIXME[4110] ... */`.

`.hhconfig` also supports `disallowed_decl_fixmes`, which forbids
error suppression of specific error codes on declarations (types,
class properties etc).

```
disallowed_decl_fixmes = 1002, 4110
```

This forbids `/* HH_FIXME[4110] ... */` outside of function and method
bodies.

Partial mode files have fewer checks. You can opt-in to specific
strict mode checks in partial files by using the error code in
`error_codes_treated_strictly` in `.hhconfig`.

```
error_codes_treated_strictly = 1002, 2045, 2055, 2060, 4005
```

## Best Practices

Great Hack code has no error suppressing comments, and only uses
strict mode.

Suppressing errors in one place can lead to runtime errors in other
places.

```
function takes_int(int $_): void {}

function return_ints_lie(): vec<int> {
  /* HH_FIXME[4110] The type error is here. */
  return vec["foo"];
}

<<__EntryPoint>>
function oh_no(): void {
  $items = return_ints_lie();
  takes_int($items[0]); // But the exception is here!
}
```

Good Hack code has no error suppression comments on its
declarations. Suppressing errors in declarations can hide a large
number of issues elsewhere.

```
function takes_int(int $_): void {}
function takes_float(float $_): void {}

/* HH_FIXME[4030] Missing a return type. */
function returns_something() {
  return "";
}

function oh_no(): void {
  // This is wrong.
  takes_int(returns_something());
  // This is wrong too!
  takes_float(returns_something());
}
```

When you use error suppression, make sure you specify a reason. Try to
keep your comments to small expressions, because the comment applies
to the entire next line.

```
/* HH_FIXME[4110] Bad: this can apply to both function calls! */
$result = foo(bar("stuff"));

/* HH_FIXME[4110] Better: we will spot errors when calling bar. */
$x = foo("stuff");
$result = bar($x);
```
