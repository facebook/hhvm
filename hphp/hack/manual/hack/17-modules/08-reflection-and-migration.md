## Reflection

You can reflect on the module of a class or function using its corresponding methods.

```Hack no-extract
ReflectionClass::getModule();
ReflectionFunctionAbstract::getModule();
```

You can check if a class, method or property is internal with the `isInternalToModule()` function.

```hack no-extract
ReflectionClass::isInternalToModule();
ReflectionFunctionAbstract::isInternalToModule();
```

## Migration and <<__SoftInternal>>
When migrating existing code to use internal, you can use the <<__SoftInternal>> attribute to help detect dynamic usages of the code outside of the module.
Internal symbols with <<__SoftInternal>> will raise a warning in HHVM instead of an exception.

```hack
//// newmodule.hack
new module foo {}

//// foo.hack
module foo;
class Cls {
  <<__SoftInternal>>
  internal function foo_soft(): void {
    echo "Hello from foo_soft\n";
  }
}
```

Calling Cls::foo_soft() from outside the code:

```hack no-extract
module bar;
<<__EntryPoint>>
function test(): void {
    Cls::foo_soft();
}
```
will result in the following output from HHVM:

```
Warning: Accessing soft internal method Cls::foo_soft in module foo from module bar is not allowed in test.php on line 4
Hello from foo_soft
```
