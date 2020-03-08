# Attributes

## General

Programmers can invent a new kind of declarative information, called an *attribute*. Attributes can be attached to various program entities, and information about those attribute can be retrieved at run-time via reflection (see library class `Reflection`, et al).

Consider the following example:

```Hack
<<Help("http://www.MyOnlineDocs.com/Widget.html")>>
class Widget {
  …
}

$rc = new ReflectionClass('Widget');
$attrHelp = $rc->getAttribute('Help');
```

The method `getAttribute` returns an array containing the values corresponding to an attribute, in lexical order of their specification. As the `Help` attribute on class `Widget` has only one value, a string, array element 0 is the string containing that string, in this case, a URL at which the corresponding help information can be found.

A number of predefined attributes ([§§](21-attributes.md#predefined-attributes)) affect the way in which source code is compiled.

## Attribute Specification

**Syntax**
<pre>
<i>attribute-specification:</i>
  &lt;&lt;  <i>attribute-list</i>  &gt;&gt;
<i>attribute-list:</i>
  <i>attribute</i>
  <i>attribute-list</i>  ,  <i>attribute</i>
<i>attribute:</i>
  <i>attribute-name</i>  <i>attribute-value-list<sub>opt</sub></i>
<i>attribute-name:</i>
  <i>name</i>
<i>attribute-value-list:</i>
  (  <i>attribute-values<sub>opt</sub></i>  )
<i>attribute-values</i>
  <i>attribute-value</i>
  <i>attribute-values</i>  ,  <i>attribute-value</i>
<i>attribute-value:</i>
  <i>expression</i>
</pre>

*name* is defined in [§§](09-lexical-structure.md#names); and *expression* is defined in [§§](10-expressions.md#yield-operator).

**Constraints**

The *name*s in an *attribute-list* must be distinct.

The *expression*s in an *attribute-list* must be *constant-expressions* ([§§](10-expressions.md#constant-expressions)).

**Semantics**

A *name* in a user-defined attribute means whatever the programmer wants.

The *name*s do not have any scope, per se. They appear only in the context of an *attribute-specification*, and do not hide nor conflict with the same names used in other contexts.  

Omitting *attribute-value-list* is equivalent to specifying it without *attribute-values*.

**Examples**

```Hack
<<A1(123), A2>>
class C {
  <<A3(true, 100)>> public function __construct() { … }
  <<A4>> public function doit(): void { … }
}
```

## Predefined Attributes

### General

The following attributes are recognized by a conforming implementation:
* `__ConsistentConstruct` ([§§](21-attributes.md#attribute-__consistentconstruct))
* `__Memoize` ([§§](21-attributes.md#attribute-__memoize))
* `__Override` ([§§](21-attributes.md#attribute-__override))

### Attribute `__ConsistentConstruct`

This attribute can be applied to classes; it has no attribute values.

When a method is overridden in a derived class, it must have exactly the same number, type, and order of parameters as that in the base class ([§§](16-classes.md#general)). However, that is not the case for constructors ([§§](16-classes.md#constructors)). Having a family of constructors with different signatures can cause a problem, however, especially when using `new static` ([§§](10-expressions.md#the-new-operator)).

Consider the following example:

```Hack
<<__ConsistentConstruct>>
class Base {
  public function __construct() { … }

  public static function make(): this {
    return new static();
  }
}

class Derived extends Base {
  public function __construct() {
    …
    parent::__construct();
  }
}

$v2 = Derived::make();
```

When `make` is called on a `Derived` object, `new static` results in `Derived`'s constructor being called knowing only the parameter list of `Base`'s constructor. As such, `Derived`'s constructor must either have the exact same signature as `Base`'s constructor, or the same plus an ellipses ([§§](15-functions.md#function-definitions)) indicating a trailing variable-argument list.

### Attribute `__Memoize`

This attribute can be applied to functions and static or instance methods; it has no attribute values.

The presence of this attribute causes the designated method to cache automatically each value it looks up and returns, so future requests for that same lookup can be retrieved more efficiently. The set of parameters is hashed into a single hash key, so changing the type, number, and/or order of the parameters can change that key.

Consider the following example:

```Hack
class Item {
  <<__Memoize>>
  public static function getNameFromProductCode(int $productCode): string {
    return Item::getNameFromStorage($productCode);  // conditional call
  }
  private static function getNameFromStorage(int $productCode): string {
    // get name from alternate store
    return …;
  }
}
```

`Item::getNameFromStorage` will only be called if the given product code is not in the cache.

The types of the parameters are restricted to the following: `null`, `bool`, `int`, `float`, `string`, any object type that implements `IMemoizeParaminterface`, enum constants, tuples, shapes, and arrays/collections containing any supported element type.

The interface type `IMemoizeParam` ([§§](17-interfaces.md#interface-IMemoizeParam)) assists with memorizing objects passed to async functions.

### Attribute `__Override`

This attribute can be applied to static or instance methods; it has no attribute values.

The presence of this attribute indicates that the designated method is intended to override a method having the same name in a direct or indirect base class. If no such base-class method exists, a compile-time error occurs.

Consider the following example:

```Hack
class Button {
  public function draw(): void { /* … */ }
}
class CustomButton extends Button {
  <<__Override>>
  public function draw(): void { /* … */ }
}
```

If a subsequent refactoring of class `Button` results in the removal of method draw, the presence of the attribute in class `CustomButton` will cause the dependence to be reported.

When `__Override` is applied to a method in a trait, the check for whether the overridden method exists takes place when a class uses that trait.

