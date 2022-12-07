# Schema Compatibility

<!-- https://www.internalfb.com/intern/wiki/Thrift/Thrift_Guide/IDL/SchemaEvolution/?noredirect -->

## **Compatibility between different Thrift Structs (Versioning)**

It is normal to make changes to Thrift files as part of software development.  However, unlike a single program where everything gets rebuilt at once, multiple programs that interact with each other via Thrift get rebuilt at different times.  This requires the ability for different versions of the same Thrift file to be compatible with each other.  The rules for serialization and deserialization described earlier are designed to support this compatibility.

Clearly not all changes to a Thrift struct permit the old and new version to be compatible with each other. For example a change to the type of a field results in incompatible versions.

This section describes when two Thrift structs are compatible with each other - i.e., when it is always possible to serialize from one struct and deserialize into the other without problems.  This section simply clarifies information already presented in the previous sections on **Serialization** and **Deserialization**.  Everything described here can be inferred by reading those sections.

NOTE: Thrift fields using **"[default requiredness](https://thrift.apache.org/docs/idl#default-requiredness-implicit)"** (i.e. niether set as **required** or **optional**) are defined as **"unqualified"** in this document.


### **Serialization by name**

When serializing by name, the id of the field is not used, and can be ignored.  For two Thrift struct types **Struct1** and **Struct2** to be compatible in this case, the following conditions must hold:

* If **Struct1** has an **optional** field with name **n** of type **T**, then **Struct2** can have a field with name **n**, but is not required to have a field with this name.  However if the field is present, then its type must be **T** and it should be either **optional** or **unqualified**.
* If **Struct1** has an **unqualified** field with name **n** of type **T**, then **S2** can have a field with name **n**, but is not required to have a field with this name.  However if the field is present, then its type must be **T** (and can be any of **required**, **optional**, or **unqualified**).

### **Serialization by id**

When serializing by id, it is the id of the fields in the two types that must match, not their names.

For two Thrift struct types **Struct1** and **Struct2** to be compatible when serializing by id, the following conditions must hold:

* If **Struct1** has an **optional** field with id **i** of type **T**, then **Struct2** can have a field with id **i**, but is not required to have a field with this id.  However if the field is present, then its type must be **T** and it should be either **optional** or **unqualified**.
* If **Struct1** has an **unqualified** field with id **i** of type **T**, then **Struct2** can have a field with id **i**, but is not required to have a field with this id.  However if the field is present, then its type must be **T** (and can be any of **required**, **optional**, or **unqualified**).

### **Consequence of incompatible field**

The incompatible field will be dropped during deserialization. There will be temporary data loss until the new version is pushed everywhere.

### **How to avoid incompatible versions**

Instead of modifying the name/id and type, it's recommanded to

1. Create a new field with desired name/id and type
2. Modify writer to write to both field
3. Push service with new changes to the fleet
4. Remove old field

e.g., for the following struct

```
struct Foo {
  1: i32 field;
}
```
We want to change type of `Foo.field` from `i32` to `i64`. If we change it directly, deserializer will drop this field due to type mismatches. To avoid this problem, it's recommanded to create a new field first, then double writes the data to both field before deleting the old field.

```
struct Foo {
  1: i32 field;
  2: i64 new_field;
}
```
### **Changes that maintain compatibility**

Given the rules above, the following changes to Thrift structs will maintain compatibility between old and new versions:

* Changing a **required** field to **unqualified**
* Changing an **optional** field to **unqualified**
* Changing an **unqualified** field to **required**
* Changing an **unqualified** field to **optional**
* Deleting an **optional** field
* Deleting an **unqualified** field
* Adding a new **optional** field
* Adding a new **unqualified** field
* Renaming a field but keeping its field id the same (only when serializing by field id)

WARNING: The **unqualified** to **optional** migration while compatible from the protocol standpoint, is potentially unsafe because it changes access semantics. For example, in C++ reading an unset optional fields may throw. Also if any C++ code uses legacy field writes (`s.field = ..`) to set a newly optional field, field values will be silently dropped during serialization after making the field optional. Ensure all clients are migrated to setters (`s.field_ref() = ..`) first.


### **A degenerate example**

Consider the following degenerate example where the id's are swapped in the two structs:

```
struct Struct1 {
  1: i32 x,
  2: i32 y,
}

struct Struct2 {
  2: i32 x,
  1: i32 y,
}
```
When serializing by field name, the field ids are ignored.  Field `x` in `Struct1` is copied to field `x` in `Struct2`, and field `y` in `Struct1` is copied to field `y` in `Struct2`.

When serializing by field id, field `x` in `Struct1` is copied to field `y` in `Struct2`, and field `y` in `Struct1` is copied to field `x` in `Struct2`.

I.e., `Struct1` and `Struct2` are compatible regardless of the kind of serialization used, but the results are very different based on the kind of serialization used.

> Note: Generally changes that maintain compatibility when serializing by field id do not necessarily maintain compatibility when serializing by field name.


> Note: If you want to delete a **required** field from a Thrift struct, you can do it in two steps: First change the field from **required** to **unqualified**.  Then, once you are sure that all usage of the older version (with **required**) have been updated to use the new version, then you can make the second change to delete the field.


### **Versioning with enumeration types**

When a Thrift struct contains fields with enumeration types, the following modifications to the enumeration types maintain compatibility:

* Deleting an enumerator
* Adding a new enumerator
* Renaming an enumerator but keeping its integer value the same (only when serializing by field id)

---
