# Using

A `using` statement is used to enforce [object disposal](/hack/classes/object-disposal).  It has two forms: block and non-block. Here is an
example of the block form:

```hack no-extract
  using ($f1 = new TextFile("file1.txt", "rw")) {
    // ... work with the file
  } // __dispose is called here
```

The type of the expression inside the parentheses must implement either `IDisposable` (or `IAsyncDisposable`). The scope of `$f1` is
the `using` block, and at the end of that scope, `__dispose` (or `__disposeAsync`) is called. If the assignment (as in, `$f1 = `) is
omitted, we cannot access the object directly inside the block.

Within the block, there are limits to what we can do with `$f1`. Specifically, we *cannot* assign to it again or make copies of it.  And to
pass it to a function, we must mark the function's corresponding parameter with the
[attribute __AcceptDisposable](/hack/attributes/predefined-attributes#__acceptdisposable).  We can also call methods on the object
that `$f1` designates.  Consider the following:

```hack no-extract
  using ($f1 = new TextFile("file1.txt", "rw")) {
//  echo "\$f1 is >" . $f1 . "<\n";  // usage not permitted
    echo "\$f1 is >" . $f1->__toString() . "<\n";
    // ...
  }
```

Note the commented-out trace statement at the start of the block. Under the hood, we're trying to pass a copy of a TextFile to `echo`, but
`echo` doesn't know anything about TextFile's object cleanup, so that is rejected. We can, however, directly call a method on that object,
which is why `__toString` is called explicitly in the statement following.

Here is an example of the non-block form:

```hack no-extract
function foo(): void {
  using $f4 = TextFile::open_TextFile("file4.txt", "rw");
  using $f5 = new TextFile("file5.txt", "rw");
  // ... work with both files
} // __dispose is called here for both $f4 and $f5
```

The difference here is that no parentheses are required around the controlling expression, we use a trailing semicolon instead of a block,
and the scope of the assigned-to variables ends at the end of the parent block, which avoids the need to use nested `using` statements.

See [object disposal](/hack/classes/object-disposal) for a detailed example of the use of both forms.
