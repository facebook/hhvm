# HIP: `static` in Expression Trees

### Motivation

The goal of this proposal is to make it easier for developers to write clear and concise code in expression trees.

Currently, expression trees don't support `static::` for class constants and static methods. To work around this, developers reference full class names which is especially verbose for XHP:

```
MyDsl`:component:with-a-very-verbose-name::CARD_WIDTH`
MyDsl`:component:with-a-very-verbose-name::getOverlay()`
```

### Proposed Design

This proposes that we update expression trees so that developers can write code like:

```
MyDsl`static::CARD_WIDTH`
MyDsl`static::getOverlay()`
```

If we stop raising a parser error for `static::`, the code above will be desugared to:

```
$v->visitConstant(
  new ExprPos(...),
  nameof static,
  "CARD_WIDTH",
  \static::CARD_WIDTH,
);

$sm0 = static::getOverlay<>;
$v->visitCall(
  new ExprPos(...),
  $v->visitStaticMethod(
    new ExprPos(...),
    $sm0,
  ),
);
```

### Design Considerations

#### Consistent with Hack Semantics

`static::` behaves the same way in expression trees as it does in regular Hack code. This reduces cognitive overhead for developers already familiar with Hack, and prevents subtle bugs that could arise from mismatched expectations.

#### Traits and Non-Final Classes

Hack does not support using `self::` in function references within traits or non-final classes. Because of this restriction, this proposal excludes support for `self::` and instead prioritizes `static::`, which is supported in these scenarios.

#### Cache Correctness

To support caching class constants and static methods, the class context is provided at runtime and should be included in the cache key for correctness:
- In `visitConstant`, the class context is provided as `nameof static`.
- In `visitStaticMethod`, the class context can be resolved at runtime by calling `fb_callable2name` on the function reference.

### Known Limitations

- **Cannot access private members**: Similar to Hack, private members cannot be accessed with `static::` since a child class may also have an identically named private member.
