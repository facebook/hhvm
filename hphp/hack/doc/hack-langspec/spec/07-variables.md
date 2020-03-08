# Variables

## General

A *variable* is a named area of data storage that has a type and a
value. A variable is represented by a 
[VSlot](04-basic-concepts.md#general). A variable is created by [assigning a value to it](10-expressions.md#simple-assignment). A variable that somehow becomes defined, but is not initialized starts out with the value `null`.

Variables have [names](09-lexical-structure.md#names). Distinct variables may have
the same name provided they are in different [scopes](04-basic-concepts.md#scope).

A [constant](06-constants.md#general) is a variable that, once initialized, its value cannot
be changed. 

Based on the context in which it is declared, a variable has a 
[scope](04-basic-concepts.md#scope) and a [storage duration](04-basic-concepts.md#storage-duration).

The following kinds of variable may exist in a script:

-   [Local variable](07-variables.md#local-variables)
-   [Array element](07-variables.md#array-elements)
-   [Function static](07-variables.md#function-statics)
-   [Instance property](07-variables.md#instance-properties)
-   [Static property](07-variables.md#static-properties)
-   [Class and interface constant](07-variables.md#class-and-interface-constants)

## Kinds of Variables

### Local Variables

**Syntax**

See Semantics below.

**Semantics**

Except for a [function parameter](15-functions.md#general), a local variable is never defined explicitly; instead, it is created when it is first assigned a value. A local variable can be assigned to as a parameter in the parameter list of a [function definition](15-functions.md#function-definitions) or inside any [compound statement](11-statements.md#compound-statements). It has [function scope](04-basic-concepts.md#scope) and [automatic storage duration](04-basic-concepts.md#storage-duration). A local variable is a modifiable lvalue.

**Examples**

```Hack
function doit(bool $p1): void {  // assigned the value true when called
  $count = 10;
    ...
  if ($p1)
  {
    $message = "Can't open master file.";
    ...
  }
  ...
}
doit(true);
// -----------------------------------------
function f(): void
{
  $lv = 1;
  echo "\$lv = $lv\n";
  ++$lv;
}
for ($i = 1; $i <= 3; ++$i)
  f();
```

Unlike the [function static equivalent](07-variables.md#function-statics), function `f` outputs
"`$lv = 1`" each time.

See the [recursive function example](04-basic-concepts.md#storage-duration).

### Array Elements

**Syntax**

[Arrays](05-types.md#array-types) are created via the [array-creation operator](10-expressions.md#array-creation-operator) or
the intrinsic [`array`](10-expressions.md#array). At the same time, one or more elements
may be created for that array. New elements are inserted into an
existing array via the [simple-assignment operator](10-expressions.md#simple-assignment) in
conjunction with the [subscript operator `[]`](10-expressions.md#subscript-operator).

**Semantics**

The [scope](04-basic-concepts.md#scope) of an array element is the same as the scope of that
array's name. An array element has [allocated storage duration](04-basic-concepts.md#storage-duration).

**Examples**

```Hack
$colors = ["red", "white", "blue"]; // create array with 3 elements
$colors[] = "green";                // insert a new element
```

### Function Statics

**Syntax:**

<pre>
  <i>function-static-declaration:</i>
    static <i>static-declarator-list</i>  ;
  <i>static-declarator-list:</i>
    <i>static-declarator</i>  
    <i>static-declarator-list</i>  ,  <i>static-declarator</i> 
  <i>static-declarator:</i> 
    <i>variable-name</i>  <i>function-static-initializer<sub>opt</sub></i>
  <i>function-static-initializer:</i>
    = <i>const-expression</i>
</pre>

**Defined elsewhere**

* [*variable-name*](09-lexical-structure.md#names)
* [*constant-expression*](10-expressions.md#constant-expressions)

**Semantics**

A function static may be defined inside any [compound statement](11-statements.md#compound-statements).
It is a modifiable lvalue.

A function static has [function scope](04-basic-concepts.md#scope) and 
[static storage duration](04-basic-concepts.md#storage-duration).

The value of a function static is retained across calls to its parent
function. Each time the function containing a function static
declaration is called, that execution is dealing with an [alias](04-basic-concepts.md#general)
to that static variable. The next time that function
is called, a new alias is created.

**Examples**

```Hack
function f(): void {
  static $fs = 1;
  echo "\$fs = $fs\n";
  ++$fs;
}
for ($i = 1; $i <= 3; ++$i)
  f();
```

Unlike the [local variable equivalent](07-variables.md#local-variables), function `f` outputs "`$fs
= 1`", "`$fs = 2`", and "`$fs = 3`", as `$fs` retains its value across
calls.

### Instance Properties

These are described in [class instance properties section](16-classes.md#properties). They have [class scope](04-basic-concepts.md#scope) and [allocated storage duration](04-basic-concepts.md#storage-duration).

### Static Properties

These are described in [class static properties section](16-classes.md#properties). They have [class scope](04-basic-concepts.md#scope) and [static storage duration](04-basic-concepts.md#storage-duration).

### Class and Interface Constants

These are described in [class constants section](16-classes.md#constants) and [interface constants section](17-interfaces.md#constants). They have [class or interface scope](04-basic-concepts.md#scope) and [static storage duration](04-basic-concepts.md#storage-duration).
