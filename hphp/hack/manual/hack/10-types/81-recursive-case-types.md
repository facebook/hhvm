# Recursive Case Types

Recursive case types extend [case types](/hack/types/case-types) to allow self-referencing definitions.

## Examples

```hack
// A tree of ints: either a vec of subtrees, or an int leaf
case type IntTree = vec<IntTree> | int;

// A linked list: either a (head, tail) pair, or null
case type Lst<T> = (T, Lst<T>) | null;

// A JSON-like value
case type JsonValue = null | bool | int | float | string
  | vec<JsonValue> | dict<string, JsonValue>;
```

## Recursion Must Be Guarded

The recursive reference must appear inside a **type constructor** such as `vec<_>`, `dict<_, _>`, a tuple, a shape, or a class type parameter. It must not appear bare at the top level. This ensures the type is well-founded — each level of recursion is wrapped in a constructor that can be checked at runtime.

Examples of invalid case types:

```hack
// Bare self-reference
case type A = A;

// Nullable is not a type constructor
case type B = ?B;
```

## Mutual Recursion

Multiple case types may reference each other:

```hack
case type Forest<T> = (Tree<T>, Forest<T>) | null;
case type Tree<T> = shape(
  'tree' => T,
  'forest' => Forest<T>,
);
```

Mutual recursion through regular type aliases also works as long as a case type is involved:

```hack
type Cons<T> = (T, Lst<T>);
case type Lst<T> = Cons<T> | null;
```

## Known limitation: Single-variant recursive case types

Even case types with only a single variant follow the same asymmetric subtyping rules for case types. This means that, for example, a case type whose only variant is a shape still requires an `is` check before you can access its fields.

```hack
case type Tree<T> = shape(
  'data' => T,
  'children' => vec<Tree<T>>,
);

function get_data_bad<T>(Tree<T> $tree): T {
  return $tree['data']; // ERROR: can't access shape fields on a case type
}

function get_data_good<T>(Tree<T> $tree): T {
  if ($tree is shape(...)) {
    return $tree['data']; // OK: narrowed to the shape variant
  }
}
```

In some cases, one way to avoid this is to use a transparent `type` alias for the shape and make the case type reference it:

```hack
type Tree<T> = shape(
  'data' => T,
  'children' => vec<Forest<T>>,
);
case type Forest<T> = Tree<T> | null;

function get_data<T>(Tree<T> $tree): T {
  return $tree['data']; // OK: type aliases expand directly
}
```
