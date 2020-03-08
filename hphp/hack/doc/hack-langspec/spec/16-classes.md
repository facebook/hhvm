# Classes

## General

A class is a type that may contain zero or more explicitly declared
*members*, which can be any combination of [*class constants*](16-classes.md#constants);
data members, called [*properties*](16-classes.md#properties); function members, called
[*methods*](16-classes.md#methods); and [*type constants*](16-classes.md#type-constants). (The ability to add methods to an
instance at runtime is described in [§§](16-classes.md#dynamic-methods).) An object (often called an
*instance*) of a class type is created (i.e., *instantiated*) via the
[new operator](10-expressions.md#the-new-operator).

Hack supports [inheritance](16-classes.md#class-declarations), a means by which a *derived class* can
*extend* and specialize a single *base class*. However, unlike numerous
other languages, classes in Hack are **not** all derived from a common
ancestor. An [*abstract* class](16-classes.md#class-declarations) is a base type intended for
derivation, but which cannot be instantiated directly. A *concrete*
class is a class that is not abstract. A [*final* class](16-classes.md#class-declarations) is one
from which other classes cannot be derived.

A class may *implement* one or more *interfaces* ([§§](16-classes.md#class-declarations), [§§](17-interfaces.md#general)), each of
which defines a contract.

A class can *use* one or more [traits](18-traits.md#general), which allows a class to
have some of the benefits of multiple inheritance.

A [*constructor*](16-classes.md#constructors) is a special method that is used to initialize
an instance immediately after it has been created. A [*destructor*](16-classes.md#destructors) is a special method that is used to free resources when an
instance is no longer needed. Other special methods exist; they are
described in ([§§](16-classes.md#methods-with-special-semantics)).

The members of a class each have a default or explicitly declared
*visibility*, which decides what source code can access them. A
member with `private` visibility may be accessed only from within its own
class. A member with `protected` visibility may be accessed only from
within its own class and from classes derived from that class. Access to
a member with `public` visibility is unrestricted. 

The *signature* of a method is a combination of the parent class name,
that method's name, and its parameter types.

The members of a base class can be *overridden* in a
derived class by redeclaring them with the same signature defined in the
base class. However, overridden constructors are exempt from this requirement ([§§](16-classes.md#constructors)).

When an instance is allocated, new returns a handle that points to that
object. As such, assignment of a handle does not copy the object itself.
(See [§§](04-basic-concepts.md#cloning-objects) for a discussion of shallow and deep copying.)

## Class Declarations

**Syntax**

<pre>
  <i>class-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  <i>class-modifier<sub>opt</sub></i>  class  <i>name  generic-type-parameter-list<sub>opt</sub></i>  <i>class-base-clause<sub>opt</sub></i>
      <i>class-interface-clause<sub>opt</sub></i>  {  <i>trait-use-clauses<sub>opt</sub>  class-member-declarations<sub>opt</sub></i>  }

  <i>class-modifier:</i>
    abstract
    final
    abstract final

  <i>class-base-clause:</i>
    extends  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>

  <i>class-interface-clause:</i>
    implements  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
    <i>class-interface-clause</i>  ,  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
</pre>

**Defined elsewhere**

* [*attribute-specification*](21-attributes.md#attribute-specification)
* [*class-member-declarations*](16-classes.md#class-members)
* [*generic-type-argument-list*](14-generic-types-methods-and-functions.md#type-parameters)
* [*generic-type-parameter-list*](14-generic-types-methods-and-functions.md#type-parameters)
* [*name*](09-lexical-structure.md#names)
* [*qualified-name*](20-namespaces.md#defining-namespaces)
* [*trait-use-clauses*](18-traits.md#trait-declarations)

**Constraints**

A class must not be derived directly or indirectly from itself.

A *class-declaration* containing any *class-member-declarations* that
have the modifier `abstract` must itself have an `abstract`
*class-modifier*.

*class-base-clause* must not name a final class.

*qualified-name* in *class-base-clause* must name a class type, and must
not be `parent`, `self`, or `static`.

A concrete class must implement each of the methods from all the
[interfaces](17-interfaces.md#general) specified in *class-interface-clause*, using the
exact same signature as defined in each interface.

*qualified-name* in *class-interface-clause* must name an interface
type.

A generic class and a non-generic class in the same scope cannot have the same *name*.

**Semantics**

A *class-declaration* defines a class type by the name *name*. Class
names are [case-preserved](03-terms-and-definitions.md).

The `abstract` modifier declares a class usable only as a base class; the
class cannot be instantiated directly. An abstract class may contain one
or more abstract members, but it is not required to do so. When a
concrete class is derived from an abstract class, the concrete class
must include an implementation for each of the abstract members it
inherits.

The `final` modifier prevents a class from being used as a base class.

The optional *class-base-clause* specifies the one base class from which
the class being defined is derived. In such a case, the derived class
inherits all the members from the base class.

The optional *class-interface-clause* specifies the one or more
interfaces that are implemented by the class being defined.

A class is required to implement a given interface in either of the following cases:
* If its *class-declaration* specifies that interface directly in a *class-interface-clause*; or
* If that class contains a *trait-use-clause* for a trait whose *trait-declaration* contains a *class-interface-clause* naming that interface; or
* Both.

A class can use one or more traits via a *trait-use-clauses*; see [§§](18-traits.md#general)
and [§§](18-traits.md#trait-declarations).

**Examples**

```Hack
abstract class Vehicle {
  public abstract function getMaxSpeed(): int;
  …
}
abstract class Aircraft extends Vehicle {
  public abstract function getMaxAltitude(): int;
  …
}
class PassengerJet extends Aircraft {
  public function getMaxSpeed(): int {
    // implement method
  }
  public function getMaxAltitude(): int {
    // implement method
  }
  …
}
$pj = new PassengerJet(…);
echo "\$pj's maximum speed: " . $pj->getMaxSpeed() . "\n";
echo "\$pj's maximum altitude: " . $pj->getMaxAltitude() . "\n";
// -----------------------------------------
final class MathLibrary {
  private function MathLibrary() {} // disallows instantiation
  public static function sin(float $p): float { … }
  …
}
$v = MathLibrary::sin(2.34);
// -----------------------------------------
interface MyCollection {
  function put(int $item): void;
  function get(): int;
}
class MyList implements MyCollection {
  public function put(int $item): void  {
    // implement method
  }
  public function get(): int {
    // implement method
  }
  …
}
```

## Class Members

**Syntax**

<pre>
  <i>class-member-declarations:</i>
    <i>class-member-declaration</i>
    <i>class-member-declarations   class-member-declaration</i>

   <i>class-member-declaration:</i>
     <i>const-declaration</i>
     <i>property-declaration</i>
     <i>method-declaration</i>
     <i>constructor-declaration</i>
     <i>destructor-declaration</i>
     <i>type-constant-declaration</i>
</pre>

**Defined elsewhere**

* [*const-declaration*](16-classes.md#constants)
* [*constructor-declaration*](16-classes.md#constructors)
* [*destructor-declaration*](16-classes.md#destructors)
* [*method-declaration*](16-classes.md#methods)
* [*property-declaration*](16-classes.md#properties)
* [*type-constant-declaration*](16-classes.md#type-constants)

**Semantics**

The members of a class are those specified by its
*class-member-declaration*s, and the members inherited from its base
class, from the interfaces it implements, and from the traits that it uses. (A class may also contain dynamic members, as described in [§§](16-classes.md#dynamic-methods).
However, as these have no compile-time names, they can only be accessed
via method calls.)

A class may contain the following members:

* [Constants](16-classes.md#constants) – the constant values associated with the class.
* [Properties](16-classes.md#properties) – the variables of the class.
* [Methods](16-classes.md#methods) – the computations and actions that can be performed by the class (see also [§§](16-classes.md#methods-with-special-semantics)).
* [Constructor](16-classes.md#constructors) – the actions required to initialize an instance of the class.
* [Destructor](16-classes.md#destructors) – the actions to be performed when an instance of the class is no longer needed.
*	[Type constant](16-classes.md#type-constants) – a way of parameterizing class types without using generics.

A number of names are reserved for methods with special semantics, which
user-defined versions must follow. These are described in ([§§](16-classes.md#methods-with-special-semantics)).

Methods and properties can either be *static* or *instance* members. A
static member is declared using `static`. An instance member is one that
is not static. The name of a static method or property can never be used
on its own; it must always be used as the right-hand operand of the
[scope resolution operator](10-expressions.md#scope-resolution-operator). The name of an instance method or
property can never be used on its own; it must always be used as the
right-hand operand of the[ member selection operator](10-expressions.md#member-selection-operator).

Each instance of a class contains its own, unique set of instance
properties of that class. An instance member is accessed via the
[`->` operator](10-expressions.md#member-selection-operator). In contrast, a static property designates
exactly one VSlot for its class, which does not belong to any instance,
per se. A static property exists whether or not any instances of that
class exist. A static member is accessed via the [`::` operator](10-expressions.md#scope-resolution-operator).

When any instance method operates on a given instance of a class, within
that method that object can be accessed via [`$this`](10-expressions.md#general-1). As a
static method does not operate on a specific instance, it has no `$this`.

**Examples**

```Hack
class Point {
  private static int $pointCount = 0;     // static property

  private float $x;           // instance property
  private float $y;           // instance property

  public static function getPointCount(): int { // static method
    return self::$pointCount;     // access static property
  }
  public function move(num $x, num $y): void {// instance method
    $this->x = (float)$x;
    $this->y = (float)$y;
  }
  public function __construct(num $x = 0, num $y = 0) { // instance method
    $this->x = (float)$x;         // access instance property
    $this->y = (float)$y;         // access instance property
    ++self::$pointCount;        // access static property
  }
  public function __destruct() {        // instance method
    --self::$pointCount;        // access static property
    …
  }
  …
}
echo "Point count = " . Point::getPointCount() . "\n";
$cName = 'Point';
echo "Point count = " . $cName::getPointCount() . "\n";
```

## Dynamic Methods

Ordinarily, all of the instance methods of a class are
declared explicitly in that class's definition. However, under certain circumstances, *dynamic
methods* can be added to a particular instance of a class or to the
class as a whole at runtime.

With a dynamic method, no method is really added to the
instance or the class. However, the illusion of doing that is achieved
by allowing a call to an instance or static method, but one which is not
declared in that instance's class, to be accepted, intercepted by a
method called [`__call`](16-classes.md#method-__call) or [`__callStatic`](16-classes.md#method-__callstatic), and
dealt with under program control. 

Consider the following code fragment, in which class Widget has neither
an instance method called `iMethod` nor a static method called `sMethod`,
but that class has made provision to deal with dynamic methods:

```Hack
$obj = new Widget();
$obj->iMethod(10, true, "abc");
Widget::sMethod(null, 1.234);
```

The call to `iMethod` is treated as if it were

```Hack
$obj->__call('iMethod', array(10, true, "abc"))
```

and the call to `sMethod` is treated as if it were

```Hack
Widget::__callStatic('sMethod', array(null, 1.234))
```

## Constants

**Syntax**

<pre>
  <i>const-declaration:</i>
    abstract<i><sub>opt</sub></i>  const  <i>type-specifier<sub>opt</sub></i>  <i>constant-declarator-list</i>  ;
  <i>constant-declarator-list:</i>
    <i>constant-declarator</i>
    <i>constant-declarator-list</i>  ,  <i>constant-declarator</i>
  <i>constant-declarator:</i>
    <i>name</i>  <i>constant-initializer<sub>opt</sub></i>

  <i>constant-initializer:</i>
    =  <i>const-expression</i>
</pre>

**Defined elsewhere**

* [*const-expression*](10-expressions.md#constant-expressions)
* [*name*](09-lexical-structure.md#names)
* [*type-specifier*](05-types.md#general)

**Constraints**

A *const-declaration* must be
a *class constant* (inside a [*class-definition*](16-classes.md#class-members) or be an
*interface constant* (inside an [*interface-definition*](17-interfaces.md#interface-members).

If `abstract` is present, no *constant-initializer*s are permitted. If `abstract` is absent, each *constant-declarator* must have a *constant-initializer*.

**Semantics**

A *const-declaration* defines a c-constant.

All class constants have public visibility.

All constants are implicitly `static`.

If *type-specifier* is omitted, the type is inferred from *const-expression*.

Note: Although the grammar allows a class constant to have any type, there is no way to write a *constant-expression* for closures, tuples, or shapes.

**Examples:**

```Hack
class Automobile {
  const DEFAULT_COLOR = "white";
  …
}
$col = Automobile::DEFAULT_COLOR;
```

## Properties

**Syntax**

<pre>
  <i>property-declaration:</i>
    <i>property-modifier</i>  <i>type-specifier</i>  <i>property-declarator-list</i>  ;

  <i>property-declarator-list:</i>
    <i>property-declarator</i>
    <i>property-declarator-list</i>  ,  <i>property-declarator</i>

  <i>property-declarator:</i>
    <i>variable-name</i>  <i>property-initializer<sub>opt</sub></i>

  <i>property-modifier:</i>
    <i>visibility-modifier</i>  <i>static-modifier<sub>opt</sub></i>
    <i>static-modifier</i>  <i>visibility-modifier</i>

  <i>visibility-modifier:</i>
    public
    protected
    private

  <i>static-modifier:</i>
    static

  <i>property-initializer:</i>
    =  <i>expression</i>
</pre>

**Defined elsewhere**

* [*constant-expression*](10-expressions.md#yield-operator)
* [*type-specifier*](05-types.md#general)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

A static property cannot have a *type-specifier* of the form `this` `::` *name*.

A static property cannot have a *type-specifier* of `this` or `?this`.

**Semantics**

A *property-declaration* defines an instance or static property called *variable-name*.

The visibility modifiers are described in [§§](16-classes.md#general). The `static` modifier is described in [§§](16-classes.md#class-members).

The *property-initializer*s for instance properties are applied prior to
the class's constructor being called. 

All properties of non-nullable type must be initialized explicitly either by a *property-initializer* or by the constructor. Properties of nullable type that are not explicitly initialized take on the value `null`.

**Examples**

```Hack
class Point {
  private static int $pointCount = 0; // static property with initializer
  private float $x;         // instance property
  private float $y;         // instance property
  …
}
```

## Methods

**Syntax**

<pre>
  <i>method-declaration:</i>
p
  <i>attribute-specification<sub>opt</sub></i> <i>method-modifiers</i>  <i>function-definition-no-attribute</i>
  <i>attribute-specification<sub>opt</sub></i> <i>method-modifiers</i>  <i>function-definition-header</i>  ;

  <i>method-modifiers:</i>
    <i>method-modifier</i>
    <i>method-modifiers</i>  <i>method-modifier</i>

  <i>method-modifier:</i>
    <i>visibility-modifier</i>
    <i>static-modifier</i>
    abstract
    final
</pre>

**Defined elsewhere**

* [*function-definition*](15-functions.md#function-definitions)
* [*function-definition-header*](15-functions.md#function-definitions)
* [*static-modifier*](16-classes.md#class-members)
* [*visibility-modifier*](16-classes.md#general)

**Constraints**

When defining a concrete class that inherits from an abstract class, the
definition of each abstract method inherited by the derived class must
have the same or a
less-restricted [visibility](http://docs.hhvm.com/manual/en/language.oop5.visibility.php)
than in the corresponding abstract declaration. Furthermore, the
signature of a method definition must match that of its abstract
declaration.

The *method-modifiers* preceding a *function-definition* must not contain
the `abstract` modifier.

The *method-modifiers* preceding a *function-definition-header* must
contain the `abstract` modifier.

A method must not have the same modifier specified more than once. A
method must not have more than one *visibility-modifier*. A method must
not have both the modifiers `abstract` and `private`, or `abstract` and `final`.

An abstract method must not also be asynchronous. However, an abstract method can have a return-type of `Awaitable<T>`, so an async concrete implementation can be provided.

**Semantics**

A *method-declaration* defines an instance or static method. A method is
a function that is defined inside a class. However, the presence of
`abstract` indicates an abstract method, in which case, no implementation
is provided. The absence of `abstract` indicates a concrete method, in
which case, an implementation is provided.

The presence of `final` indicates the method cannot be overridden in a
derived class.

**Examples**

See [§§](16-classes.md#class-members) for examples of instance and static methods. See [§§](16-classes.md#class-declarations) for
examples of abstract methods and their subsequent definitions.

## Constructors

**Syntax**

<pre>
  <i>constructor-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  <i>constructor-modifiers</i>  function  __construct  (
      <i>constructor-parameter-declaration-list<sub>opt</sub></i>  ) <i>void-return<sub>opt</sub></i>  <i>compound-statement</i>
  <i>constructor-parameter-declaration-list:</i>
    <i>constructor-parameter-declaration</i>
    <i>constructor-parameter-declaration-list</i>  ,  <i>constructor-parameter-declaration</i>
  <i>constructor-parameter-declaration:</i>
    <i>visibility-modifier<sub>opt</sub></i>  <i>type-specifier</i>  <i>variable-name</i> <i>default-argument-specifier<sub>opt</sub></i>
  <i>constructor-modifiers:</i>
    <i>constructor-modifier</i>
    <i>constructor-modifiers</i>  <i>constructor-modifier</i>

  <i>constructor-modifier:</i>
    <i>visibility-modifier</i>
    abstract
    final
</pre>

**Defined elsewhere**

* [*attribute-specification*](21-attributes.md#attribute-specification)
* [*compound-statement*](11-statements.md#compound-statements)
* [*default-argument-specifier*](14-generic-types-methods-and-functions.md#type-constraints)
* [*type-specifier*](05-types.md#general)
* [*variable-name*](09-lexical-structure.md#names)
* [*visibility-modifier*](16-classes.md#properties)

**Constraints**

An overriding constructor in a derived class must have the same or a
less-restricted [visibility](http://docs.hhvm.com/manual/en/language.oop5.visibility.php)
than that being overridden in the base class.

The *variable-name* in a *constructor-parameter-declaration* containing a *visibility-modifier* must not be the same as an explicitly declared property in this class.

*compound-statement* must not contain a call to a non-private method in the same class.

**Semantics**

A constructor is a specially named [instance method](16-classes.md#methods) that is used
to initialize an instance immediately after it has been created. Any
instance properties of nullable type not explicitly initialized by a constructor take on
the value `null`.

Constructors can be overridden in a derived class by redeclaring them.
However, an overriding constructor need not have the same signature as
defined in the base class. 

Constructors are called by [*object-creation-expression*s](10-expressions.md#the-new-operator)
and from within other constructors.

The `abstract` and `final` modifiers are described in [§§](16-classes.md#methods).

If classes in a derived-class hierarchy have constructors, it is the
responsibility of the constructor at each level to call the constructor
in its base-class explicitly, using the notation
`parent::__construct(...)`. If a constructor calls its base-class
constructor, it should do so as the first statement in
*compound-statement*, so the object hierarchy is built from the
bottom-up. A constructor should not call its base-class constructor more
than once. A call to a base-class constructor searches for the nearest
constructor in the class hierarchy. Not every level of the hierarchy
need have a constructor.

When a *constructor-parameter-declaration* contains a *visibility-modifier*, a property called *variable-name* with visibility *visibility-modifier* is added to the current class. When that constructor is called, the value of the argument for that parameter is assigned to the added property. Note: This feature simply provides a programming shortcut by having the implementation declare and initialize such properties.

A constructor does not require a return type annotation; if one is included, it must be void.

**Examples**

```Hack
class Point {
  private static int $pointCount = 0;
  public function __construct(private float $x = 0.0, private float $y = 0.0) {
    // private properties $x and $y are created and initialized
        ++self::$pointCount;
  }
  public function __destruct() {
    --self::$pointCount;
    …
  }
  …
}
// -----------------------------------------
class MyRangeException extends Exception {
  public function __construct(string $message, /* whatever */ ) {
    parent::__construct($message);
    …
  }
  …
}
```

## Destructors

**Syntax**

<pre>
  <i>destructor-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  <i>visibility-modifier</i>  function  __destruct  ( )  <i>void-return<sub>opt</sub></i>  <i>compound-statement</i>

  <i>void-return:</i>
    : void

</pre>

**Defined elsewhere**

* [*attribute-specification*](21-attributes.md#attribute-specification)
* [*compound-statement*](11-statements.md#compound-statements)
* [*visibility-modifier*](16-classes.md#general)

**Constraints**

An overriding destructor in a derived class must have the same or a
less-restricted [visibility](http://www.php.net/manual/en/language.oop5.visibility.php)
than that being overridden in the base class.

**Semantics**

A destructor is a special-named [instance method](16-classes.md#methods) that is used to
free resources when an instance is no longer needed. The destructors for
instances of all classes are called automatically once there are no
handles pointing to those instances or in some unspecified order during
program shutdown.

Destructors can be overridden in a derived class by redeclaring them. 

Destructors are called by the Engine or from within other destructors.

If classes in a derived-class hierarchy have destructors, it is the
responsibility of the destructor at each level to call the destructor in
the base-class explicitly, using the notation `parent::__destruct()`. If
a destructor calls its base-class destructor, it should do so as the
last statement in *compound-statement*, so the object hierarchy is
destructed from the top-down. A destructor should not call its
base-class destructor more than once. A call to a base-class destructor
searches for the nearest destructor in the class hierarchy. Not every
level of the hierarchy need have a destructor. A `private` destructor
inhibits destructor calls from derived classes.

A destructor does not require a return type annotation; if one is included, it must be void.

**Examples**

See [§§](#constructors) for an example of a constructor and destructor.

## Type Constants

**Syntax**

<pre>
  <i>type-constant-declaration:</i>
    <i>abstract-type-constant-declaration</i>
    <i>concrete-type-constant-declaration</i>
  <i>abstract-type-constant-declaration:</i>
    abstract  const  type  <i>name</i>  <i>type-constraint<sub>opt</sub></i>  ;
  <i>concrete-type-constant-declaration:</i>
    const  type  name  <i>type-constraint<sub>opt</sub></i>  =  <i>type-specifier</i>  ;
</pre>

**Defined elsewhere**

* [*name*](09-lexical-structure.md#names)
* [*type-constraint*](05-types.md#general)
* [*type-specifier*](05-types.md#general)

**Constraints**

A class or interface must not have multiple *type-constant-declaration*s for the same *name*.

A class having an *abstract-type-constant-declaration* must be abstract. (Any class that defines an abstract type constant must be abstract itself, and any class that inherits an abstract type constant must be either abstract itself or override it with a concrete type constant.

An interface must not contain a *concrete-type-constant-declaration* that has a *type-constraint*.

When overriding a type constant having a *type-constraint*, the overriding type must be a subtype of the constraint type.

A concrete type constant without a *type-constraint* cannot be overridden.

*type-specifier* cannot designate a class type parameter.

By convention, *name* should begin with an uppercase `T`.

**Semantics**

Type constants provide an alternative to [parameterized types](14-generic-types,-methods,-and-functions.md#general). For example, a base class can define an abstract type constant---essentially a name without a concrete type attached---and subclasses each override that with a concrete type constant. Conceptually, type constants are to types, as abstract methods are to methods.

A type constant has public [visibility](16-classes.md#general) and is implicitly [static](16-classes.md#class-members).

*type-constraint* restricts the type by which a type constant can be overridden.

In an *abstract-type-constant-declaration*, the absence of a *type-constraint* is the same as the presence of one with a type of `mixed`. In a *concrete-type-constant-declaration*, the absence of a *type-constraint* prevents name from being overridden.

**Examples**

```Hack
interface I1 {
  abstract const type T1 as arraykey;
  public function getID1(): this::T1;
  const type T4a = int;
}

interface I2 {
  abstract const type T2;
  public function getID2(this::T2 $p): void;
  abstract const type T3;
  const type T4c = string;
}

interface I3 extends I2 {
  public function f(this::T2 $p): void;
}

class Ctc6 implements I1, I2 {
  const type T1 = int;
  public function __construct(private this::T1 $id) {}
  public function getID1(): this::T1 {
    return $this->id;
  }
  const type T2 = string;
  public function getID2(this::T2 $p): void {}
  const type T3 = float;
  public function f(this::T4a $p1, this::T4c $p2): void {}
}
// -----------------------------------------
abstract class CBase {
  abstract const type T;
  public function __construct(protected this::T $value) {}
}

class Cstring extends CBase {
  const type T = string;
  public function getString(): string {
    return $this->value;	// gets the string
  }
}

class Cint extends CBase {
  const type T = int;
  public function getInt(): int {
    return $this->value;	// gets the int
  }
}

function run2(): void {
  var_dump((new Cstring('abc'))->getString());
  var_dump((new Cint(123))->getInt());
}
```

## Methods with Special Semantics

### General

If a class contains a definition for a method having one of the
following names, that method must have the prescribed visibility,
signature, and semantics:

Method Name	| Description
------------|-------------
[`__call`](16-classes.md#method-__call) | Calls a dynamic method in the context of an instance-method call
[`__callStatic`](16-classes.md#method-__callstatic) | Calls a dynamic method in the context of a static-method call
[`__clone`](16-classes.md#method-__clone) | Typically used to make a deep copy of an object
[`__construct`](16-classes.md#constructors) | A constructor
[`__destruct`](16-classes.md#destructors) | A destructor
[`__sleep`](16-classes.md#method-__sleep) | Executed before [serialization](16-classes.md#serialization) of an instance of this class
[`__toString`](16-classes.md#method-__tostring) | Returns a string representation of the instance on which it is called
[`__wakeup`](16-classes.md#method-__wakeup) | Executed after [unserialization](16-classes.md#serialization) of an instance of this class

### Method `__call`

**Syntax**

<pre>
  public  function  __call  (  string  <i>$name</i>  ,  array&lt;mixed&gt;  <i>$arguments</i>  )  :  mixed  <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](11-statements.md#compound-statements)

**Semantics**

This instance method is called to invoke the [dynamic method](16-classes.md#dynamic-methods)
designated by `$name` using the arguments specified by the elements of
the array designated by `$arguments`. It can return any value deemed
appropriate.

Typically, `__call` is called implicitly, when the [`->` operator](10-expressions.md#member-selection-operator) is used to call an instance method that is not visible.

While a method-name source token has a prescribed syntax, there are no
restrictions on the spelling of the dynamic method name designated by
*$name*. Any source character is allowed here.

**Examples**

```Hack
class Widget {
  public function __call(string $name, array<mixed> $arguments): mixed {
    // using the method name and argument list, redirect/process
    // the method call, as desired.
  }
  …
}
$obj = new Widget;
$obj->iMethod(10, true, "abc"); // $obj->__call('iMethod', array(…))
```

### Method `__callStatic`

**Syntax**

<pre>
  public  static  function  __callStatic  (  string  <i>$name</i>  ,  array&lt;mixed&gt;  <i>$arguments</i>  )  :  mixed
    <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](11-statements.md#compound-statements)

**Semantics**

This static method is called to invoke the [dynamic method](16-classes.md#dynamic-methods)
designated by `$name` using the arguments specified by the elements of
the array designated by `$arguments`. It can return any value deemed
appropriate.

Typically, `__callStatic` is called implicitly, when the [`::` operator](10-expressions.md#scope-resolution-operator) is used to call a static method that is not visible. Now while
`__callStatic` can be called explicitly, the two scenarios do not
necessarily produce the same result. Consider the expression `C::m(...)`,
where `C` is a class and `m` is a static-method name. If `m` is the name of a
visible method, `C::m(...)` does not result in `__callStatic`'s being
called. Instead, the visible method is used. On the other hand, the
expression `C::__callStatic('m',array(...))` always calls the named
dynamic method, ignoring the fact that a static visible method having
the same name might exist. If m is not the name of a visible method, the
two expressions are equivalent; that is; when handling `C::m(...)`, if no
visible method by that name is found, a dynamic method is assumed, and
`__callStatic` is called. (Note: While it would be unusual to create
deliberately a static dynamic method with the same name as a static
visible one, the visible method might be added later. This name
"duplication" is convenient when adding a dynamic method to a class
without having to worry about a name clash with any method names that
class inherits.)

While a method-name source token has a prescribed syntax, there are no
restrictions on the spelling of the dynamic method name designated by
`$name`. Any source character is allowed here.

**Examples**

```Hack
class Widget {
  public static function __callStatic(string $name,
    array<mixed> $arguments): mixed {
    // using the method name and argument list, redirect/process
    // the method call, as desired.
  }
  …
}
Widget::sMethod(null, 1.234); // Widget::__callStatic('sMethod', array(…))
```

### Method `__clone`

**Syntax**

<pre>
  public  function  __clone  (  )  :  void  <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](11-statements.md#compound-statements)

**Semantics**

This instance method is called by the [`clone` operator](10-expressions.md#the-clone-operator),
(typically) to make a [deep copy](04-basic-concepts.md#cloning-objects) of the current class component of the instance on which it is
called. (Method `__clone` cannot be called directly by the program.) 

Consider a class `Employee`, from which is derived a class `Manager`. Let us
assume that both classes contain properties that are objects. To make a
copy of a `Manager` object, its `__clone` method is called to do whatever
is necessary to copy the properties for the `Manager` class. That method
should, in turn, call the `__clone` method of its parent class,
`Employee`, so that the properties of that class can also be copied (and
so on, up the derived-class hierarchy).

To clone an object, the `clone` operator makes a [shallow copy](04-basic-concepts.md#cloning-objects)) of the object on which it is called.
Then, if the class of the instance being cloned has a method called
`__clone`, that method is automatically called to make a deep copy.
Method `__clone` cannot be called directly from outside a class; it can
only be called by name from within a derived class, using the notation
`self::__clone()`. This method can return a value; however, if it does
so and control returns directly to the point of invocation via the `clone`
operator, that value will be ignored. The value returned to a
`self::__clone()` call can, however, be retrieved.

While cloning creates a new object, it does so without using a
constructor, in which case, code may need to be added to the `__clone`
method to emulate what happens in a corresponding constructor. (See the
`Point` example below.)

**Examples**

```Hack
class Employee {
  …
  public function __clone(): void {
    // do what it takes here to make a copy of Employee object properties
  }
}
class Manager extends Employee {
  …
  public function __clone(): void {
    parent::__clone(); // request cloning of the Employee properties
    // do what it takes here to make a copy of Manager object properties
  }
  …
}
// -----------------------------------------
class Point {
  private static int $pointCount = 0;
  public function __construct(float $x = 0.0, float $y = 0.0) {
    …
    ++self::$pointCount;
  }
  public function __clone(): void {
    ++self::$pointCount; // emulate the constructor
  }
  …
}
$p1 = new Point();  // created using the constructor
$p2 = clone $p1;  // created by cloning
```

### Method `__sleep`

**Syntax**

<pre>
public  function  __sleep  ( )  :  array&lt;string&gt;  <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](11-statements.md#compound-statements)

**Semantics**

The instance methods [`__sleep` and `__wakeup`](16-classes.md#method-__wakeup) support
[serialization](16-classes.md#serialization).

If a class has a `__sleep` method, the library function [`serialize`](http://www.php.net/serialize)
calls that method to find out which visible instance properties it
should serialize. (In the absence of a `__sleep` or `serialize` method,
all such properties are serialized.) This information is returned by `__sleep` as an array of zero
or more elements, where each element's value is distinct and is the name
of a visible instance property. These properties' values are serialized
in the order in which the elements are inserted in the array. If
`__sleep` does not return a value explicitly, `null` is returned, and that
value is serialized.

Besides creating the array of property names, `__sleep` can do whatever
else might be needed before serialization occurs.

Consider a `Point` class that not only contains x- and y-coordinates, it
also has an `id` property; that is, each distinct `Point` created during a
program's execution has a unique numerical id. However, there is no need
to include this when a `Point` is serialized. It can simply be recreated
when that `Point` is unserialized. This information is transient and need
not be preserved across program executions. (The same can be true for
other transient properties, such as those that contain temporary results
or run-time caches.)

In the absence of methods `__sleep` and `__wakeup`, instances of derived
classes can be serialized and unserialized. However, it is not possible
to perform customize serialization using those methods for such
instances. For that, a class must implement the interface [`Serializable`](17-interfaces.md#interface-iteratoraggregate).

**Examples**

```Hack
class Point {
  private static int $nextId = 1;
  private float $x;
  private float $y;
  private int $id;
  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
    $this->id = self::$nextId++;  // assign the next available id
  }
  public function __sleep(): array<string> {
    return array('y', 'x'); // serialize only $y and $x, in that order
  }
  public function __wakeup(): void {
    $this->id = self::$nextId++;  // assign a new id
  }
  …
}
$p = new Point(-1, 0);
$s = serialize($p);   // serialize Point(-1,0)
$v = unserialize($s);   // unserialize Point(-1,0)
```

### Method `__toString`

**Syntax**

<pre>
public  function  __toString  ( )  :  string  <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](11-statements.md#compound-statements)

**Constraints**

This function must not throw any exceptions.

**Semantics**

This instance method is intended to create a string representation of
the instance on which it is called. If the instance's class is derived
from a class that has or inherits a `__toString` method, the result of
calling that method should be prepended to the returned string.

`__toString` is called by a number of language and library facilities,
including `echo`, when an object-to-string conversion is needed.
`__toString` can be called directly.

**Examples**

```Hack
class Point {
  private float $x;
  private float $y;
  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
  }
  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }
  …
}
$p1 = new Point(20, 30);
echo $p1 . "\n";  // implicit call to __toString() returns "(20,30)"
// -----------------------------------------
class MyRangeException extends Exception {
  public function __toString(): string {
    return parent::__toString()
      . string-representation-of-MyRangeException
  }
  …
}
```

### Method `__wakeup`

**Syntax**

<pre>
public  function  __wakeup  ( )  : void  <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](11-statements.md#compound-statements)

**Semantics**

The instance methods [`__sleep` and `__wakeup`](16-classes.md#method-__sleep) support
[serialization](16-classes.md#serialization)).

When the library function [`unserialize`](http://www.php.net/unserialize) is called on the string
representation of an object, as created by the library function
[`serialize`](http://www.php.net/serialize), `unserialize` creates an instance of that object's type
**without calling a constructor**, and then calls that class's
`__wakeup` method, if any, to initialize the instance. In the absence of
a `__wakeup` method, all that is done is that the values of the instance
properties encoded in the serialized string are restored.

Consider a `Point` class that not only contains x- and y-coordinates, it
also has an `id` property; that is, each distinct `Point` created during a
program's execution has a unique numerical id. However, there is no need
to include this when a `Point` is serialized. It can simply be recreated
by `__wakeup` when that `Point` is unserialized. This means that
`__wakeup` must emulate the constructor, as appropriate.

**Examples**

See [§§](16-classes.md#method-__sleep).

## Serialization

In Hack, variables can be converted into some external form suitable for
use in file storage or inter-program communication. The process of
converting to this form is known as *serialization* while that of
converting back again is known as *unserialization*. These facilities
are provided by the library functions [`serialize`](http://www.php.net/serialize) and [`unserialize`](http://www.php.net/unserialize), respectively. (Library function [`serialize_memoize_param`](http://www.php.net/serialize_memoize_param) helps when serializing parameters to async functions.)

In the case of variables that are objects, on their own, these two
functions serialize and unserialize all the instance properties, which
may be sufficient for some applications. However, if the programmer
wants to customize these processes, they can do so in one of two,
mutually exclusive ways. The first approach is to define methods called
`__sleep` and `__awake`, and have them get control before serialization
and after serialization, respectively. For information on this approach,
see [§§](16-classes.md#method-__sleep) and [§§](16-classes.md#method-__wakeup). The second approach involves implementing
the interface [`Serializable`](17-interfaces.md#interface-Serializable) by defining two methods, `serialize`
and `unserialize`.

Consider a `Point` class that not only contains x- and y-coordinates, it
also has an `id` property; that is, each distinct `Point` created during a
program's execution has a unique numerical id. However, there is no need
to include this when a `Point` is serialized. It can simply be recreated
when that `Point` is unserialized. This information is transient and need
not be preserved across program executions. (The same can be true for
other transient properties, such as those that contain temporary results
or run-time caches.) Furthermore, consider a class `ColoredPoint` that
extends `Point` by adding a `color` property. The following code shows how
these classes need be defined in order for both `Points` and `ColoredPoints`
to be serialized and unserialized:

```Hack
class Point implements Serializable { // note the interface
  private static int $nextId = 1;
  private float $x;
  private float $y;
  private int $id;  // transient property; not serialized
  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
    $this->id = self::$nextId++;
  }
  public function __toString(): string {
    return 'ID:' . $this->id . '(' . $this->x . ',' . $this->y . ')';
  }
  public function serialize(): string {
    return serialize(array('y' => $this->y, 'x' => $this->x));
  }
```

The custom method `serialize` calls the library function `serialize` to
create a string version of the array, whose keys are the names of the
instance properties to be serialized. The insertion order of the array
is the order in which the properties are serialized in the resulting
string. The array is returned.

```Hack
  public function unserialize(string $data): void {
    $data = unserialize($data);
    $this->x = $data['x'];
    $this->y = $data['y'];
    $this->id = self::$nextId++;
  }
}
```

The custom method `unserialize` converts the serialized string passed to
it back into an array. Because a new object is being created, but
without any constructor being called, the `unserialize` method must
perform the tasks ordinarily done by a constructor. In this case, that
involves assigning the new object a unique id.

```Hack
$p = new Point(2, 5);
$s = serialize($p);
```

The call to the library function `serialize` calls the custom `serialize`
method. Afterwards, the variable `$s` contains the serialized version of
the `Point(2,5)`, and that can be stored in a database or transmitted to a
cooperating program. The program that reads or receives that serialized
string can convert its contents back into the corresponding variable(s),
as follows:

```Hack
$v = unserialize($s);
```

The call to the library function `unserialize` calls the custom
`unserialize` method. Afterwards, the variable `$s` contains a new
`Point(2,5)`.


```Hack
class ColoredPoint extends Point implements Serializable {
  const RED = 1;
  const BLUE = 2;
  private int $color; // an instance property

  public function __construct(float $x = 0.0, float $y = 0.0,
    int $color = ColoredPoint::RED) {
    parent::__construct($x, $y);
    $this->color = $color;
  }

  public function __toString(): string {
    return parent::__toString() . $this->color;
  }

  public function serialize(): string {
    return serialize(array(
      'color' => $this->color,
      'baseData' => parent::serialize()
    ));
  }
```

As with class `Point`, this custom method returns an array of the instance
properties that are to be serialized. However, in the case of the second
element, an arbitrary key name is used, and its value is the serialized
version of the base Point within the current `ColoredPoint` object. The
order of the elements is up to the programmer.

```Hack
  public function unserialize(string $data): void {
    $data = unserialize($data);
    $this->color = $data['color'];
    parent::unserialize($data['baseData']);
  }
}
```

As `ColoredPoint` has a base class, it unserializes its own instance
properties before calling the base class's custom method, so it can
unserialize the `Point` properties.

```Hack
$cp = new ColoredPoint(9, 8, ColoredPoint::BLUE);
$s = serialize($cp);
...
$v = unserialize($s);
```

## Predefined Classes

### Class `AsyncGenerator`

This class supports the `yield` operator when dealing with asynchronous operations. This class cannot be instantiated directly. It is defined, as follows:

```Hack
class AsyncGenerator<Tk,Tv,Ts> implements AsyncKeyedIterator {
  public function next(): Awaitable<?tuple<Tk,Tv>>;
  public function raise(Exception $e): Awaitable<?tuple<Tk,Tv>>;
  public function send(?Ts $v): Awaitable<?tuple<Tk,Tv>>;
}
```

The class members are defined below:

Name | Purpose
---- | -------
`next` | Returns the `Awaitable<T>` associated with the next key/value tuple in the async generator, or `null` if the end of the iteration has been reached. (The result should always be subject to an `awai`t to get the actual key/value tuple. This function cannot be called without having the value returned from a previous call to `next`, `send`, or `raise` having first been the subject of an `await`.)
`raise` | Raises exception *$e* to the async generator. (This function cannot be called without having the value returned from a previous call to `next`, `send`, or `raise` having first been the subject of an `await`.)
`send` | Sends value *$v* to the async generator and resumes execution of that generator. (This function cannot be called without having the value returned from a previous call to `next`, `send`, or `raise` having first been the subject of an `await`.) If *$v* is `null`, the call is equivalent to calling `next`.

### Class `Generator`

This class supports the [`yield` operator](10-expressions.md#yield-operator). This class cannot be
instantiated directly. It is defined, as follows:

```Hack
class Generator implements Iterator {
  public function current(): mixed;
  public function key(): mixed;
  public function next(): void;
  public function rewind(): void;
  public function send(mixed $value): mixed;
  public function throw(Exception $exception): mixed;
  public function valid(): bool;
  public function __wakeup(): void;
```
The type `Continuation<T>` is an alias for `Generator<int, T, void>`.

The class members are defined below:

Name | Purpose
---- | -------
`current` | An implementation of the instance method [`Iterator::current`](17-interfaces.md#interface-Iterator).
`key` | An implementation of the instance method [`Iterator::key`](17-interfaces.md#interface-Iterator).
`next` | An implementation of the instance method [`Iterator::next`](17-interfaces.md#interface-Iterator).
`rewind` | An implementation of the instance method [`Iterator::rewind`](17-interfaces.md#interface-Iterator).
`send` | This instance method sends the value designated by `$value` to the generator as the result of the current [`yield`](http://us2.php.net/manual/en/ language.generators.syntax.php#control-structures.yield) expression, and resumes execution of the generator. `$value` is the return value of the [`yield`](http://us2.php.net/manual/en/language.generators.syntax.php#control-structures.yield) expression the generator is currently at. If the generator is not at a [`yield`](http://us2.php.net/manual/en/language.generators.syntax.php#control-structures.yield) expression when this method is called, it will first be let to advance to the first [`yield`](http://us2.php.net/manual/en/language.generators.syntax.php#control-structures.yield) expression before sending the value. This method returns the yielded value.
`throw` | This instance method throws an exception into the generator and resumes execution of the generator. The behavior is as if the current [`yield`](http://us2.php.net/manual/en/language.generators.syntax.php#control-structures.yield) expression was replaced with throw `$exception`. If the generator is already closed when this method is invoked, the exception will be thrown in the caller's context instead. This method returns the yielded value.
`valid` |  An implementation of the instance method [`Iterator::valid`](17-interfaces.md#interface-Iterator).
`__wakeup` | An implementation of the special instance method [`__wakeup`](16-classes.md#method-__wakeup). As a generator can't be serialized, this method throws an exception of an unspecified type. It returns no value.

### Class `__PHP_Incomplete_Class`

There are certain circumstances in which a program can generate an
instance of this class, which on its own contains no members. One
involves an attempt to unserialize ([§§](#method-__wakeup), [§§](#serialization)) a string that
encodes an instance of a class for which there is no definition in
scope. Consider the following class, which supports a two-dimensional
Cartesian point:

```Hack
class Point {
  private float $x;
  private float $y;
  …
}
$p = new Point(2, 5);
$s = serialize($p); // properties $x and $y are serialized, in that order
```

Let us assume that the serialized string is stored in a database from
where it is retrieved by a separate program. That program contains the
following code, but does not contain a definition of the class Point:

```Hack
$v = unserialize($s);
```

Instead of returning a point, `Point(2, 5`), an instance of
`__PHP_Incomplete_Class` results, with the following contents:

```Hack
__PHP_Incomplete_Class {
   __PHP_Incomplete_Class_Name => "Point"
  x:Point:private => 2
  y:Point:private => 5
}
```

The three properties contain the name of the unknown
class, and the name, visibility, and value of each property that was
serialized, in order of serialization.

### Class `Shapes`

This class provides some shape-related methods. It is defined, as follows:

```Hack
abstract final class Shapes {
  public static function idx(S $shape, arraykey $index) : ?Tv;
  public static function idx(S $shape, arraykey $index, Tv $default) : Tv;
  public static function keyExists(S $shape, arraykey $index): bool;
  public static function removeKey(S $shape, arraykey $index): void;
  public static function toArray(S $shape): array<arraykey, mixed>;
}
```

where S is any shape type.

The class members are defined below:

Name | Purpose
---- | -------
`idx` | This method searches shape `$shape` for the field named `$index`. If the field exists, its value is returned; otherwise, a default value is returned. For a field of type `T`, the function returns a value of type `?T`. A default value `$default` can be provided; however, if that argument is omitted, the value `null` is used. `$index` must be a single-quoted string or a class constant of type `string` or `int`.
`keyExists` | This method searches shape `$shape` for the field named `$index`. If the field exists, `true` is returned; otherwise, `false` is returned. `$index` must be a single-quoted string or a class constant of type `string` or `int`.
`removeKey` | Given a shape `$shape` and a field name `$index`, this method removes the specified field from that shape. If the field specified does not exist, the removal request is ignored. `$index` must be a single-quoted string or a class constant of type `string` or `int`.
`toArray` | This method returns an array of type `array<arraykey, mixed>` containing one element for each field in the shape `$shape`. Each element's key and value are the name and value, respectively, of the corresponding field.

### Class `stdClass`

This class contains no members. It can be instantiated and used as a
base class.
