
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return all types which transitively extend, implement, or use the given
base type, as ` (name, path, kind, abstract) ` tuples




``` Hack
namespace HH\Facts;

function transitive_subtypes<T>(
  classname<T> $base_type,
  ?DeriveFilters<string, dynamic> $filters = NULL,
): vec<(string, TypeKind, classname<T>, bool)>;
```




The 'kind' and 'derive_kind' filters passed in determine which relationships
and types we look at while traversing the inheritance graph. So if you
filter traits out, we'll exclude classes which are only related because
they ` use ` a trait which `` implements `` the interface you passed in.




The 'attributes' filters passed in will be applied to the final list of
transitive subtypes. So if you look for types with the ` <<Oncalls('team')>> `
attribute, we'll only filter the final list of subtypes, instead of ignoring
all types that don't have the given attribute.




Throws InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<T> $base_type `
+ ` ?DeriveFilters<string, dynamic> $filters = NULL `




## Returns




* ` vec<(string, TypeKind, classname<T>, bool)> `
<!-- HHAPIDOC -->
