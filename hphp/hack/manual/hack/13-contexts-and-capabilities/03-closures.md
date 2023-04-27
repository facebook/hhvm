**Note:** Context and capabilities are enabled by default since
[HHVM 4.93](https://hhvm.com/blog/2021/01/19/hhvm-4.93.html).

As with standard functions, closures may optionally choose to list one or more contexts. Note that the outer function may or may not have its own context list. Lambdas wishing to specify a list of contexts must include a (possibly empty) parenthesized argument list.

```hack no-extract
function some_function(): void {
  $no_list = () ==> {/* some fn body */};
  $single = ()[C] ==> {/* some fn body */};
  $multiple = ()[C1, C2, Cn] ==> {/* some fn body */};
  $with_types = ()[C]: void ==> {/* some fn body */};
  // legacy functions work too
  $legacy = function()[C]: void {};
}
```

By default, closures require the same capabilities as the context in which they are created.

```hack no-extract
function foo()[io]: void { // scope has {IO}
  $callable1 = () ==> {/* some fn body */}; // requires {IO} - By far the most common usage
}
```

Explicitly annotating the closure can be used to opt-out of this implicit behaviour. This is most useful when requiring the capabilities of the outer scope result in unnecessary restrictions, such as if the closure is returned rather than being invoked within the enclosing scope.

```hack no-extract
function foo()[io]: void { // scope has {IO}
  $callable = ()[] ==> {/* some fn body */}; // requires {}
  $uncallable1 = ()[rand] ==> {/* some fn body */}; // requires {Rand}
  $uncallable2 = ()[defaults] ==> {/* some fn body */}; // requires the default set
}
```

Note that in the previous example, `$uncallable1` cannot be called as foo cannot provide the required Rand capability. `$callable` is invocable because it requires strictly fewer capabilities than `foo` can provide.
