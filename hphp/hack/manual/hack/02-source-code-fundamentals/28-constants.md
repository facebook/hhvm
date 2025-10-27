# Constants

A constant is a named value. Once defined, the value of the constant *cannot* be changed.  A constant can be defined in one
of two ways: using the `const` keyword at the top level, or inside a class or
interface. For example:

```hack
const int MAX_COUNT = 123;
class C {
  const float MAX_HEIGHT = 10.5;
  const float UPPER_LIMIT = C::MAX_HEIGHT;
}

<<__EntryPoint>>
function main(): void {
  echo "MAX_COUNT = ".MAX_COUNT."\n";
  echo "MAX_HEIGHT = ".(string)C::MAX_HEIGHT."\n";
}
```

## Context-Dependent Constants

The following constants --- sometimes referred to as *magic constants* --- are automatically available to all scripts; their values
are not fixed:

 Constant Name                    | Description
 -----------------                | ---------
`__CLASS__`                       | `string`; The name of the current class. From within a trait method, the name of the class in which that trait is used. If the current namespace is a defined, named namespace (not in root), the namespace name and "\\" are prepended, in that order. If used outside all classes, the value is the empty string.
`__DIR__`                         | `string`; The directory name of the script. A directory separator is only appended for the root directory.
`__FILE__`                        | `string`; The full name of the script.
`__FUNCTION__`                    | `string`; Inside a function, the name of the current function exactly as it was declared, with the following prepended: If a named namespace exists, that namespace name followed by "\". If used outside all functions, the result is the empty string. For a method, no parent-class prefix is present. (See `__METHOD__` and [anonymous functions](/hack/functions/anonymous-functions).)
`__LINE__`                        | `int`; the number of the current source line
`__METHOD__`                      | `string`; Inside a method, the name of the current method exactly as it was declared, with the following prepended, in order: If a named namespace exists, that namespace name followed by "\"; the parent class name or trait name followed by `::`. If used outside all methods, the result is the same as for `__FUNCTION__`.
`__NAMESPACE__`                   | `string`; The name of the current namespace exactly as it was declared. For the root namespace, the result is the empty string.
`__TRAIT__`                       | `string`; The name of the current trait. From within a trait method, the name of the current trait. If used outside all traits, the result is the empty string.

## Core Predefined Constants

Namespace HH\Lib\Math contains a number of integer-related constants (such as `INT64_MAX` and `INT64_MIN`).
