# Traits

## General

Hack's class model allows [single inheritance](16-classes.md#general) with contracts
being enforced separately via [interfaces](17-interfaces.md#general). A *trait* can provide
both implementation and contracts. Specifically, a class can inherit
from a base class while getting implementation from one or more traits.
At the same time, that class can implement contracts from one or more
interfaces as well as from one or more traits. The use of a trait by a
class does not involve any inheritance hierarchy, so unrelated classes
can use the same trait. In summary, a trait is a set of methods and/or
state information that can be reused.

Traits are designed to support classes; a trait cannot be instantiated
directly.

The members of a trait each have [visibility](16-classes.md#general), which applies once
they are used by a given class. The class that uses a trait can change
the visibility of any of that trait's members, by either widening or
narrowing that visibility. For example, a private trait member can be
made public in the using class, and a public trait member can be made
private in that class.

Once implementation comes from both a base class and one or more traits,
name conflicts can occur. However, trait usage provides a means of
disambiguating such conflicts. Names gotten from a trait can also be
given aliases.

A class member with a given name overrides one with the same name in any
traits that class uses, which, in turn, overrides any such name from
base classes. 

Traits can contain both instance and static members, including both
methods and properties. In the case of a trait with a static property,
each class using that trait has its own instance of that property.

Methods in a trait have full access to all members of any class in which
that trait is used.

## Trait Declarations

**Syntax**

<pre>
  <i>trait-declaration:</i>
   <i>attribute-specification<sub>opt</sub></i>  trait  <i>name</i>  <i>generic-type-parameter-list<sub>opt</sub></i>  <i>class-interface-clause<sub>opt</sub></i>  {
     <i>trait-use-clauses<sub>opt</sub>  trait-member-declarations<sub>opt</sub></i>  }

  <i>trait-use-clauses:</i>
    <i>trait-use-clause</i>
    <i>trait-use-clauses</i>  <i>trait-use-clause</i>

  <i>trait-use-clause:</i>
    use  <i>trait-name-list</i>  ;

  <i>trait-name-list:</i>
    <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
    <i>trait-name-list</i>  ,  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
</pre>

**Defined elsewhere**

* [*attribute-specification*](21-attributes.md#attribute-specification)
* [*class-interface-clause*](16-classes.md#class-declarations)
* [*generic-type-argument-list*](14-generic-types-methods-and-functions.md#type-parameters)
* [*generic-type-parameter-list*](14-generic-types-methods-and-functions.md#type-parameters)
* [*name*](09-lexical-structure.md#names)
* [*trait-member-declarations*](18-traits.md#trait-members)

**Constraints**

The *name*s in *trait-name-list* must designate trait names, excluding
the name of the trait being declared.

A generic trait and a non-generic trait in the same scope cannot have the same name.

**Semantics**

A *trait-declaration* defines a named set of members, which are made
available to any class that uses that trait.

Trait names are [case-preserved](03-terms-and-definitions.md).

A *trait-declaration* may also use other traits. This is done via one or
more *trait-use-clause*s, each of which contains a comma-separated list
of trait names.

The optional *class-interface-clause* specifies the one or more interfaces that must be implemented by any class that uses this trait.

**Examples**

```Hack
trait T1 { public function compute( … ) : … { … } }
trait T2 { public function compute( … ) : … { … } }
trait T1 { public function sort( … ) : … { … } }
trait T4 {
  use T3;
  use T1, T2;
}
```

## Trait Members

**Syntax**

<pre>
  <i>trait-member-declarations:</i>
    <i>trait-member-declaration</i>
    <i>trait-member-declarations   trait-member-declaration</i>

  <i>trait-member-declaration:</i>
    <i>require-extends-clause</i>
    <i>require-implements-clause</i>
    <i>property-declaration</i>
    <i>method-declaration</i>
    <i>constructor-declaration</i>
    <i>destructor-declaration</i>

  <i>require-extends-clause:</i>
    require  extends  <i>qualified-name</i>  ;
  <i>require-implements-clause:</i>
    require  implements  <i>qualified-name</i>  ;
</pre>

**Defined elsewhere**

* [*constructor-declaration*](16-classes.md#constructors)
* [*destructor-declaration*](16-classes.md#destructors)
* [*method-declaration*](16-classes.md#methods)
* [*property-declaration*](16-classes.md#properties)
* [*qualified-name*](20-namespaces.md#defining-namespaces)

**Constraints**

The *qualified-name* in *require-extends-clause* must designate the name of a class that does not directly use the trait being defined.

The *qualified-name* in *require-implements-clause* must designate the name of an interface that does not directly use the trait being defined.

**Semantics**

The members of a trait are those specified by its
*trait-member-declaration*s, and the members from any other traits it
uses.

A trait may contain the following members:

* *require-extends-clauses* each of which requires the class using this trait to directly or indirectly extend the class type designated by *qualified-name*.
* *require-implements-clauses* each of which requires the class using this trait to directly or indirectly implement the interface type designated by *qualified-name*.
* [Properties](16-classes.md#properties) – the variables made available to the class in which the trait is used.
* [Methods](16-classes.md#methods) – the computations and actions that can be performed by the class in which the trait is used. See also [§§](16-classes.md#methods-with-special-semantics).
* [Constructor](16-classes.md#constructors) – the actions required to initialize an instance of the class in which the trait is used.
* [Destructor](16-classes.md#destructors) – the actions to be performed when an instance of the class in which the trait is used is no longer needed.

*trait-member-declarations* may contain multiple *require-extends-clauses* that designate the same class, in which case, the duplicates are redundant. 

*trait-member-declarations* may contain multiple *require-implements-clauses* that designate the same interface, in which case, the duplicates are redundant.

**Examples**

```Hack
trait T {
  require extends C1;
  private int $prop1 = 1000;
  require implements I1;
  protected static int $prop2;
  public function compute( … ): void { … }
  public static function getData( … ): void { … }
}
```


