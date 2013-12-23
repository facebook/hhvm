# Zend Extension Source Compatability Layer

If you want to compile your existing zend extension against HHVM, you can use
these headers. The runtimes are similar enough that we can just map the macros
to our data structures and it mostly works.

## Migration Steps

```sh
cp -R <zend_extension_dir> runtime/ext_zend_compat/<ext_name>

# move all the .c to .cpp
for i in runtime/ext_zend_compat/<ext_name>/*.c; do mv $i "$i"pp; done

# If your extension has docs on php.net you can make the idl like this:
cd system/idl
php newexp.php <ext_name>
# Otherwise you have to make the .idl by hand
```

<setup the build environment>

## Things you have to fix in your code

* C++ compile errors
* Use `Z_RESVAL` instead of `Z_LVAL` for resource access
* Don't use `PHP_MALIAS`. Define the other function.
* Change any `ZVAL_STRING(foo, "literal string", 0)` to `ZVAL_STRING(foo, "literal string", 2)`

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
