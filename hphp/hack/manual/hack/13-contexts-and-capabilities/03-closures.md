# Closures

By default, closures require the same capabilities as the context in which they are created.

```hack
function foo()[write_props]: void { // scope has {WriteProperty}
  // inherits [write_props] — the most common usage
  $callable = () ==> {/* ... */};
}
```

Closures may optionally declare an explicit context list.
This is useful when the closure is meant to be passed elsewhere or returned
and not directly invoked in the enclosing function.

```hack
function bar()[globals]: void { // scope has {AccessGlobals}
  $pure = ()[] ==> {/* ... */}; // create a closure with fewer capabilities
  $defaults = ()[defaults] ==> {/* ... */}; // create a closure with more capabilities
}
```

Note that in the previous example, `$defaults` could not be called in `bar` because `bar` has only the `AccessGlobals` capability.
