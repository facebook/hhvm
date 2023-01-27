Type aliases have special rules when it comes to internal types depending on which kind of type alias is used.

## Internal type aliases
A type alias declared with `type` can be marked internal. Just as with classes, this means that the type can only be referenced from within the module.

```hack
//// newmodule.hack
new module foo {}

//// foo.hack
module foo;
internal class Foo {}
internal type FooInternal = Foo;
internal newtype FooOpaque = FooInternal;
internal newtype FooOpaque2 as Foo = Foo;
```

Internal newtypes can also be constrained by other internal types, since they can only be used from within the module.

## Public Opaque type aliases
Since public opaque type aliases hide their implementations from users outside of the current file, you can implement them with internal types. The opaque type alias acts as an empty interface for the internal type. You cannot, however, constrain them with an internal type, since a public user would not know what type it's being constrained by.

```hack no-extract
newtype FooOpaque = FooInternal; // ok
newtype FooErr as FooInternal = FooInternal; // error, FooInternal is an internal type, cannot be used as constraint
```

## Public transparent type aliases
Transparent type aliases leak their implementation, so they cannot be implemented by internal types.

```no-extract
type FooOpaque = FooInternal; // error, cannot use internal type FooInternal in transparent type FooOpaque.
```

## Module-level type aliases
We also introduce a new type alias known as a **module-level** type alias. You can create one using the syntax `module newtype TFoo as ... = ...`.
A module-level type alias is opaque outside of the module it's defined in, and transparent inside (rather than just being opaque outside of the file it's defined in). Since they hide their implementations from outside the module, you can use internal types from within a module to implement them. They still cannot be constrained by internal types.

```hack no-extract
module newtype FooModule = FooInternal; // ok
module newtype FooModuleErr as FooInternal = FooInternal; // error, FooInternal is an internal type, cannot be used as a constraint
```
Since the purpose of module newtypes is to create an interface surrounding a module boundary, you cannot mark module newtypes themselves internal.

```hack error
internal module newtype FooModuleErr2 = int; // Parse error
```
