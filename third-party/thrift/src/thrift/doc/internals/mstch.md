# mstch-based code generators

* [Thrift Generators](#thrift-generators)
* [Creating a generator](#creating-a-generator)
* [Using Templates](#using-templates)
  * [Writing the driver](#writing-the-driver)
  * [Partial Templates](#partial-templates)
  * [Context types](#context-types)
  * [Context structure](#context-structure)
  * [Extending Context Maps](#extending-context-maps)
* [Using the new generator](#using-the-new-generator)

## Thrift Generators

A Thrift generator is a component of the Thrift compiler whose job it is to
generate code from the Thrift IDL for some particular language. This generated
code is used to implement Thrift clients and servers in the various languages
for which a generator exists.

This document describes how to create and use generators written using mstch,
a template library used by Thrift to increase maintainability and clarity of
the Thrift generators.

## Creating a generator

Each mstch generator is comprised of two components. Firstly, there is a C++
source file which drives template generation, controls output, and integrates
the generator with the rest of the compiler. Secondly, there are mstch template
files which contain the templates to be rendered by the C++ driver.

Taking the Java Swift generator as an example, the C++ driver is located in
`thrift/compiler/generate/t_mstch_swift_generator.cc` and the templates are
located in `thrift/compiler/generate/templates/java/swift`. The specific
directory under the `templates` root under which the templates are located is
specified in the constructor to `t_mstch_generator`.

Creating a new generator then requires the following steps:

1. Create the C++ file in which the driver will be contained.
  - Declare a new class inheriting from `t_mstch_generator` in this file.
    + Include `<thrift/compiler/generate/t_mstch_generator.h>` to extend this
      class.
  - Register the new generator with the rest of the compiler.
    + `THRIFT_REGISTER_GENERATOR(generator_flag, "Short name", "Long name");`
2. Implement the `generate_program` function for your generator.
3. Write templates, iterating on the driver if necessary.
  - Template filenames must end in `.mustache`, and this extension will be
    stripped when loading in the map of templates. This means that the canonical
    name of a template stored in `FooBar.mustache` will be `FooBar` from within
    the driver and other templates.

## Using Templates

### Writing the driver

There are several inherited functions which are useful when implementing
`generate_program` for a mstch generator.

- `t_mstch_generator::dump`
  + Takes an element of the Thrift AST and converts it into a mstch context
    node that can be provided to the renderer.
- `t_mstch_generator::render`
  + Given the name of a template and a mstch context node, renders the template
    and returns the results as a string.
    * A template's name is the same as its filename, but with the trailing
      ".mustache" removed.
- `t_mstch_generator::write_output`
  + Given render output and a relative path, writes the output to disk, putting
    the generated code in the proper absolute directory and recording that
    the file was written.
- `t_mstch_generator::get_option`
  + Returns the value associated with a particular
    option key passed on the command line. This lets you change small behavior
    details of your generator from the command line without needing a separate
    generator.

A simple body for `generate_program` is as follows:

```c++
auto context = this->dump(this->get_program());
auto output = this->render("MyTemplate", context);
this->write_output("my_output_file.txt", output);
```

### Partial Templates

From within a given template for a generator, other templates may be included
with the mstch inclusion syntax. If you have an existing template file named
"Foo.mustache", it can be included by writing `{{> Foo}}`. Templates
are permitted to recursively include themselves; however, care must be taken to
make sure that this terminates.

Note that the final newline, if present in a partial template file, is stripped
and not copied to the output. This allows template files to be properly
terminated with newlines without forcing a newline in the generated code. If a
template wants to emit an explicit newline at the end, simply add a blank line
or a commented line `{{! terminate with newline }}`.

### Context types

mstch is an untyped system, and tags may have different behavior determined by
the structure of the context passed in from C++. For clarity of documentation,
I ascribe to each key in the default Thrift context a type which shows how it
can be used.

Note: Throughout the generators, I use `:` as a separator in mstch keys and `?`
to indicate mstch keys which are booleans. These characters have no special
meaning within mstch, and are only used as a convention adopted by the
generators.

#### `string`

Keys with type `string` represent leaves in the mstch context. At template
render time, they expand directly into a string with no mstch control flow.

```
The name of this struct is {{struct:name}}
```

#### `bool`

Bools are keys which do not expand to text, but only indicate some property
about the context.

```mustache
{{#struct:plain?}}Rendered iff true{{/struct:plain?}}
{{^struct:plain?}}Rendered iff false{{/struct:plain?}}
```

#### `list<?>`

Lists represent zero or more elements of some other type, and function similar
to for-each loops in other languages. The body of the loop will be expanded
once per element in the list, and, on each iteration, the context will be
extended using each member of the list. While expanding lists, two boolean keys
named `first?` and `last?` will be available, indicating whether the current
iteration is the first or the last iteration, respectively.

```mustache
{{#program:structs}}
  {{#first?}}This is the first struct{{/first?}}
  This struct has the name {{struct:name}}
  {{^last?}}This is not the last struct{{/last?}}
{{/program:structs}}
```

You can also use the `^` syntax with lists, which will render the block if and
only if the list is empty.

```mustache
{{^program:structs}}
  There are no structs in this program
{{/program:structs}}
```


### Context structure

The following describes the default structure of the mstch contexts. These
defaults can be extended as described in a section below. For each
type there is a list of the keys available from within that type, the type of
each of those keys, and a short description of what the key means.

#### `program`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `program:name`          | `string`           | Name of thrift program                           |
| `program:path`          | `string`           | Path to thrift source file                       |
| `program:outPath`       | `string`           | Path to output directory for generation          |
| `program:includePrefix` | `string`           | Path to include prefix for Thrift IDL            |
| `program:structs`       | `list<struct>`     | Structs and Exceptions declared in this program  |
| `program:enums`         | `list<enum>`       | Enums declared in this program                   |
| `program:constants`     | `list<constant>`   | Constants declared in this program               |
| `program:services`      | `list<service>`    | Services declared in this program                |

####  `struct`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `struct:name`           | `string`           | Declared name of this struct                     |
| `struct:fields`         | `list<field>`      | Fields within this struct                        |
| `struct:union?`         | `bool`             | True iff declared as a union                     |
| `struct:exception?`     | `bool`             | True iff declared as an exception                |
| `struct:plain?`         | `bool`             | True iff not a union or exception                |

#### `field`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `field:name`            | `string`           | Name of this field                               |
| `field:key`             | `string`           | The numeric key of this field                    |
| `field:type`            | `type`             | The declared type of this field                  |
| `field:required?`       | `bool`             | True iff this field is required                  |
| `field:optional?`       | `bool`             | True iff this field is optional                  |
| `field:optInReqOut?`    | `bool`             | True iff this field is optional in, required out |
| `field:annotations`     | `list<annotation>` | List of annotations on this field                |

#### `enum`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `enum:name`             | `string`           | Name of this enum                                |
| `enum:values`           | `list<enum_value>` | Values of this enum                              |
| `enum:annotations`      | `list<annotation>` | List of annotations declared on this enum        |

#### `enum_value`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `enum_value:name`       | `string`           | Name of this variant of the enum                 |
| `enum_value:value`      | `string`           | Integer key for this enum variant                |

#### `type`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `type:void?`            | `bool`             | True iff this type is void                       |
| `type:string?`          | `bool`             | True iff this type is non-binary string          |
| `type:binary?`          | `bool`             | True iff this type is binary string              |
| `type:bool?`            | `bool`             | True iff this type is boolean                    |
| `type:byte?`            | `bool`             | True iff this type is byte                       |
| `type:i16?`             | `bool`             | True iff this type is i16 (16-bit signed int)    |
| `type:i32?`             | `bool`             | True iff this type is i32 (32-bit signed int)    |
| `type:i64?`             | `bool`             | True iff this type is i64 (64-bit signed int)    |
| `type:double?`          | `bool`             | True iff this type is double                     |
| `type:float?`           | `bool`             | True iff this type is float                      |
| `type:struct?`          | `bool`             | True iff this type is a struct                   |
| `type:struct`           | `struct`           | Information about struct, if this is a struct    |
| `type:service?`         | `bool`             | True iff this type is a service                  |
| `type:service`          | `struct`           | Information about struct, if this is a service   |
| `type:enum?`            | `bool`             | True iff this type is an enum                    |
| `type:enum`             | `enum`             | Information about struct, if this is an  enum    |
| `type:list?`            | `bool`             | True iff this type is a list                     |
| `type:listElemType`     | `type`             | Type of contained elements, if this is a list    |
| `type:stream?`          | `bool`             | True iff this type is a stream                   |
| `type:streamElemType`   | `type`             | Type of contained elements, if this is a stream  |
| `type:set?`             | `bool`             | True iff this type is a set                      |
| `type:setElemType`      | `type`             | Type of contained elements, if this is a set     |
| `type:map?`             | `bool`             | True iff this type is a map                      |
| `type:keyType`          | `type`             | Type of keys, if this is a set                   |
| `type:valueType`        | `type`             | Type of values, if this is a set                 |

#### `service`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `service:name`          | `string`           | Name of this service                             |
| `service:annotations`   | `list<annotation>` | Annotations declared on this service             |
| `service:functions`     | `list<function>`   | Functions defined on this service                |
| `service:extends?`      | `bool`             | True iff this service extends another service    |
| `service:extends`       | `service`          | The service this service extends, if one exists  |

#### `function`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `function:name`         | `string`           | Name of this function                            |
| `function:oneway?`      | `bool`             | True iff this function was declared oneway       |
| `function:returnType`   | `type`             | Return type of this function                     |
| `function:exceptions`   | `list<field>`      | List of checked exceptions in this function      |
| `function:annotations`  | `list<annotation>` | List of annotations on this function             |
| `function:args`         | `list<field>`      | List of arguments taken by this function         |

#### `annotation`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `annotation:key`        | `string`           | Key for this annotation                          |
| `annotation:value`      | `string`           | Value for this annotation                        |

#### `constant`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `constant:name`         | `string`           | Name of this constant                            |
| `constant:type`         | `type`             | Type of this constant                            |
| `constant:value`        | `value`            | Defined value of this constant                   |

#### `value`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `value:double?`         | `bool`             | True iff this value is a double                  |
| `value:doubleValue`     | `string`           | If this is a double, the double's value          |
| `value:integer?`        | `bool`             | True iff this value is an integer                |
| `value:integerValue?`   | `string`           | If this is an integer, the integer's value       |
| `value:nonzero?`        | `bool`             | True iff this is a non-zero numeric constant     |
| `value:string?`         | `bool`             | True iff this value is a string                  |
| `value:stringValue`     | `string`           | If this is a string, the string's value          |
| `value:list?`           | `bool`             | True iff this value is a list of other values    |
| `value:listElements`    | `list<value>`      | The list of values in this list                  |
| `value:map?`            | `bool`             | True iff this value is a map                     |
| `value:mapElements`     | `list<mapElement>` | The elements in this map of values               |


#### `mapElement`

| Key                     | Type               | Description                                      |
|-------------------------|--------------------|--------------------------------------------------|
| `element:key`           | `value`            | The key of this element of a map                 |
| `element:value`         | `value`            | The value of this element of a map               |

### Extending Context Maps

In some cases, mstch templates are not powerful enough to perform desired
operations and the default context described above does not contain enough
information to implement a feature for a given generator. If you would prefer
to sort the fields of a struct alphabetically, or escape the names of structs
in some way, that would not be possible using the default context alone. To
solve this, the generators support extending the default context with custom
generator-defined elements to overcome these limitations.

#### An Example: Escaping

Suppose the generated code for a given language needs to escape the name of each
struct before emitting. To accomplish this, we will override
`t_mstch_generator::extend_struct` in our subclass.

```cpp
mstch::map extend_struct(const t_struct& strct) const override {
  return {
    {"escapedName", my_escaping_function(strct.get_name())},
  };
}
```

Where `my_escaping_function` is some function that performs the desired
escaping. From within templates now, as long as a struct is currently in scope,
`{{struct:escapedName}}` will refer to the escaped name as defined above, as the
builtin `dump` functions are aware of the `extend_*` functions.

## Using the new generator

To invoke a mstch generator, run the following command:

```sh
thrift --gen $MY_GENERATOR -o $OUT_DIR $THRIFT_SRC
```

Where `MY_GENERATOR` is the name of the generator as specified in the C++ driver,
`OUT_DIR` is the desired output directory, and `THRIFT_SRC` is the Thrift IDL
source file.

Options can be specified as part of the string passed to `--gen`, for example:

```sh
thrift --gen mstch_swift:option=value -o $PWD $PWD/test.thrift
```
