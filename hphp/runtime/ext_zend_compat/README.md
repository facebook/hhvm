# Zend Extension Source Compatibility Layer

If you want to compile your existing zend extension against HHVM, you can use
these headers. The runtimes are similar enough that we can just map the macros
to our data structures and it mostly works.

## Migration Steps

First, copy in the files:

```sh
cp -R <zend_extension_dir> runtime/ext_zend_compat/<ext_name>

# move all the .c to .cpp
for i in runtime/ext_zend_compat/<ext_name>/*.c; do mv $i "$i"pp; done
```
Then create a system library at `runtime/ext_zend_compat/ext_<ext_name>.php`
This system library contains definitions for any functions and classes that have
C implementations in your extension. Type hinting is required, using the Hack
syntax documented at http://docs.hhvm.com/hack/overview/typing .
You can follow the examples from non-Zend extensions in HHVM -- however, note
that using types other than "mixed" is not as useful as it is in native HHVM
extensions. All parameters will be converted to variants before they are passed
to your extension, regardless of what type you specify.

All functions should have the `__Native("ZendCompat")` attribute. This causes
C implementation to be called with appropriate arguments. For example, to
create a function called "foo" that takes one parameter:

```<<__Native("ZendCompat")>> foo(mixed $a) : mixed;```

Internal classes should have the attribute `<<__NativeData("ZendCompat")>>`, for
example:

```
<<__NativeData("ZendCompat")>> class Foo {
  <<__Native("ZendCompat")>> function bar(mixed $a) : mixed;
}
```

Note that constructors and destructors should omit their types entirely.
These special methods will ignore any value populated into return_value.

```
<<__NativeData("ZendCompat">> class Foo {
  <<__Native("ZendCompat")>> function __construct(mixed $arg);
}
```

This causes the create_object function to be called and thus allows 
`zend_object_store_get_object()` to return a valid pointer.

## Things you have to fix in your code

* C++ compile errors
* Use `Z_RESVAL` instead of `Z_LVAL` for resource access
* Don't use `PHP_MALIAS`. Define the other function.
* Change any `ZVAL_STRING(foo, "literal string", 0)` to `ZVAL_STRING(foo, "literal string", 2)`
* Allocate hashtables with `ALLOC_HASHTABLE()` and zvals with `ALLOC_ZVAL()` or
  one of the macros that calls `ALLOC_ZVAL()`, don't allocate them directly.
  Don't use `malloc(sizeof(zval))` or create them on the stack.

## Bugs and caveats

* Many functions are missing (causing link errors) or have empty
  implementations.
* Object destructors are currently not called.
* The object handlers (`read_property`, `clone_obj`, etc.) are not called either.
* Most core globals, e.g. `SG(...)` are missing and will give a link error if
  referenced.
* The `EG(...)` globals are defined, but most of them aren't read or set, so will
  just contain null pointers.
* Persistent HashTables and zvals will not work.

## File structure

The `php-src` directory should exactly mirror Zend's directory layout.
The `.h` files are exact copies from there with minor edits wrapped in
`#define HHVM`. The `.cpp` files are inspired by the `.c` file with the same
name but will devaiate wildly.

The `hhvm` directory contains various glue code that is needed to be written but
doesn't have a Zend function with the same name.

Extensions are stored under their PECL name and are unmodified except for the
changes required above.

Tests go in either `test/zend` if they are bundled extensions or
`test/slow/ext-` for all other PECL ones.
