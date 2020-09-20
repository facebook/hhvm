(** Type A *)
type a = int

(** Type B
 * is int
 *)
type b = int

(** Type C has a fenced code block:
 *
 * ```
 * function f(): int {
 *   return 0;
 * }
 * ```
 *
 * And an unfenced code block:
 *
 *     function g(): int {
 *       return 0;
 *     }
 *
 * They should stay indented.
 *)
type c = int

(** Type D has no leading asterisks and a code block:

```
function f(): int {
  return 0;
}
```

And an indented code block:

    ```
    function g(): int {
      return 0;
    }
    ```
*)
type d = int

(** Records can have comments on the fields. *)
type record = {
  foo: int;
      (** The comments need to trail the field declaration in OCaml (unfortunately). *)
  bar: int;  (** bar comment *)
}

(** Variant types can have comments on each variant. *)
type variant =
  | Foo
      (** Again, the comments need to trail the variant declaration.
       * Multiline comments are understood. *)
  | Bar
      (** Multiline comments do not need the leading asterisk on subsequent lines,
          but it is removed when it is present. *)
