<style type = "text/css">
  header {
    border-color: red;
    border-width: .25em;
    border-style: solid;
    padding: .25em;
  }
</style>
<header>
NOTICE: This spec is currently very out of date and does not reflect the current version of Hack.
</header>

# Basic Concepts
## Program Structure
A Hack *program* consists of one or more source files, known formally as *scripts*.

<pre>
<i>script:
  </i>&lt;?hh <i>declaration-list<sub>opt</sub></i> (in a .php script)
  <i>declaration-list<sub>opt</sub></i> (in a .hack script)

<i>declaration-list:</i>
  <i>declaration</i>
  <i>declaration-list</i> <i>declaration</i>

<i>declaration:</i>
  <i>inclusion-directive</i>
  <i>enum-declaration</i>
  <i>function-definition</i>
  <i>class-declaration</i>
  <i>interface-declaration</i>
  <i>trait-declaration</i>
  <i>namespace-definition</i>
  <i>namespace-use-declaration</i>
  <i>alias-declaration</i>
</pre>

**Defined elsewhere**

* [*alias-declaration*](05-types.md#type-aliases)
* [*class-declaration*](16-classes.md#class-declarations)
* [*enum-declaration*](13-enums.md#enum-declarations)
* [*function-definition*](15-functions.md#function-definitions)
* [*inclusion-directive*](12-script-inclusion.md#general)
* [*interface-declaration*](17-interfaces.md#interface-declarations)
* [*namespace-definition*](20-namespaces.md#defining-namespaces)
* [*namespace-use-declaration*](20-namespaces.md#namespace-use-declarations)
* [*trait-declaration*](18-traits.md#trait-declarations)

~~A Hack script can be processed in any one of a number of *modes*, of which
`strict` is one. This mode is specified in a [*special single-line-comment*](09-lexical-structure.md#comments), on the first source line, as shown. This comment may be separated
from the preceding &lt;?hh by an arbitrary amount of [horizontal white space](09-lexical-structure.md#white-space), which must not include any [*delimited-comments*](09-lexical-structure.md#comments). This
specification is written from the perspective of strict mode only. A
conforming implementation may provide modes other than `strict`, but they are
outside the scope of this specification.~~

A script can import another script via [script inclusion](12-script-inclusion.md#script-inclusion-operators).

The top level of a script is simply referred to as the *top level*.

## Program Start-Up
Once the start-up function begins execution, it is implementation-defined as
to whether it has access to things like command-line arguments and environment
variables. ~~[PHP's global variables `$argc` and `$argv` are not available in
`strict` mode.]~~ Execution begins with the function with the `<<__EntryPoint>>` attribute in the primary file (not files loaded via [script inclusion](12-script-inclusion.md#script-inclusion-operators)).

## Program Termination
A program may terminate normally in the following ways:

-   A [`return` statement](11-statements.md#the-return-statement) in the start-up function is executed.
-   The start-up function reaches the end of the function.
-   The intrinsic [`exit`](10-expressions.md#exit) is called explicitly.

The behavior of the first case is equivalent to corresponding calls
to `exit`.

A program may terminate abnormally under various circumstances, such as
the detection of an uncaught exception, or the lack of memory or other
critical resource. If execution reaches the end of the start-up script
via a fatal error, or via an uncaught exception and there is no uncaught
exception handler registered by `set_exception_handler`, that is
equivalent to `exit(255)`. If execution reaches the end of the start-up
script via an uncaught exception and an uncaught exception handler was
registered by `set_exception_handler`, that is equivalent to `exit(0)`. ~~It
is unspecified whether [object destructors](16-classes.md#destructors) are run.~~ In all other cases,
the behavior is unspecified.

## The Memory Model
### General
This section and those immediately following it describe the abstract
memory model used by Hack for storing variables. A conforming
implementation may use whatever approach is desired as long as from any
testable viewpoint it appears to behave as if it follows this abstract
model. The abstract model makes no explicit or implied restrictions or
claims about performance, memory consumption, and machine resource
usage.

The abstract model presented here defines three kinds of abstract memory
locations:

-   A *value storage location* (VStore) is used to represent a program
    value, and is created by the Engine as needed. A VStore can contain
    a scalar value such as an integer or a Boolean, or it can contain a
    handle pointing to an HStore (see below).
-   A *variable slot* (VSlot) is used to represent a variable named by
    the programmer in the source code, such as a local variable, an
    array element, an instance property of an object, or a static
    property of a class. A VSlot comes into being based on explicit
    usage of a variable in the source code. A VSlot contains a pointer
    to a VStore.
-   A *heap storage location* (HStore) is used to represent the contents
    of any non-scalar value, and is created by the Engine as needed.

Each existing variable has its own VSlot, which at any time contains a
pointer to a VStore. A VSlot cannot contain a null pointer. A VSlot can
be changed to point to different VStores over time. Multiple VSlots may
simultaneously point to the same VStore. When a new VSlot is created, a
new VStore is also created and the VSlot is initially set to point to
the new VStore.

A VStore can be changed to contain different scalar values and handles
over time. Multiple VStores may simultaneously contain handles that
point to the same HStore. When a VStore is created it initially contains
the scalar value `null` unless specified otherwise. In addition to
containing a value, VStores also carry a *type tag* that indicates the [type](05-types.md#types) of the VStore’s value. ~~A VStore’s type tag can be changed over
time. At any given time a VStore’s type tag may be one of the following:
`Null`, `Bool`, `Int`, `Float`, `Str`, `Arr`, `Arr-D` (see [§§](#deferred-array-copying)), `Obj`, or `Res`.~~

An HStore represents the contents of a non-scalar value, and it may
contain zero or more VSlots. At run time, the Engine may add new VSlots
and it may remove and destroy existing VSlots as needed to support
adding/removing array elements (for arrays) and to support
adding/removing instance properties (for objects). HStores that
represent the contents of arrays and objects have some unspecified way
to identify and retrieve a contained VSlot using a dictionary scheme
(such as having values with integer keys or case-sensitive string keys).
Whether an HStore is a fixed-size during its whole lifetime or whether
it can change size, is unspecified. Whether it allocates auxiliary
chunks of memory or not, is unspecified. Whether it organizes it's
contained VSlots in a linked list or some other manner is unspecified.

An HStore’s VSlots (i.e., the VSlots contained within the HStore) point
to VStores, and each VStore contains a scalar value or a handle to an
HStore, and so on through arbitrary levels, allowing arbitrarily complex
data structures to be represented. For example, a singly linked list
might consist of a variable called `$root`, which is represented by a
VSlot pointing to a VStore containing a handle to the first node. Each
node is represented by an HStore that contains the data for that node in
one or more VSlots, as well as a VSlot pointing to VStore containing a
handle to the next node. Similarly, a binary tree might consist of a
variable called `$root`, which is represented by a VSlot pointing to a
VStore containing a handle to the root node. Each node is represented by
an HStore that contains the data for that node in one or more VSlots, as
well as a pair of VSlots pointing to VStores containing the handles to
the left and right branch nodes. The leaves of the tree would be VStores
or HStores, as needed.

VSlots cannot contain pointers to VSlots or handles to HStores. VStores
cannot contain pointers to VSlots or VStores. HStores cannot directly
contain any pointers or handles to any abstract memory location; HStores
can only directly contain VSlots.

Here is an example demonstrating one possible arrangement of VSlots,
VStores, and HStores:

<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *] [VSlot $y *]]
                                                        |            |
                                                        V            V
                                                [VStore Int 1]  [VStore Int 3]
</pre>

In this picture the VSlot in the upper left corner represents the
variable `$a`, and it points to a VStore that represents `$a`’s current
value. This VStore contains a handle to an HStore which represents the
contents of an object of type Point with two instance properties `$x`
and `$y`. This HStore contains two VSlots representing instance
properties `$x` and `$y`, and each of these VSlots points to a distinct
VStore which contains an integer value. 

***Implementation Notes*** A concrete implementation of Hack may be mapped roughly
onto the abstract memory model as follows: `zval pointer => VSlot, zval
=> VStore, HashTable => HStore`, and
`zend_object/zend_object_handlers => HStore`. Note, however, that the
abstract memory model is not intended to exactly match any specific
implementation’s model, and for generality and simplicity there may be some
superficial differences between the described model and an implementation's model.

~~For most operations, the mapping between VSlots and VStores remains the
same. Only the following program constructs can change a VSlot to point
to different VStore: the use of `&` in a [`foreach` statement](11-statements.md#the-foreach-statement).~~

### Reclamation and Automatic Memory Management
The Engine is required to manage the lifetimes of VStores and HStores
using some form of automatic memory management.

When dealing with VStores and HStores, the Engine is required to implement
some form of automatic memory management. When a VStore or HStore
is created, memory is allocated for it, and for an HStore that represents
an [object](05-types.md#class-types), its [constructor](16-classes.md#constructors) is invoked.

Later, if a VStore or HStore becomes unreachable through any existing
variable, they become eligible for reclamation to release the memory
they occupy. The engine may reclaim a VStore or HStore at any time
between when it becomes eligible for reclamation and when the script
exits. ~~Before reclaiming an HStore that represents an [object](05-types.md#class-types),
the Engine will invoke the object’s [destructor](16-classes.md#destructors) if one is defined.~~

~~The Engine must reclaim each VSlot when the [storage duration](#storage-duration) of its
corresponding variable ends, when the variable is explicitly unset by the
programmer, or when the script exits, whichever comes first. In the case where
a VSlot is contained within an HStore (i.e. an array element or an object
instance property), the engine must immediate reclaim the VSlot when it is
explicitly unset by the programmer, when the containing HStore is reclaimed,
or when the script exits, whichever comes first.~~

The precise form of automatic memory management used by the Engine is
unspecified, which means that the time and order of the reclamation of
VStores and HStores is unspecified.

A VStore’s refcount is defined as the number of unreclaimed VSlots that point
to the VStore. Because the precise form of automatic memory management is not
specified, a VStore’s refcount at a given time may differ between
conforming implementations due to VSlots, VStores, and HStores being
reclaimed at different times. Despite the use of the term refcount,
conforming implementations are not required to use a reference
counting-based implementation for automatic memory management.

### Assignment
#### General
This section and those immediately following it describe the abstract
model’s implementation of *value assignment*.
Value assignment of non-array types to local variables is described
first, followed by
value assignment of array types to local variables, and ending with
value assignment with complex left-hand side expressions.

Value assignment is core to HHVM, the Engine which
supports the Hack language. Many other operations in the
Hack specification are described in terms of value assignment.

#### Value Assignment of Scalar Types to a Local Variable
Value assignment is the primary means by which the programmer can create
local variables. If a local variable that appears on the left-hand side of
value assignment does not exist, the engine will bring a new local
variable into existence and create a VSlot and initial VStore for
storing the local variable’s value.

Consider the following example of [value assignment](10-expressions.md#simple-assignment) of scalar
values to local variables:

```Hack
$a = 123;

$b = false;
```
<pre>
[VSlot $a *]-->[VStore Int 123]

[VSlot $b *]-->[VStore Bool false]
</pre>

Variable `$a` comes into existence and is represented by a newly created
VSlot pointing to a newly created VStore. Then the integer value 123 is
written to the VStore. Next, `$b` comes into existence represented by a
VSlot and corresponding VStore, and the Boolean value false is written
to the VStore.

Next consider the value assignment `$b = $a`:

<pre>
[VSlot $a *]-->[VStore Int 123]

[VSlot $b *]-->[VStore Int 123 (Bool false was overwritten)]
</pre>

The integer value 123 is read from `$a`’s VStore and is written into
`$b`’s VStore, overwriting its previous contents. As we can see, the two
variables are completely self-contained; each has its own VStore
containing a separate copy of the integer value 123. Value assignment
reads the contents of one VStore and overwrites the contents of the
other VStore, but the relationship of VSlots to VStores remains
unchanged. Changing the value of `$b` has no effect on `$a`, and vice
versa.

Using literals or arbitrarily complex expressions on the right hand side
of value assignment value works the same as it does for variables,
except that the literals or expressions don’t have their own VSlots or
VStores. The scalar value or handle produced by the literal or
expression is written into the VStore of the left hand side, overwriting
its previous contents.

To illustrate the semantics of value assignment further, consider `++$b`:

<pre>
[VSlot $a *]-->[VStore Int 123]

[VSlot $b *]-->[VStore Int 124 (123 was overwritten)]
</pre>

Now consider `$a = 99`:

<pre>
[VSlot $a *]-->[VStore Int 99 (123 was overwritten)]

[VSlot $b *]-->[VStore Int 124]
</pre>

In both of these examples, one variable’s value is changed without
affecting the other variable’s value. While the above examples only
demonstrate value assignment for integer and Boolean values, the same
mechanics apply for all scalar types.

Note that strings are also considered scalar values for the purposes of
the abstract memory model. Unlike non-scalar types which are represented
using a VStore pointing to an HStore containing the non-scalar value’s
contents, the abstract model assumes that a string’s entire contents
(i.e., the string’s characters and its length) can be stored in a VStore
and that value assignment for a string eagerly copies a string’s entire
contents to the VStore being written to. Consider the following example:

```Hack
$a = 'gg';

$b = $a;
```

<pre>
[VSlot $a *]-->[VStore Str 'gg']

[VSlot $b *]-->[VStore Str 'gg']
</pre>

`$a`’s string value and `$b`’s string values are distinct from each other,
and mutating `$a`’s string will not affect `$b`. Consider `++$b`, for
example:

<pre>
[VSlot $a *]-->[VStore Str 'gg']

[VSlot $b *]-->[VStore Str 'gh']
</pre>

Because a string’s content can be arbitrarily large, copying a string’s
entire contents for value assignment can be expensive. In practice an
application written in Hack may rely on value assignment of strings being
relatively inexpensive (in order to deliver acceptable performance), and
as such it is common for an implementation to use a deferred copy
mechanism to reduce the cost of value assignment for strings. Deferred
copy mechanisms work by not copying a string during value assignment and
instead allowing multiple variables to share the string’s contents
indefinitely until a mutating operation (such as the increment operator)
is about to be executed on the string, at which time some or all of the
string’s contents are copied. A conforming implementation may choose to
defer copying a string’s contents for value assignment so long as it has
no observable effect on behavior from any testable viewpoint (excluding
performance and resource consumption).

#### Value Assignment of Object ~~and Resource~~ Types to a Local Variable

To demonstrate value assignment of objects to local variables, consider
the case in which we have a Point class that supports a two-dimensional
Cartesian system. An instance of Point contains two instance properties,
`$x` and `$y`, that store the x- and y-coordinates, respectively. A
[constructor call](14-classes.md#constructors) of the form `Point(x, y)` used with [operator `new`](10-expressions.md#the-new-operator)
creates a new point at the given location, and a method call
of the form `move(newX, newY)` moves a `Point` to the new location.

With the `Point` class, let us consider the value assignment `$a = new
Point(1, 3)`:

<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *] [VSlot $y *]]
                                                        |            |
                                                        V            V
                                                 [VStore Int 1]  [VStore Int 3]
</pre>

Variable `$a` is given its own VSlot, which points to a VStore that
contains a handle pointing to an HStore allocated by [`new`](10-expressions.md#the-new-operator) and
that is initialized by `Point`'s constructor.

Now consider the value assignment `$b = $a`:

<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *] [VSlot $y *]]
                                  ^                     |            |
                                  |                     V            V
[VSlot $b *]-->[VStore Obj *]-----+             [VStore Int 1] [VStore Int 3]
</pre>

`$b`‘s VStore contains a handle that points to the same object as does
`$a`‘s VStore’s handle. Note that the Point object itself was not copied,
and note that `$a`’s and `$b`’s VSlots point to distinct VStores.

Let's modify the value of the Point whose handle is stored in `$b` using
`$b->move(4, 6)`:

<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *] [VSlot $y *]]
                                  ^                     |            |
                                  |                     V            V
[VSlot $b *]-->[VStore Obj *]-----+         [VStore Int 4] [VStore Int 6]
                                       (1 was overwritten) (3 was overwritten)
</pre>

As we can see, changing `$b`'s Point changes `$a`'s as well.

Now, let's make `$a` point to a different object using `$a = new Point(2,
1)`:

<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *] [VSlot $y *]]
                                                        |            |
[VSlot $b *]-->[VStore Obj *]-----+                     V            V
                                  |             [VStore Int 2] [VStore Int 1]
                                  V
                                [HStore Point [VSlot $x *] [VSlot $y *]]
                                                        |            |
                                                        V            V
                                                [VStore Int 4] [VStore Int 6]
</pre>

Before `$a` can take on the handle of the new `Point`, its handle to the
old `Point` must be removed, which leaves the handles of `$a` and `$b`
pointing to different Points.

We can remove all these handles using `$a = null` and `$b = null`:
<pre>
[VSlot $a *]-->[VStore Null]    [HStore Point [VSlot $x *] [VSlot $y *] (dead)]
                                                        |            |
[VSlot $b *]-->[VStore Null]    [VStore Int 2 (dead)]&lt;--+            V
                                                          [VStore Int 1 (dead)]
                                                          
                                [HStore Point [VSlot $x *] [VSlot $y *] (dead)]
                                                        |            |
                                [VStore Int 4 (dead)]&lt;--+            V
                                                        [VStore Int 6 (dead)]
</pre>

By assigning null to `$a`, we remove the only handle to `Point(2,1)`, which
allows that object to be reclaimed at some unspecified future time. A similar
thing happens with `$b`, as it too is the only handle to its Point.

Although the examples above only show with only two instance properties,
the same mechanics apply for value assignment of all object types, even
though they can have an arbitrarily large number of instance properties
of arbitrary type. ~~Likewise, the same mechanics apply to value
assignment of all resource types.~~

#### Value Assignment of Array Types to Local Variables
The semantics of value assignment of array types is different from value
assignment of other types. Recall the `Point` class from the examples in [§§](#value-assignment-of-object-and-resource-types-to-a-local-variable), and consider the following [value assignments](10-expressions.md#simple-assignment) and their abstract implementation:

`$a = array(10, 'B' => new Point(1, 3));`
<pre>
[VSlot $a *]-->[VStore Arr *]-->[HStore Array [VSlot 0 *] [VSlot 'B' *]]
                                                       |             |
                                                       V             V
                                             [VStore Int 10]   [VStore Obj *]
                                                                           |
                                [HStore Point [VSlot $x *] [VSlot $y *]]&lt;--+
                                                        |            |
                                                        V            V
                                            [VStore Int 1]  [VStore Int 3]
</pre>

In the example above, `$a`‘s VStore is initialized to contain a handle to
an HStore for an array containing two elements, where one element is an
integer and the other is a handle to an HStore for an object.

Now consider the following value assignment `$b = $a`. A conforming
implementation must implement value assignment of arrays in one of the
following ways: (1) eager copying, where the implementation makes a copy
of `$a`’s array during value assignment and changes `$b`’s VSlot to point
to the copy; or (2) deferred copying, where the implementation uses a
deferred copy mechanism that meets certain requirements. This section
describes eager copying, and the section that immediately follows ([§§](#deferred-array-copying))
describes deferred copying.

To describe the semantics of eager copying, let’s begin by considering
the value assignment `$b = $a`:
<pre>
[VSlot $a *]-->[VStore Arr *]-->[HStore Array [VSlot 0 *] [VSlot 'B' *]]
                                                       |             |
[VSlot $b *]-->[VStore Arr *]                          V             V
                           |                    [VStore Int 10]  [VStore Obj *]
                           V                                                 |
[HStore Array [VSlot 0 *] [VSlot 'B' *]]                                     |
                       |             |                                       |
             +---------+   +---------+                                       |
             V             V                                                 |
[VStore Int 10] [VStore Obj *]-->[HStore Point [VSlot $x *] [VSlot $y *]]&lt;---+
                                                         |            |
                                                         V            V
                                                 [VStore Int 1]  [VStore Int 3]
</pre>

The value assignment `$b = $a` made a copy of `$a`’s array. Note how
`$b`’s VSlot points to a different VStore than `$a`’s VSlot, and `$b`’s
VStore points to a different HStore than `$b`’s VStore. Each source array
element is copied using value assignment (`destination = $source`).

The first element VSlots in `$a`’s array and `$b`’s array point
to distinct VStores, each of which contain a distinct copy of the
integer value 10. The second element VSlots in `$a`’s array and `$b`’s
array point to distinct VStores, each of which contain a handle to the
same object HStore.

Let’s consider another example:
```Hack
$x = 123;
$a = array(array($x, 'hi'));
$b = $a;
```

<pre>
[VSlot $a *]---->[VStore Arr *]---->[HStore Array [VSlot 0 *]]
                                                           |
[VSlot $x *]---->VStore Int 123]        [VStore Arr *]&lt;----+
                                                     |
[VSlot $b *]-->[VStore Arr *]                        V
                           |            [HStore Array [VSlot 0 *] [VSlot 1 *]]
                           V                                   |           |
         [HStore Array [VSlot 0 *]]                            V           |
                                |                      [VStore Int 123]    |
                                V                                          V
                     [VStore Arr *]                            [VStore Str 'hi']
                                 |
                                 V
                    [HStore Array [VSlot 0 *] [VSlot 1 *]]
                                           |           |
                                           V           V 
                                  [VStore Int 123]  [VStore Str 'hi']
</pre>

Value assignment with eager copying makes a
copy of `$a`’s array, copying the array’s single element using
value assignment, which in turn
makes a copy of the inner array inside `$a`’s array, copying the inner
array’s elements using value assignment.

Although the examples in this section only use arrays with one
element or two elements, the model works equally well for all
arrays even though they can have an arbitrarily large number
of elements. As to how an HStore accommodates all of them, is
unspecified and unimportant to the abstract model.

#### Deferred Array Copying
As mentioned in the previous section ([§§](#value-assignment-of-array-types-to-local-variables)), an implementation may
choose to use a deferred copy mechanism instead of eagerly making a copy
for value assignment of arrays. An implementation may use any deferred
copy mechanism desired so long as it conforms to the abstract model’s
description of deferred array copy mechanisms presented in this
section.

Because an array’s contents can be arbitrarily large, eagerly copying an
array’s entire contents for value assignment can be expensive. In
practice an application written in Hack may rely on value assignment of
arrays being relatively inexpensive for the common case (in order to deliver
acceptable performance), and as such it is common for an implementation
to use a deferred array copy mechanism in order to reduce the cost of
value assignment for arrays.

Like conforming deferred string copy mechanisms discussed in [§§](#value-assignment-of-scalar-types-to-a-local-variable)
that must produce the same observable behavior as eager string copying,
deferred array copy mechanisms are required to produce the same observable behavior as eager array copying.

#### General Value Assignment
The sections above thus far have described the mechanics of value assignment
to a local variable. This section describes how value assignment works
when general modifiable lvalue expressions are used on the left hand side.

For example, assuming `Point` definition as in previous sections and further
assuming all instance properties are public, this code:

```Hack
$a = new Point(1, 3);
$b = 123;
$a->x = $b;
```

will result in:
<pre>
[VSlot $a *]-->[VStore object *]-->[HStore Point [VSlot $x *] [VSlot $y *]]
                                                           |            |
                                                           V            V
                                                  [VStore int 123] [VStore int 3]
[VSlot $b *]-->[VStore int 123]
</pre>

<mark>TODO(aorenste): I think this following section can be deprecated - Hack no longer supports dynamically adding new VSlots to non-dynamic objects.</mark>

If needed, new VSlots are created as part of the containing VStore, for example:

```Hack
$a = new Point(1, 3);
$b = 123;
$a->z = $b;
```

will result in:
<pre>
[VSlot $a *]-->[VStore object *]-->[HStore Point [VSlot $x *] [VSlot $y *] [VSlot $z *]]
                                                           |            |            |
                                                           V            V            V
                                                  [VStore int 1] [VStore int 3] [VStore int 123]
[VSlot $b *]-->[VStore int 123]
</pre>

The same holds for array elements:
```Hack
$a = array('hello', 'world');
$b = 'hack';
$a[1] = $b;
$a[2] = 'World!';
```

will result in:
<pre>
[VSlot $a *]-->[VStore array *]-->[HStore Array [VSlot 0 *]  [VSlot 1 *]  [VSlot 2 *]]
                                                         |            |            |
                                                         V            V            V
                                    [VStore string 'hello'] [VStore string 'hack'] [VStore string 'World!']
[VSlot $b *]-->[VStore string 'hack']
</pre>

where the third VSlot with index 2 was created by the assignment.

Note that any array element and instance property, including a designation of non-existing ones,
is considered a modifiable lvalue, and the VSlot will be created by the engine and added
to the appropriate HStore automatically. Static class properties are considered modifiable lvalues too,
though new ones would not be created automatically.

### Argument Passing
Argument passing is defined in terms of simple assignment[§§](#value-assignment-of-scalar-types-to-a-local-variable), [§§](#value-assignment-of-object-and-resource-types-to-a-local-variable), [§§](#value-assignment-of-array-types-to-local-variables), and [§§](10-expressions.md#simple-assignment)). 
That is, passing an argument to a function having a corresponding
parameter is like assigning that argument to that parameter.

<mark>TODO(aorenste): inout parameters </mark>

### Value Returning
Returning a value from a function is defined in terms of simple
assignment ([§§](#value-assignment-of-scalar-types-to-a-local-variable), [§§](#value-assignment-of-object-and-resource-types-to-a-local-variable), [§§](#value-assignment-of-array-types-to-local-variables), and [§§](10-expressions.md#simple-assignment)).  That is, returning a value from a function to its
caller is like assigning that value to the user of the caller's return
value.

### Cloning objects
When an instance is allocated, [operator `new`](10-expressions.md#the-new-operator) returns a handle
that points to that object. (As described in [§§](#value-assignment-of-object-and-resource-types-to-a-local-variable)), value assignment of a handle to an object does not copy the object HStore itself. Instead, it creates a copy of the handle. How then to make a copy of the object itself? Our only access to it is
via the handle. The Hack language allows us to do this via [operator `clone`](10-expressions.md#the-clone-operator).

To demonstrate how the `clone` operator works, consider the case in which
an instance of class `Widget` contains two instance properties: `$p1` has
the integer value 10, and `$p2` is a handle to an array of elements of
some type(s) or to an instance of some other type.
<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Widget [VSlot $p1 *][VSlot $p2 *]]
                                                          |          |
                                                          V          V
                                               [VStore Int 10] [VStore Obj *]
                                                                         |
                                                                         V
                                                                 [HStore ...]
</pre>

Let us consider the result of `$b = clone $a`:
<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Widget [VSlot $p1 *][VSlot $p2 *]]
                                                          |            |
[VSlot $b *]-->[VStore Obj *]                             V            V
                             |                  [VStore Int 10] [VStore Obj *]
     +-----------------------+                                              |
     V                                                                      V
   [HStore Widget [VSlot $p1 *] [VSlot $p2 *]]              +--->[HStore ...]
                             |             |                |
                             V             V                |
                 [VStore Int 10] [VStore Obj *]-------------+
</pre>

The clone operator will create another object HStore of the same class
as the original, copy `$a`’s object’s instance properties using
member-copy [assignment `=*`](#value-assignment-of-array-types-to-local-variables). For the example shown above, the
handle to the newly created HStore stored into `$b` using value
assignment. Note that the clone operator will not recursively clone
objects held in `$a`’s instance properties; hence the object copying
performed by the clone operator is often referred to as a *shallow
copy*. If a *deep copy* of an object is desired, the programmer must
achieve this manually by using the method [`__clone`](16-classes.md#method-__clone) or by
other means.

## Scope

The same name can designate different things at different places in a
program. For each different thing that a name designates, that name is
visible only within a part of the program called that name's *scope*.
The following distinct scopes exist:

-   Script, which means from the point of declaration/first
    initialization through to the end of that script, including any
    [included scripts](12-script-inclusion.md#script-inclusion-operators).
-   Function, which means from the point of declaration/first
    initialization through to the end of that [function](15-functions.md#function-definitions).
-   Class, which means the body of that class and any classes derived
    from it ([§§](16-classes.md#class-declarations)).
-   Interface, which means the body of that interface, any interfaces
    derived from it, and any classes that implement it ([§§](17-interfaces.md#interface-declarations)).
-   Namespace, which means from the point of declaration/first
    initialization through to the end of that [namespace](20-namespaces.md#general).

A variable declared or first initialized inside a function has function scope.

Each function has its own function scope. An [anonymous function](15-functions.md#anonymous-functions)
has its own scope separate from that of any function inside which that
anonymous function is defined.

The scope of a parameter is the body of the function in which the
parameter is declared. For the purposes of scope, a [catch-block](11-statements.md#the-try-statement)
is treated like a function body, in which case, the *variable-name* in
*parameter-declaration-list* is treated like a parameter.

The scope of a class member `m` ([§§](16-classes.md#class-members)) declared in, or inherited by, a
class type `C` is the body of `C`.

The scope of an interface member `m` ([§§](17-interfaces.md#interface-members)) declared in, or inherited by,
an interface type `I` is the body of `I`.

When a [trait](18-traits.md#general) is used by a class or an interface, the [trait's
members](16-classes.md#class-members) take on the scope of a member of that class or
interface.

## Storage Duration

The lifetime of a variable is the time during program execution that
storage for that variable is guaranteed to exist. This lifetime is
referred to as the variable's *storage duration*, of which there are
three kinds: automatic, static, and allocated.

A variable having *automatic storage duration* comes into being and is
initialized at its declaration or on its first use, if it has no
declaration. Its lifetime is delimited by an enclosing [scope](#scope). The
automatic variable's lifetime ends at the end of that scope. Automatic
variables lend themselves to being stored on a stack where they can help
support argument passing and recursion. [Local variables](07-variables.md#local-variables), which
include [function parameters](15-functions.md#function-definitions), have automatic storage duration.

A variable having *static storage duration* comes into being and is
initialized before its first use, and lives until program shutdown. The
following kinds of variables have static storage duration: [function statics](07-variables.md#function-statics), [static properties](07-variables.md#static-properties), and [class and interface constants](07-variables.md#class-and-interface-constants).

A variable having *allocated storage duration* comes into being based on
program logic by use of the [`new` operator](10-expressions.md#the-new-operator). Ordinarily, once
such storage is no longer needed, it is reclaimed automatically by the
Engine via its garbage-collection process and the use of
[destructors](16-classes.md#destructors). The following kinds of variables have allocated
storage duration: [array elements](07-variables.md#array-elements) and [instance properties](07-variables.md#instance-properties).

The following example demonstrates the three storage durations:

```Hack
class Point { ... }

function doit($p1) {
  $av2 = ...;           // auto variable $av2 created and initialized
  static $sv2 = ...;    // static variable $sv2 created and initialized
  if ($p1) {
    $av3 = ...;         // auto variable $av3 created and initialized
    static $sv3 = ...;  // static variable $sv3 created and initialized
    ...
  }
  global $av1;
  $av1 = new Point(2, 3);   // Point(0,1) is eligible for destruction
  ...
}                       // $av2 and $av3 are eligible for destruction

function main($p1): void {
  $av1 = new Point();   // auto variable $av1 created and initialized
  static $sv1 = ...;    // static variable $sv1 created and initialized
  doit(true);
}

// At end of script, $sv1, $sv2, and $sv3 are eligible for destruction
```

The comments indicate the beginning and end of lifetimes for each
variable.

If function `doit` is called multiple times, each time it is called, its
automatic variables are created and initialized, whereas its static
variables retain their values from previous calls.

Consider the following recursive function: 

```Hack
function factorial(int $i): int {
  if ($i > 1) return $i * factorial($i - 1);
  else if ($i == 1) return $i;
  else return 0;
}
```

When `factorial` is first called, the local variable parameter `$i` is
created and initialized with the value of the argument in the call.
Then, if this function calls itself, the same process is repeated each
call. Specifically, each time `factorial` calls itself, a new local
variable parameter `$i` is created and initialized with the value of the
argument in the call.

The lifetime of any [VStore](#general) or [HStore](#general) can be extended by
the Engine as long as needed. Conceptually, the lifetime of a VStore ends
when it is no longer pointed to by any [VSlots](#general). Conceptually, the
lifetime of an HStore ends when no VStores have a handle to it.
