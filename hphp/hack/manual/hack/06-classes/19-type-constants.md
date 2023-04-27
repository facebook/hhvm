Type constants provide a way to abstract a type name.  However, type constants only make sense in the context of interfaces
and inheritance hierarchies, so they are discussed under those topics.

For now, the declaration of a type constant involves the keywords `const type`.  Without explanation, here's an example:

```Hack
abstract class CBase {
  abstract const type T;
  // ...
}

class CString extends CBase {
  const type T = string;
  // ...
}
```

A type constant has public visibility and is implicitly static.

By convention, type constant names begin with an uppercase `T`.

See [inheritance](inheritance.md) and [type constants revisited](type-constants-revisited.md) for more information.
