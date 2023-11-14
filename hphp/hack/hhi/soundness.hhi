<?hh
namespace HH\FIXME;

/**
 * `UNSAFE_CAST` allows you to lie to the type checker. **This is almost
 * always a bad idea**. You might get an exception, or you might get an
 * unexpected value. Even scarier, you might cause an exception in a
 * totally different part of the codebase.
 *
 * ```
 * UNSAFE_CAST<Dog, Cat>($my_dog, 'my reason here')->meow()
 * ```
 *
 * In this example, the type checker will accept the code, but the code
 * will still crash when you run it (no such method "meow" on "Dog").
 *
 * `UNSAFE_CAST` has no runtime effect. It only affects the type
 * checker. The above example will run the same as `$my_dog->meow()`.
 *
 * ## You can fix it!
 *
 * It is always possible to write code without `UNSAFE_CAST` and without
 * `HH_FIXME`. This usually requires changing type signatures and some
 * refactoring. Your code will be more reliable, the type checker can
 * help you, and future changes will be less scary.
 *
 * `UNSAFE_CAST` is still better than `HH_FIXME`, because `HH_FIXME`
 * applies to the entire next line, and `UNSAFE_CAST` applies to a single
 * expression.
 */
function UNSAFE_CAST<<<__Explicit>> Tin, <<__Explicit>> Tout>(
  Tin $t,
  ?\HH\FormatString<nothing> $msg = null,
)[]: Tout;

/**
 * `UNSAFE_NONNULL_CAST` allows you to lie to the type checker and
 * pretend that a value is never `null`. **This is almost always a bad
 * idea**. It can lead to exceptions that the type checker would have
 * prevented.
 *
 * If you're sure a value is never null, use `$my_value as nonnull`. The
 * type checker understands this, and the runtime checks the value.
 *
 * If you're not sure whether a value is null, check for null first.
 *
 * ```
 * if ($my_value is null) { ... } else { ... }
 * ```
 */
function UNSAFE_NONNULL_CAST<T as nonnull>(
  ?T $t,
  ?\HH\FormatString<nothing> $msg = null,
)[]: T;



/* Acts as T under current semantics, and ~T under sound dynamic */
type POISON_MARKER<+T> = T;



/**
 * We haven't written the type for every property in the codebase yet.
 * Properties which are missing their types have this placeholder instead.
 */
type MISSING_PROP_TYPE = dynamic;

/**
 * We haven't written the type for every parameter to every function in the
 * codebase yet. Function parameters which are missing their types have this
 * placeholder instead.
 */
type MISSING_PARAM_TYPE = mixed;

/**
 * We haven't written the return type for every function in the codebase yet.
 * Functions which are still missing return types have this placeholder
 * instead.
 */
type MISSING_RETURN_TYPE = dynamic;
