Besides `is`-expressions, Hack supports another form of type
refinements, which we refer to as `with`-refinements in this section.
This feature allows more precise typing of classes/interfaces/trait in
a way that specific _type_ or _context_ constant(s) are more specific
(i.e., refined).

For example, given the definition

```Hack file:box-with-type+ctx.hack

interface Box {
  abstract const type T;
  abstract const ctx C super [defaults];
  public function get()[this::C]: this::T;
}
```

one can write a function for which Hack statically guarantees the
returned `Set` is
[valid](https://docs.hhvm.com/hack/reference/class/HH.Set/), i.e., it
only contains integers and/or strings, and not objects of any other type:

```Hack file:box-with-type+ctx.hack
function unwrap_as_singleton_set<Tb as arraykey>(
  Box with { type T = Tb } $int_or_string_box
): Set<Tb> {
  return Set { $int_or_string_box->get() };
}
```

Independently, one can constrain context `C` in `Box`. For example, to
work with `Box` subtypes which implement the `get` method in a pure
way (without side effects), `with`-refinements can be used as follows:

```Hack file:box-with-type+ctx.hack
function unwrap_pure<Tb>(
  Box with { ctx C = []; type T = Tb } $box,
)[]: Tb {
  return $box->get(); // OK (type-checker knows `get` must have the empty context list)
}
```

A notable use case unlocked by this feature is that a `with`-refinement
can appear in return positions, e.g.:

```Hack file:box-with-type+ctx.hack
// API
function wrap_number(num $x): Box with { type T = num } {
  return new IntBox($x);
}
// implementation details (subject to change):
class IntBox implements Box {
  const type T = num;
  const ctx C = [];

  public function __construct(private this::T $v) {}
  public function get()[]: this::T { return $this->v; }
}
```

This is something that is inexpressible with where-clauses.

Loose (`as`, `super`) bounds on refined constants, such as `type T`
and `context C`, are also supported. For example, you can write
functions _statically_ safe functions such as:

```Hack file:box-with-type+ctx.hack
function boxed_sum(
  Traversable<Box with { type T as num }> $numeric_boxes
): float {
  $sum = 0.0;
  foreach ($numeric_boxes as $nb) {
    $sum += $nb->get();
  }
  return $sum;
}
```

and avoid assertions that objects returned by Box’s `get` methods are
numbers (`int` or `float`).

Finally, you can also use
[generics](https://docs.hhvm.com/hack/generics/introduction) in
bounds; e.g., the above function could have signature

```
function boxed_sum_generic<T as num>(
  Traversable<Box with { type T = T }> $numeric_boxes
): T /* or float */
```

### Sound alternative to `TGeneric::TAbstract`

This section shows how to improve type safety of existing Hack code
that employs a common pattern in which the intent is to read the type
constant associated with the function- or class-level generic that is
bounded by an abstract class or interface.

As an example, consider the following definition:

```Hack file:box-with-type+ctx.hack
interface MeasuredBox extends Box {
  abstract const type TQuantity;
  public function getQuantity(): this::TQuantity;
}
```

Projections off a generic at the function level, such as

```Hack file:box-with-type+ctx.hack
function weigh_bulk_unsafe<TBox as MeasuredBox>(TBox $box): float
where TBox::TQuantity = float {
  return $box->getQuantity();
}
```

can and should be translated into:

```Hack file:box-with-type+ctx.hack
function weigh_bulk(MeasuredBox with { type TQuantity = float } $box): float {
  return $box->getQuantity();
}
```

Hack offers a means of _conditionally_ enabling specific methods via
where-clauses. E.g.,
to define a method `unloadByQuantity` that is only callable on
subclasses of `Box` where `TQuantity` is an integer (representing
boxes with quantity that is countable exactly), one could write:

```Hack no-extract
class Warehouse<TBox as Box> {
  public function unloadByCount(TBox $boxes): void
  where TBox::TQuantity = int
  { /* … */ }
}
```

This can be translated to type refinements with a nuance:

```Hack no-extract
class Warehouse<TBox as Box> {
  public function unloadByCount(TBox $boxes): int
  where TBox as Box with { type TQuantity = int }
  { /* … */ }
}
```

#### **Migration note**:

This is _stricter_ than the original version with where-clauses
because it is actually sound. Notably, the migrated method, which now
uses type refinements, is _uncallable_ from unmigrated methods that
still use where-clauses.

```Hack no-extract
function callee_that_now_errs<TBox as Box>(
  Warehouse<TBox> $warehouse,
  TBox $unknown_box,
  T $contents,
): void where TBox::T = T {
  $warehouse->unloadByCount($unknown_box); // ERROR
  /* … */
}
```

Therefore, while migrating code with pattern `TGeneric::TAbstractType`
and where-clauses, you will need to migrate top-down in the callee
graph. This process may also reveal some unsafe usages of the previous
pattern, which is too permissive in theory and could allow reading
abstract type (and thus fail at run-time).
and Hack may chose to do so in the future, too.
