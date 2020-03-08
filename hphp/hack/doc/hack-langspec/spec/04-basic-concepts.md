# Basic Concepts
## Program Structure
A Hack *program* consists of one or more source files, known formally as
*scripts*.

<pre>
<i>script:
  </i>&lt;?hh // strict
  <i>declaration-list<sub>opt</sub></i>

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

A Hack script can be processed in any one of a number of *modes*, of which
`strict` is one. This mode is specified in a [*special single-line-comment*](09-lexical-structure.md#comments), on the first source line, as shown. This comment may be separated
from the preceding &lt;?hh by an arbitrary amount of [horizontal white space](09-lexical-structure.md#white-space), which must not include any [*delimited-comments*](09-lexical-structure.md#comments). This
specification is written from the perspective of strict mode only. A
conforming implementation may provide modes other than `strict`, but they are
outside the scope of this specification.

A script can import another script via [script inclusion](12-script-inclusion.md#script-inclusion-operators).

The top level of a script is simply referred to as the *top level*.

## Program Start-Up
Once the start-up function begins execution, it is implementation-defined as
to whether it has access to things like command-line arguments and environment
variables. [PHP's global variables `$argc` and `$argv` are not available in
`strict` mode.]

## Program Termination
A program may terminate normally in the following ways:

-   A [`return` statement](11-statements.md#the-return-statement) in the start-up function is executed.
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
registered by `set_exception_handler`, that is equivalent to exit(0). It
is unspecified whether [object destructors](16-classes.md#destructors) are run. In all other cases,
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
containing a value, VStores also carry a *type tag* that indicates the [type](05-types.md#types) of the VStore’s value. A VStore’s type tag can be changed over
time. At any given time a VStore’s type tag may be one of the following:
`Null`, `Bool`, `Int`, `Float`, `Str`, `Arr`, `Arr-D` (see [§§](#deferred-array-copying)), `Obj`, or `Res`.

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

For most operations, the mapping between VSlots and VStores remains the
same. Only the following program constructs can change a VSlot to point
to different VStore: the use of `&` in a [`foreach` statement](11-statements.md#the-foreach-statement).

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
exits. Before reclaiming an HStore that represents an [object](05-types.md#class-types),
the Engine will invoke the object’s [destructor](16-classes.md#destructors) if one is defined.

The Engine must reclaim each VSlot when the [storage duration](#storage-duration) of its
corresponding variable ends, when the variable is explicitly unset by the
programmer, or when the script exits, whichever comes first. In the case where
a VSlot is contained within an HStore (i.e. an array element or an object
instance property), the engine must immediate reclaim the VSlot when it is
explicitly unset by the programmer, when the containing HStore is reclaimed,
or when the script exits, whichever comes first.

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
model’s implementation of *value assignment* and *byRef assignment*.
Value assignment of non-array types to local variables is described
first, followed by byRef assignment with local variables, followed by
value assignment of array types to local variables, and ending with
value assignment with complex left-hand side expressions, and byRef
assignment with complex expressions on the left- or right-hand side.

Value assignment and byRef assignment are core to HHVM, the Engine which
supports the  PHP and Hack languages. Many other operations in the PHP and
Hack specification are described in terms of value assignment. On the other
hand, byRef assignment is used only by PHP, not by Hack. However, a discussion
of such assignment has been retained here for historical reference.

#### Value Assignment of Scalar Types to a Local Variable
Value assignment is the primary means by which the programmer can create
local variables. If a local variable appears on the left-hand side of
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

***Implementation Notes*** For simplicity, the abstract model’s
definition of value assignment never changes the mapping from VSlots to
VStores. This aspect of the abstract model is superficially different
from the php.net implementation’s model, which in some cases will set
two variable slots to point to the same zval when performing value
assignment. Despite this superficial difference, php.net’s
implementation produces the same observable behavior as the abstract
model presented here.

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

***Implementation Notes*** For simplicity, the abstract model represents
a string as a scalar value that can be entirely contained within VStore.
This aspect of the abstract model is superficially different from the
php.net implementation’s model, where a zval points to a separate buffer
in memory containing a string’s characters and in the common case
multiple slots point to the same zval that holds the string. Despite
this superficial difference, php.net’s implementation produces the same
observable behavior (excluding performance and resource consumption) as
the abstract model presented here.

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

#### Value Assignment of Object and Resource Types to a Local Variable

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
allows that object's [destructor](14-classes.md#destructors) to run. A similar thing happens
with `$b`, as it too is the only handle to its Point.

Although the examples above only show with only two instance properties,
the same mechanics apply for value assignment of all object types, even
though they can have an arbitrarily large number of instance properties
of arbitrary type. Likewise, the same mechanics apply to value
assignment of all resource types.

#### ByRef Assignment for Scalar Types with Local Variables
Let's begin with the same [value assignment](10-expressions.md#simple-assignment) as in the previous
section, `$a = 123` and `$b = false`:

<pre>
[VSlot $a *]-->[VStore Int 123]

[VSlot $b *]-->[VStore Bool false]
</pre>

Now consider the [byRef assignment](10-expressions.md#byref-assignment) `$b =& $a`, which has byRef
semantics:
<pre>
[VSlot $a *]-->[VStore Int 123]
                 ^
                 |
[VSlot $b *]-----+     [VStore Bool false (dead)]
</pre>

In this example, byRef assignment changes `$b`’s VSlot point to the same
VStore that `$a`’s VSlot points to. The old VStore that `$b`’s VSlot used
to point to is now unreachable. As stated in [§§](#general), it is not possible for a VSlot to point to another VSlot, so `$b`‘s VSlot cannot point to `$a`‘s VSlot. When multiple variables’ VSlots point to the same VStore,
the variables are said to be *aliases* of each other or they are said to
have an *alias relationship*. In the example above, after the byRef
assignment executes the variables `$a` and `$b` will be aliases of each
other.

Now, let's observe what happens when we change the value of `$b` using
`++$b`:
<pre>
[VSlot $a *]-->[VStore Int 124 (123 was overwritten)]
                 ^
                 |
[VSlot $b *]-----+
</pre>

`$b`‘s value, which is stored in the VStore that `$b`’s VSlot points, is
changed to 124. And as that VStore is also aliased by `$a`’s VSlot, the
value of `$a` is also 124. Indeed, any variable’s VSlot that is aliased
to that VStore will have the value 124.

Now consider the value assignment `$a = 99`:
<pre>
[VSlot $a *]-->[VStore Int 99 (124 was overwritten)]
                 ^
                 |
[VSlot $b *]-----+
</pre>

The alias relationship between `$a` and `$b` can be broken explicitly by
using `unset` on variable `$a` or variable `$b`. For example, consider
`unset($a)`:
<pre>
[VSlot $a (dead)]      [VStore Int 99]
                         ^
                         |
[VSlot $b *]-------------+
</pre>

Unsetting `$a` causes variable `$a` to be destroyed and its corresponding
alias to the VStore to be removed, leaving `$b`’s VSlot as the only
pointer remaining to the VStore.

Other operations can also break an alias relationship between two or
more variables. For example, `$a = 123` and `$b =& $a`, and `$c = 'hi'`:
<pre>
[VSlot $a *]-->[VStore Int 123]
                 ^
                 |
[VSlot $b *]-----+

[VSlot $c *]-->[VStore Str 'hi']
</pre>

After the byRef assignment, `$a` and `$b` now have an alias relationship.
Next, let's observe what happens for `$b =& $c`:
<pre>
[VSlot $a *]-->[VStore Int 123]

[VSlot $b *]-----+
                 |
                 V
[VSlot $c *]-->[VStore Str 'hi']
</pre>

As we can see, the byRef assignment above breaks the alias relationship
between `$a` and `$b`, and now `$b` and `$c` are aliases of each other. When
byRef assignment changes a VSlot to point to a different VStore, it
breaks any existing alias relationship the left hand side variable had
before the assignment operation.

It is also possible to use byRef assignment to make three or more VSlots
point to the same VStore. Consider the following example:

```Hack
$b =& $a;
$c =& $b;
$a = 123;
```
<pre>
[VSlot $a *]-->[VStore Int 123]
                 ^   ^
                 |   |
[VSlot $b *]-----+   |
                     |
[VSlot $c *]---------+
</pre>

Like value assignment, byRef assignment provides a means for the
programmer to created variables. If the local variables that appear on
the left- or right-hand side of byRef assignment do not exist, the
engine will bring new local variables into existence and create a VSlot
and initial VStore for storing the local variable’s value.

Note that literals, constants, and other expressions that don’t
designate a modifiable lvalue cannot be used on the left- or right-hand
side of byRef assignment.

#### Byref Assignment of Non-Scalar Types with Local Variables
byRef assignment of non-scalar types works using the same mechanism as
byRef assignment for scalar types. Nevertheless, it is worthwhile to
describe a few examples to clarify the semantics of byRef assignment.
Recall the example from [§§](#value-assignment-of-object-and-resource-types-to-a-local-variable)) using the `Point` class:

`$a = new Point(1, 3);`
<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *] [VSlot $y *]]
                                                        |            |
                                                        V            V
                                               [VStore Int 1]  [VStore Int 3]
</pre>

Now consider the [byRef assignment](10-expressions.md#byref-assignment) `$b =& $a`, which has byRef
semantics:
<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *][VSlot $y *]]
                 ^                                      |           |
                 |                                      V           V
[VSlot $b *]-----+                               [VStore Int 1] [VStore Int 3]
</pre>
`$a` and `$b` now aliases of each other. Note that byRef assignment
produces a different result than `$b = $a` where `$a` and `$b` would point
to distinct VStores pointing to the same HStore.

Let's modify the value of the `Point` aliased by `$a` using `$a->move(4,
6)`:
<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *] VSlot $y *]]
                 ^                                      |           |
                 |                                      V           V
[VSlot $b *]-----+                           [VStore Int 4] [VStore Int 6]
                                        (1 was overwritten) (3 was overwritten)
</pre>

Now, let's change `$a` itself using the value assignment `$a = new
Point(2, 1)`:
<pre>
[VSlot $a *]-->[VStore Obj *]-->[HStore Point [VSlot $x *][VSlot $y *]]
                 ^                                      |           |
                 |                                      V           V
[VSlot $b *]-----+                             [VStore Int 2] [VStore Int 1]

                               [HStore Point [VSlot $x *]   [VSlot $y *] (dead)]
                                                       |              |
                                                       V              V
                                     [VStore Int 4 (dead)] [VStore Int 6 (dead)]
</pre>

As we can see, `$b` continues to have an alias relationship with `$a`.
Here's what's involved in that assignment: `$a` and `$b`'s VStore’s handle
pointing to `Point(4,6)` is removed, `Point(2,1)` is created, and `$a` and
`$b`’s VStore is overwritten to contain a handle pointing to that new
`Point`. As there are now no VStores pointing to `Point(4,6)`, its
[destructor](14-classes.md#destructors) can run.

We can remove these aliases using `unset($a, $b)`:
<pre>
[VSlot $a (dead)]       [HStore Point [VSlot $x *] [VSlot $y *] (dead)]
                                                |            |
                                                V            V
[VSlot $b (dead)]             [VStore Int 2 (dead)]  [VStore Int 1 (dead)]
</pre>

Once all the aliases to the VStores are gone, the VStores can be
destroyed, in which case, there are no more pointers to the HStore, and
its [destructor](16-classes.md#destructors) can be run.

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
element is copied using *member-copy assignment* `=*`, which is defined
as follows:

```Hack
   $destination =* $source
```
-   If `$source`’s VStore has a refcount equal to 1, the Engine copies the
    array element using  value assignment (`destination = $source`).
-   If `$source`’s VStore has a refcount that is greater than 1, the Engine
    uses an implementation-defined algorithm to decide whether to copy the element
    using value assignment (`$destination = $source`) or byRef
    assignment (`$destination =& $source`).

Note the member-copy assignment `=*` is **not** an operator or language
construct in the Hack language, but instead it is used internally to
describe behavior for the engine for array copying and other operations

For the particular example above, member-copy assignment exhibits the
same semantics as value assignment for all conforming implementations
because all of the array elements’ VStores have a refcount equal to 1.
The first element VSlots in `$a`’s array and `$b`’s array point
to distinct VStores, each of which contain a distinct copy of the
integer value 10. The second element VSlots in `$a`’s array and `$b`’s
array point to distinct VStores, each of which contain a handle to the
same object HStore.

Let’s consider another example:
```Hack
$x = 123;
$a = array(array(&$x, 'hi'));
$b = $a;
```

Eager copying can produce two possible outcomes depending on the
implementation. Here is the first possible outcome:
<pre>
[VSlot $a *]---->[VStore Arr *]---->[HStore Array [VSlot 0 *]]
                                                           |
[VSlot $x *]-------------------------+   [VStore Arr *]&lt;---+
                                     |               |
[VSlot $b *]-->[VStore Arr *]        |               V
                           |         |  [HStore Array [VSlot 0 *][VSlot 1 *]]
                           V         |                          |          |
         [HStore Array [VSlot 0 *]]  |                          V          |
                                |    +---------------->[VStore Int 123]    |
                                V                          ^               V
                     [VStore Arr *]                        |   [VStore Str 'hi']
                                 |          +--------------+
                                 V          |
                     [HStore Array [VSlot 0 *] [VSlot 1 *]]
                                                        |
                                                        V
                                                     [VStore Str 'hi']
</pre>

Here is the second possible outcome:
<pre>
[VSlot $a *]---->[VStore Arr *]---->[HStore Array [VSlot 0 *]]
                                                           |
[VSlot $x *]-------------------------+  [VStore Arr *]&lt;----+
                                     |               |
[VSlot $b *]-->[VStore Arr *]        |               V
                           |         |  [HStore Array [VSlot 0 *] [VSlot 1 *]]
                           V         |                         |           |
         [HStore Array [VSlot 0 *]]  |                         V           |
                                |    +---------------->[VStore Int 123]    |
                                V                                          V
                     [VStore Arr *]                            [VStore Str 'hi']
                                 |
                                 V
                    [HStore Array [VSlot 0 *] [VSlot 1 *]]
                                           |           |
                                           V           V 
                                  [VStore Int 123]  [VStore Str 'hi']
</pre>

In both possible outcomes, value assignment with eager copying makes a
copy of `$a`’s array, copying the array’s single element using
member-copy assignment (which in this case will exhibit the same
semantics of value assignment for all implementations), which in turn
makes a copy of the inner array inside `$a`’s array, copying the inner
array’s elements using member-copy assignment. The inner array’s first
element VSlot points to a VStore that has a refcount that is greater than 1,
so an implementation-defined algorithm is used to decide whether to use value
assignment or byRef assignment. The first possible outcome shown above
demonstrates what happens if the implementation chooses to do byRef
assignment, and the second possible outcome shown above demonstrates
what happens if the implementation chooses to do value assignment. The
inner array’s second element VSlot points to a VStore that has a refcount
equal to 1, so value assignment is used to copy the inner array’s second
element for all conforming implementations that use eager copying.

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

Unlike conforming deferred string copy mechanisms discussed in [§§](#value-assignment-of-scalar-types-to-a-local-variable)
that must produce the same observable behavior as eager string copying,
deferred array copy mechanisms are allowed in some cases to exhibit
observably different behavior than eager array copying. Thus, for
completeness this section describes how deferred array copies can be
modeled in the abstract memory model and how conforming deferred array
copy mechanisms must behave.

Conforming deferred array copy mechanisms work by not making an array
copy during value assignment, by allowing the destination VStore to
share an array HStore with the source VStore, and by making a copy of
the array HStore at a later time if or when it is necessary. The
abstract model represents a deferred array copy relationship by marking
the destination VStore with a special “Arr-D” type tag and by sharing
the same array HStore between the source and destination VStores. Note
that the source VStore’s type tag remains unchanged. For the purposes of
this abstract model, the “Arr-D” type tag is considered identical to the
“Arr” type in all respects except when specified otherwise.

To illustrate this, let’s see how the previous example would be
represented under the abstract model assuming the implementation defers
the copying the array:

```Hack
$x = 123;
$a = array(array(&$x, ‘hi’));
$b = $a;
```
<pre>
[VSlot $a *]--->[VStore Arr *]--->[HStore Array [VSlot 0 *]]
                                    ^                    |
                                    |   [VStore Arr *]&lt;--+
[VSlot $b *]--->[VStore Arr-D *]----+               |
                                                    V
                                        [HStore Array [VSlot 0 *] [VSlot 1 *]]
                                                               |           |
                                                               V           |
[VSlot $x *]------------------------------------------>[VStore Int 123]    |
                                                                           V
                                                               [VStore Str 'hi']
</pre>

As we can see, both `$a`’s VStore (the source VStore) and `$b`’s VStore
(the destination VStore) point to the same array HStore. Note the
asymmetric nature of how deferred array copies are represented in the
abstract model. In the above example the source VStore’s type tag
remains unchanged after value assignment, whereas the destination
VStore’s type tag was changed to “Arr-D”.

When the engine is about to perform an array-mutating operation on a
VStore tagged “Arr” that participates in a deferred array copy
relationship or on a VStore tagged “Arr-D”, the engine must first take
certain actions that involve making a copy of the array (described in
the next paragraph) before performing the array-mutating operation. An
array-mutating operation is any operation can add or remove array
elements, overwrite existing array elements, change the state of the
array’s internal cursor, or cause the refcount of one or more of the
array’s element VStores or subelement VStores to increase from 1 to
a value greater than 1. This requirement to take certain actions before
performing an array-mutation operation on a VStore participating in a
deferred array copy relationship is commonly referred to as the
copy-on-write requirement.

When an array-mutating operation is about to be performed on a given
VStore X with an “Arr” type tag that participates in a deferred array
copy relationship, the engine must find all of the VStores tagged
“Arr-D” that point to the same array HStore that VStore X points to,
make a copy of the array (using member-copy assignment to copy the
array’s elements as described in [§§](#value-assignment-of-array-types-to-local-variables)), and update all of these
VStores tagged “Arr-D” to point to the newly created copy (note that
VStore X remains unchanged). When an array-mutation operation is about
to be performed on a given VStore X with an “Arr-D” type tag, the engine
must make a copy of the array (as described in [§§](#value-assignment-of-array-types-to-local-variables)), update VStore
X to point to the newly created copy, and change VStore X’s type tag to
“Arr”. These specific actions that the engine must perform on VStore at
certain times to satisfy the copy-on-write requirement are collectively
referred to as “array-separation” or “array-separating the VStore”. An
array-mutation operation is said to “trigger” an array-separation.

Note that for any VStore with an “Arr” type tag that participates in a
deferred array copy relationship, or for any VStore with an “Arr-D” type
tag, a conforming implementation may choose to array-separate the VStore
at any time for any reason as long as the copy-on-write requirement is
upheld.

Continuing with the previous example, consider the array-mutating
operation `$b[1]++`. Depending on the implementation, this can produce
one of three possible outcomes. Here is the one of the possible
outcomes:
<pre>
[VSlot $a *]---->[VStore Arr *]---->[HStore Array [VSlot 0 *]]
                                                           |
[VSlot $b *]-->[VStore Arr *]            [VStore Arr *]&lt;---+
                             |                       |
      +----------------------+              +--------+
      V                                     V
  [HStore Array [VSlot 0 *] [VSlot 1 *]]  [HStore Array [VSlot 0 *] [VSlot 1 *]]
                         |           |       ^                  |          |
                         |           V       |                  V          |
                         |   [VStore Int 1]  |            [VStore Int 123] |
                         V                   |             ^               V
                       [VStore Arr-D *]-----+              |   [VStore Str 'hi']
                                                           |
 [VSlot $x *]----------------------------------------------+
</pre>

As we can see in the outcome shown above, `$b`’s VStore was
array-separated and now `$a`’s VStore and `$b`’s VStore point to distinct
array HStores. Performing array-separation on `$b`’s VStore was necessary
to satisfy the copy-on-write requirement. `$a`’s array remains unchanged
and that `$x` and `$a[0][0]` still have an alias relationship with each
other. For this particular example, conforming implementations are
required to preserve `$a`’s array’s contents and to preserve the alias
relationship between `$x` and `$a[0][0]`. Finally, note that `$a[0]` and
`$b[0]` have a deferred copy relationship with each other in the outcome
shown above. For this particular example, a conforming implementation is
not required to array-separate `$b[0]`’s VStore, and the outcome shown
above demonstrates what happens when `$b[0]`’s VStore is not
array-separated. However, an implementation can choose to array-separate
`$b[0]`’s VStore at any time if desired. The other two possible outcomes
shown below demonstrate what can possibly happen if the implementation
choose to array-separate `$b[0]`’s VStore as well. Here is the second
possible outcome:
<pre>
[VSlot $a *]---->[VStore Arr *]---->[HStore Array [VSlot 0 *]]
                                                           |
[VSlot $b *]-->[VStore Arr *]            [VStore Arr *]&lt;---+
                          |                          |
                          V                          V
  [HStore Array [VSlot 0 *] [VSlot 1 *]]  [HStore Array [VSlot 0 *] [VSlot 1 *]]
                         |           |                           |           |
       +-----------------+           V                           |           |
       |                     [VStore Int 1]                  +---+           |
       V                                                     |               V
  [VStore Arr-D *]-->[HStore Array [VSlot 0 *] [VSlot 1 *]] | [VStore Str 'hi']
                                            |           |   |
                                    +-------+           |   |
                                    |                   V   |
                                    |    [VStore Str ‘hi’]  |
                                    V                       |
 [VSlot $x *]--------------------->[VStore Int 123]&lt;--------+
</pre>

Here is the third possible outcome:
<pre>
[VSlot $a *]---->[VStore Arr *-]---->[HStore Array [VSlot 0 *]]
                                                            |
[VSlot $b *]-->[VStore Arr *]             [VStore Arr *]&lt;---+
                            |                          |
                            V                          V
 [HStore Array [VSlot 0 *] [VSlot 1 *]]  [HStore Array [VSlot 0 *] [VSlot 1 *]]
                         |           |                           |           |
       +-----------------+           V                           |           |
       |                     [VStore Int 1]                  +---+           |
       V                                                     |               V
   [VStore Arr-D *]-->[HStore Array [VSlot 0 *] [VSlot 1 *]] | [VStore Str 'hi']
                                             |           |   |
                     [VStore Int 123]&lt;-------+           |   |
                                                         V   |
                                          [VStore Str 'hi']  |
                                                             |
 [VSlot $x *]--------------------->[VStore Int 123]&lt;--------+
</pre>

The second and third possible outcomes show what can possibly happen if
the implementation chooses to array-separate `$b[0]`’s VStore. In the
second outcome, `$b[0][0]` has an alias relationship with `$x` and
`$a[0][0]`. In the third outcome, `$b[0][0]` does not have an alias
relationship, though `$x` and `$a[0][0]` still have an alias relationship
with each other. The differences between the second and third outcome
are reflect that different possibilities when the engine uses
member-copy assignment to copy `$a[0]`’s arrays’s elements into `$b[0]`’s
array.

Finally, let’s briefly consider one more example:
```Hack
$x = 0;
$a = array(&$x);
$b = $a;
$x = 2;
unset($x);
$b[1]++;
$b[0]++;
echo $a[0], ' ', $b[0];
```

For the example above, a conforming implementation could output “2 1”,
“2 3”, or “3 3” depending on how it implements value assignment for
arrays.

For portability, it is generally recommended that programs written in
Hack should avoid performing value assignment with a right-hand side that
is an array with one or more elements or sub-elements that have an alias
relationship.

***Implementation Notes*** For generality and for simplicity, the
abstract model represents deferred array copy mechanisms in a manner
that is more open-ended and superficially different than the php.net
implementation’s model, which uses a symmetric deferred copy mechanism
where a single zval contains the sole pointer to a given Hashtable and
deferred array copies are represented as multiple slots pointing to the
same single zval that holds the array. Despite this superficial
difference, php.net’s implementation produces behavior that is
compatible with the abstract model’s definition of deferred array copy
mechanisms.

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

#### General ByRef Assignment
The sections above thus far have described the mechanics of byref assignment
with local variables. This section describes how byref assignment works when
general modifiable lvalue expressions are used on the left hand side and/or
the right hand side.

For example:

```Hack
$a = new Point(1, 3);
$b = 123;
$a->z =& $b;
```

will result in:
<pre>
[VSlot $a *]-->[VStore object *]-->[HStore Point [VSlot $x *] [VSlot $y *] [VSlot $z *]]
                                                           |            |            |
                                                           V            V            |
                                                  [VStore int 1] [VStore int 3]      |
[VSlot $b *]---------------->[VStore int 123]&lt;---------------------------------------+
</pre>

### Argument Passing
Argument passing is defined in terms of simple assignment[§§](#value-assignment-of-scalar-types-to-a-local-variable), [§§](#value-assignment-of-object-and-resource-types-to-a-local-variable), [§§](#value-assignment-of-array-types-to-local-variables), and [§§](10-expressions.md#simple-assignment)). 
That is, passing an argument to a function having a corresponding
parameter is like assigning that argument to that parameter.

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
