# Compatibility

This spec discusses the semantics when the readers and writers interpret data using different schema. This is a common occurrence when upgrading a Thrift schema in a non-atomic fashion or reading previously stored serialized Thrift values. For example, when rolling out new clients/servers in a multi-machine environment or reading serialized values from disk or a database.

## [Data](data.md)

The following outlines the compatibility for changes to data:

|Change    |Wire compatible    |Code compatible    |Notes    |
|---    |---    |---    |---    |
|Add a field    |yes    |yes    |Dropped by old    |
|Remove a field    |yes    |no    |Dropped by new    |
|Rename a field    |yes    |no    |Only affect code gen and text format    |
|Change field type    |rarely    |no    |Only changes between utf-8 encoded string and binary are wire compatible    |
|Add enum value    |yes    |yes    |Preserved by old    |
|Remove enum value    |yes    |no    |Preserved by new    |
|New enum field with no 0 value    |yes    |yes    |    |
|Default on new non-optional field    |yes    |yes    |Gets new default    |
|Default on new optional field    |yes    |yes    |Stays unset    |
|Default changed on an non-optional field    |yes    |yes    |    |
|Default changed on an optional field    |yes    |yes    |    |
|Constant changed    |yes    |yes    |Old constant in old, new constant in new    |
|Required to unspecified    |yes    |no    |    |
|Unspecified to required    |yes    |no    |    |
|Optional to unspecified    |yes    |no    |    |
|Unspecified to optional    |yes    |no    |    |
|Optional to required    |yes    |no    |    |
|Required to optional    |yes    |no    |    |
|Required to terse    |yes    |no    |    |
|Terse to required    |yes    |no    |    |
|Optional to terse    |yes    |no    |    |
|Terse to optional    |yes    |no    |    |
|Unspecified to terse    |yes    |no    |    |
|Terse to unspecified    |yes    |no    |    |
|Mixin to non-mixin    |yes    |no    |Only code gen changes    |
|Non-mixin to mixin    |yes    |yes    |Only code gen changes    |
|Struct to union    |no    |no    |    |
|Union to struct    |no    |no    |    |
|Struct to exception    |yes    |yes    |    |
|Exception to struct    |yes    |yes    |    |
|Union to exception    |no    |no    |    |
|Exception to union    |no    |no    |    |
|Singular to container    |no    |no    |    |
|Container to singular    |no    |no    |    |

## [Interfaces](interface.md)

The following changes to interfaces are supported.

|Change    |Notes    |
|---    |---    |
|Add a method    |Unimplemented error from old servers    |
|Remove a method    |Unimplemented error from new servers    |
|Change arguments    |See ['Data'](https://github.com/facebook/fbthrift/blob/main/thrift/doc/spec/definition/compatibility.md#data)    |

