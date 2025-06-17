# Object Model

import specStackImageUrl from './spec-stack.png';
import thriftTypeImageUrl from './thrift-type.png';
import dataVsDatumsImageUrl from './data-vs-datums.png';
import typeSystemImageUrl from './type-system.png';
import valueMappingImageUrl from './value-mapping.png';
import serializeDeserializeImageUrl from './serialize-deserialize.png';
import serializeDeserializeDecomposedImageUrl from './serialize-deserialize-decomposed.png';

<!-- An image with an optional caption -->
export const Figure = (props) => {
  const {image, caption, width, height} = props;
  const containerStyle = {
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    justifyContent: "center",
    border: "thin silver solid",
  };
  const captionStyle = {
    fontStyle: "italic",
    fontSize: "smaller",
  };
  return (
    <figure style={containerStyle}>
      <img src={image} alt={caption} align="center" {...{width, height}} />
      {caption && <figcaption style={captionStyle}>{caption}</figcaption>}
    </figure>
  );
}

<!-- A "keyword" — a word with a precise and exact meaning in the context of this document -->
export const KW = (props) => {
  const {children} = props;
  return <i>{children}</i>;
}

<!-- A non-heading anchor within the page for internal links -->
export const Bookmark = (props) => {
  const anchorPointStyle = {
    // Otherwise, the navbar of the page will hide the anchor point
    scrollMarginTop: "calc(var(--ifm-navbar-height) + 0.5rem)",
  };
  const bookmarkStyle = {
    paddingLeft: "0.25rem",
    userSelect: "none",
  };
  return (
    <>
      <span {...props} style={anchorPointStyle} />
      <a href={`#${props.id}`} style={bookmarkStyle}>&#9873;</a>
    </>
  );
};

<!-- Center some content horizontally, such as a table -->
export const CenterHorizontally = (props) => {
  const { children } = props;
  const containerStyle = {
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    justifyContent: "center",
  };
  return (
    <div style={containerStyle}>
      {children}
    </div>
  );
};

<!-- Ditto mark indicating "same as before" -->
export const Ditto = () => {
  return <span>&#12291;</span>;
};

export const CenteredDitto = () => {
  return <CenterHorizontally><Ditto/></CenterHorizontally>;
}

<!-- Indentation in code -->
export const Indent = (props) => {
  const { times = 1 } = props;
  return <span dangerouslySetInnerHTML={{ __html: '&nbsp;'.repeat(2 * times)}} />;
};

<!-- Container for operation definitions -->
export const Operation = (props) => {
  const { children } = props;
  const containerStyle = {
    paddingLeft: "1rem",
    marginLeft: "0.5rem",
    borderLeft: "thin silver solid",
  };
  return (
    <div style={containerStyle}>{children}</div>
  );
};

<!-- Container for contract requirement description -->
export const Requirement = (props) => {
  return <Operation {...props} />;
};

<!-- Shorthands for subscripted code -->

export const S0 = () => {
  return <code>S<sub>0</sub></code>;
};

export const S1 = () => {
  return <code>S<sub>1</sub></code>;
};

export const T0 = () => {
  return <code>T<sub>0</sub></code>;
};

export const T1 = () => {
  return <code>T<sub>1</sub></code>;
};

export const V0 = () => {
  return <code>v<sub>0</sub></code>;
};

export const V1 = () => {
  return <code>v<sub>1</sub></code>;
};

export const F0 = () => {
  return <code>f<sub>0</sub></code>;
}

export const F1 = () => {
  return <code>f<sub>1</sub></code>;
};

## Introduction

The *Thrift Object Model* defines all possible data types within Thrift's type system (built-in or user-defined), and all operations (and semantics thereof) that allow interaction with those data types.
The primary objective of this specification is to establish consistency, predictability, and ease of use by providing clear guidelines for interaction with Thrift data.

The *Thrift Object Model* is language-agnostic, which allows it to be applied across various development environments.
This makes it valuable not only to the Thrift team but also to maintainers of community-supported languages and libraries that build upon the Thrift ecosystem.

The *Thrift Object Model* is the most foundational specification in the Thrift ecosystem, but does not capture its entirety.
There are several other specifications that are fundamental to Thrift, which build on top of the *Thrift Object Model*:
* The [Interface Definition Language](/idl/index.md) (IDL) — governs the syntax and semantics of `.thrift` files.
* Various Serialization protocols (e.g. [Compact](https://github.com/apache/thrift/blob/master/doc/specs/thrift-compact-protocol.md) & [Binary](https://github.com/apache/thrift/blob/master/doc/specs/thrift-binary-protocol.md)).
* The APIs of generated code in each supported target language.

<!-- https://lucid.app/lucidchart/03314913-6de3-4bf7-892c-b9bc6c58f3bd/edit?viewport_loc=407%2C313%2C1768%2C1779%2CQtNRXHD9BHA9&invitationId=inv_395eb116-6626-4d5b-bb58-fb973bf5c3ca -->
<Figure image={specStackImageUrl} caption="Layer's of Thrift's conceptual stack" width="50%" />

## Thrift Data types

### Overview

The *Thrift Object Model* is described through Thrift Data Types, which fall in one of the following categories:
1. primitive types
2. container types
3. user-defined types

The properties of Thrift types are expressed in terms of a few key characteristics defined hereunder.
While terms such as "schema" or "dataset" may be familiar to readers, their meaning throughout this document should be understood to be precisely the one below — neither more, nor less.

### Defining Characteristics

A Thrift <Bookmark id="type">**<KW>type</KW>**</Bookmark> is defined by a <KW>type identifier</KW> and a <KW>schema</KW>:

* The **<KW>type identifier</KW>** (or **<KW>typeid</KW>**) uniquely identifies the type, and has a stable canonical textual representation (the **<KW>typeid name</KW>**).
* The <Bookmark id="schema">**<KW>schema</KW>**</Bookmark> determines the set of values and operations that are valid for the <KW>type</KW>. It is defined by a <KW>dataset</KW> and, when applicable, <KW>user-specified properties</KW>:
  * The <Bookmark id="dataset">**<KW>dataset</KW>**</Bookmark> of a given <KW>schema</KW> is the set of all valid <KW>datums</KW> for that <KW>schema</KW>.
    * For example, the <KW>dataset</KW> of the Thrift [<KW>boolean type</KW>](#primitive-types) is a set that consists of the following datums: <KW>true</KW>, <KW>false</KW>.
    * A <KW>datum</KW> that is contained in the <KW>dataset</KW> of a <KW>schema</KW> is said to **conform to the <KW>schema</KW>**.
      * For example, the number 5 conforms to the schema of all Thrift [integer types](#primitive-types), but does not conform to the schema of the Thrift boolean type.
  * The <Bookmark id="user-specified-properties">**<KW>user-specified properties</KW>**</Bookmark> express additional semantics (or constraints) for the <KW>type</KW> in the context of its <KW>type system</KW>.
    * For example, the names associated with specific values of Thrift <KW>enum types</KW>, as well as the <KW>sealed</KW>-ness of <KW>structured types</KW>, are user-specified properties.

A Thrift <Bookmark id="value">**<KW>value</KW>**</Bookmark> is defined by:
* a <KW>type</KW> `T`
* a <KW>datum</KW> that conforms to the <KW>schema</KW> of `T`

<!-- https://lucid.app/lucidchart/03314913-6de3-4bf7-892c-b9bc6c58f3bd/edit?viewport_loc=-1176%2C-1093%2C1768%2C1779%2CVZHRtRBoToTE&invitationId=inv_395eb116-6626-4d5b-bb58-fb973bf5c3ca -->
<Figure image={thriftTypeImageUrl} caption="The defining characteristics of a Thrift type" width="75%" />

:::info common notation for <KW>schema</KW> and <KW>datasets</KW>
The <KW>schema</KW> of a <KW>type</KW> `T` may be written as `schema(T)`.

The <KW>dataset</KW> of a <KW>schema</KW> `S` may be written as `dataset(S)`.

`dataset(schema(T))` may be shortened as `dataset(T)` — i.e. "<KW>dataset</KW> of `T`" is shorthand for "<KW>dataset</KW> of the <KW>schema</KW> of `T`".
:::

:::info <KW>value</KW> & <KW>datum</KW>
A <KW>datum</KW> might conform to the <KW>schema</KW> of two different <KW>types</KW>, however the <KW>values</KW> defined by that <KW>datum</KW> and different types are distinct.

For example, the <KW>datum</KW> consisting of the integral number `5` conforms to the <KW>schema</KW> of both 8-bit and 32-bit signed integers.
i.e. `5` ∈ `dataset(byte)` and `5` ∈ `dataset(i32)`, but the <KW>value</KW> `(byte, 5)` is **distinct** from the <KW>value</KW> `(i32, 5)`.
:::

:::info <KW>data</KW> or <KW>datums</KW>?
The noun *datum* has two plural forms: *data* and *datums*.
This document uses the latter form ("*datums*") to refer to elements in the <KW>datasets</KW> (and <KW>schemas</KW>) of Thrift <KW>types</KW>.
The usage of this somewhat unorthodox form over the most common one is intentional, to capture the specific context and avoid confusion with the usage of "*data*" as a generic term for information.
<Figure image={dataVsDatumsImageUrl} width="50%" />
:::

### Type System

A **<KW>type system</KW>** is a collection of <KW>types</KW>, where each <KW>type</KW>, and all the <KW>types</KW> it refers to, are fully defined within the same <KW>type system</KW>.

A valid Thrift <KW>type system</KW> must, therefore, include (implicitly or explicitly):

* all [<KW>primitive types</KW>](#primitive-types)
* all instantiable [<KW>container types</KW>](#container-types)
* for every included [<KW>structured type</KW>](#structured-types):
  * all <KW>types</KW> referenced in the <KW>structured type</KW>'s <KW>fields</KW>
* for every included [<KW>opaque alias type</KW>](#opaque-alias-types):
  * the underlying <KW>target type</KW>.

<!-- https://lucid.app/lucidchart/03314913-6de3-4bf7-892c-b9bc6c58f3bd/edit?viewport_loc=-594%2C-1106%2C2309%2C2323%2CBh5LnMDg_95q&invitationId=inv_395eb116-6626-4d5b-bb58-fb973bf5c3ca -->
<Figure image={typeSystemImageUrl} caption="Overview all Thrift types and their relationships" width="100%" />

:::info <Bookmark id="type-typeids-typeid-names"><KW>type</KW>, <KW>typeid</KW>, and <KW>typeid name</KW></Bookmark>
Within a <KW>type system</KW>, the following form a bijection with one another:
* set of <KW>types</KW>,
* their <KW>typeids</KW>, and
* their <KW>typeid names</KW>

When there is no ambiguity, the specification henceforth uses these terms interchangeably.
:::

### Built-in Types

#### Primitive Types

* **<Bookmark id="boolean-type">Boolean</Bookmark>**
  * One of the two [standard logic](https://en.wikipedia.org/wiki/Classical_logic) truth values — <KW>true</KW> or <KW>false</KW>
* **<Bookmark id="fixed-width-signed-integer-types">Fixed-width signed integers</Bookmark>**: 8, 16, 32, or 64 bits
  * A number from the set of integers in the two's complement scheme of the corresponding width, i.e. from <code>-2<sup>bits-1</sup></code> to <code>2<sup>bits-1</sup> - 1</code> (inclusive).
  For example:
    * 8 bits: `-128` to `+127`
    * 16 bits: `-32,768` to `+32,767`
    * etc.
* **<Bookmark id="floating-point-number-types">Floating point numbers</Bookmark>**: single and double precision
  * A number from the set of real numbers in [IEEE 754](https://en.wikipedia.org/wiki/IEEE_754) `binary32` and `binary64` formats, respectively, except that
    * `NaN` is <Bookmark id="nan">excluded</Bookmark>— there is no such <KW>datum</KW> in the <KW>dataset</KW> of Thrift floating point numbers.
    * Zero is <Bookmark id="zero-not-signed">not signed</Bookmark> — there is exactly one <KW>datum</KW> that represents the [additive identity](https://en.wikipedia.org/wiki/Additive_identity).
* **<Bookmark id="text-type">Unicode text</Bookmark>**
  * An unbounded sequence of Unicode code points.
* **<Bookmark id="byte-array-type">Byte array</Bookmark>**
  * An unbounded sequence of (8-bit) bytes.
* **<Bookmark id="any-type">Any</Bookmark>**
  * A dynamically type-checked container of (at most) a single Thrift <KW>value</KW>.
    * An *empty* Any contains no <KW>value</KW>.
    * A non-empty Any is defined by:
      * A [<KW>typeid</KW>](#type-identifiers-typeid) — identifies the <KW>type</KW> of the contained <KW>value</KW>.
      * A [<KW>cipher<sub>P</sub></KW>](#cipher) — an opaque representation of the <KW>datum</KW> of the contained <KW>value</KW>.
      * A descriptor of a [<KW>protocol</KW>](#serialization-protocols) `P` — describes how to obtain the <KW>value</KW> from the aforementioned <KW>cipher<sub>P</sub></KW>.

:::info Primitive <KW>typeid names</KW>
*Primitive types* shall henceforth be referred to by their [<KW>typeid names</KW>](#typeid-names) (which match the [Thrift IDL](/idl/index.md) where applicable):
* **Boolean**: `bool`
* **Fixed-width signed integers**: `byte`, `i16`, `i32`, `i64`
* **Floating point numbers**: `float`, `double`
* **Unicode text**: `string`
* **Byte array**: `binary`
* **Any**: `any`
:::

#### Container Types

Thrift supports three <KW>container type [constructors](https://en.wikipedia.org/wiki/Type_constructor)</KW>: **<KW>list</KW>**, **<KW>set</KW>**, and **<KW>map</KW>**. When instantiated, these constructors produce a <KW>type</KW>.
For example, a <KW>list</KW> instantiated with the <KW>type</KW> `i32` produces the <KW>type</KW> `list<i32>`.

##### List of V

A Thrift **<KW>list of `V`</KW>** <KW>type</KW> is a finite, **ordered sequence of <KW>values</KW>** where:
* The <KW>type</KW> of each <KW>value</KW> is `V`
* `V` can be any <KW>type</KW>

##### Set of V

A Thrift **<KW>set of `V`</KW>** type is a finite, **unordered collection of unique <KW>values</KW>**, where:
* The <KW>type</KW> of each <KW>value</KW> is `V`
* No two <KW>values</KW> in the set [compare equal](#equality)
* The <KW>type</KW> `V` is [<KW>sealed</KW>](#sealed-types)

##### Map of K to V

A Thrift **<KW>map of `K` to `V`</KW>** <KW>type</KW> is defined as a finite, **unordered collection of key-value pairs** (`k` → `v`) with **unique** keys, where:
* `k` and `v` are Thrift <KW>values</KW>.
* The <KW>type</KW> of each key (`k`) is `K`
* The <KW>type</KW> of each value (`v`) is `V`
* No two keys in the set [compare equal](#equality)
* The <KW>type</KW> `K` is [<KW>sealed</KW>](#sealed-types)
* `V` can be any <KW>type</KW>

### User-defined Types

#### Structured Types

Structured types are **composite <KW>user-defined types</KW>**, consisting of a variable number of <KW>fields</KW>, each with its own <KW>type</KW>.
Thrift supports two categories of structured types: <KW>structs</KW> and <KW>unions</KW>.

A Thrift **<KW>structured type</KW>** is defined by:

* A [<KW>Thrift URI</KW>](#thrift-uri)
* A set of <KW>fields</KW>
* A set of <KW>user-specified properties</KW>, which are:
  * The <KW>type</KW>’s <Bookmark id="structured-type-sealed">[<KW>sealed</KW>](#sealed-types)-ness</Bookmark>.
  * A set of [<KW>annotation maps</KW>](#annotation-maps):
    * an <KW>annotation map</KW> for the <KW>structured type</KW> itself, and
    * an <KW>annotation map</KW> for each <KW>field</KW>.

A <Bookmark id="field">**<KW>field</KW>**</Bookmark> is defined by:

* A [<KW>field identity</KW>](#field-identity)
* A [<KW>presence qualifier</KW>](#presence-qualifier)
* A <KW>type</KW>
* Optionally, a [<KW>custom default value</KW>](#custom-default-value)

A <Bookmark id="field-identity">**<KW>field identity</KW>**</Bookmark> consists of:

* an integral **<KW>field id</KW>**
  * Must be unique among all <KW>field ids</KW> for a given <KW>type</KW>.
  * Must be between `-32,768` and `+32,767` (inclusive). i.e. the range of signed 16 bit integers.
* a textual **<KW>field name</KW>**
  * Must be non-empty and unique among all <KW>field names</KW> for a given <KW>type</KW>.

Two <KW>fields</KW> share the same identity if either their <KW>field ids</KW> or <KW>field names</KW> are equivalent.

A <Bookmark id="presence-qualifier">**<KW>presence qualifier</KW>**</Bookmark> is one of the following:

* **<KW>optional</KW>** — a <KW>value</KW> of the containing <KW>type</KW> may or may not have a <KW>value</KW> for this <KW>field</KW>.
* **<KW>always-present</KW>** — a <KW>value</KW> of the containing <KW>type</KW> always has a <KW>value</KW> for this <KW>field</KW>.
* **<KW>terse</KW>** — identical to <KW>always-present</KW>, but cannot have a <KW>custom default value</KW>.

If the <KW>type</KW> of a <KW>field</KW> forms a cycle of references between the <KW>structured types’ fields</KW> (recursive types), then at least one of those <KW>fields</KW> must have the <KW>presence qualifier</KW> of <KW>optional</KW>.

The <Bookmark id="custom-default-value">**<KW>custom default value</KW>**</Bookmark> is a <KW>value</KW> of the aforementioned <KW>type</KW>, which informs the [standard default value](#operation-createstandarddefault) of the enclosing <KW>type</KW>.

If the <KW>presence qualifier</KW> is <KW>optional</KW> or <KW>terse</KW>, there cannot be a <KW>custom default value</KW>.

:::note **Implementation Detail** — <KW>required</KW> qualifier
Historically, the Thrift IDL supported a <KW>**required** field qualifier</KW>.
Its semantics were quite confusing, as they combined the <KW>always-present</KW> <KW>presence qualifier</KW> above with specific serialization constraints.
It is deprecated, and effectively unsupported in practice.
:::

##### Datums

The <KW>datum</KW> of a <KW>structured type</KW> `T` is a **collection of <KW>field values</KW>**, where...

* A <Bookmark id="field-value">**<KW>field value</KW>**</Bookmark> consists of:
  * a [<KW>field identity</KW>](#field-identity), that matches the identity of exactly one <KW>field</KW> of `T`
  * a [<KW>value</KW>](#value) that conforms to that <KW>field</KW>’s <KW>type</KW>
* and the collection has...
  * **exactly one** *field value* for every <KW>field</KW> of `T` whose <KW>presence qualifier</KW> is **not <KW>optional</KW>**.
  * **at most one** *field value* for every <KW>field</KW> of `T` whose <KW>presence qualifier</KW> is **<KW>optional</KW>**.
    * i.e., an <KW>optional</KW> <KW>field</KW> may or may not have a corresponding <KW>field value</KW>, whereas a non-<KW>optional</KW> (<KW>always-present</KW> or <KW>terse</KW>) field always has a <KW>field value</KW>.

##### Struct

A Thrift **<KW>struct</KW>** is a <KW>structured type</KW> with no restrictions.

:::note **Implementation Detail** — <KW>exceptions</KW>
The Thrift IDL defines one more <KW>structured type</KW> — **<KW>exceptions</KW>**. For the purpose of the *Thrift Object Model*, they are equivalent to <KW>structs</KW>.
:::

##### Union

A Thrift **<KW>union</KW>** is a <KW>structured type</KW> with the following additional constraints:
* <KW>fields</KW> must have a <KW>presence qualifier</KW> of <KW>optional</KW>
* The <KW>datum</KW> of a Thrift <KW>union type</KW> has **at most** one <KW>field value</KW>.

#### Enum Types

A Thrift **<KW>enum</KW>** is defined by:
* A [*Thrift URI*](#thrift-uri)
* A set of <KW>user-specified properties</KW>, which are:
  * A mapping from unique textual <Bookmark id="enum-name"><KW>enum-names</KW></Bookmark> to their corresponding unique [32-bit signed integer](#fixed-width-signed-integer-types) <Bookmark id="enum-value"><KW>enum-values</KW></Bookmark>.
  * A set of [<KW>annotation maps</KW>](#annotation-maps):
    * an <KW>annotation map</KW> for the <KW>enum type</KW> itself, and
    * an <KW>annotation map</KW> for each specified <KW>enum-name</KW>.

#### Opaque Alias Types

Opaque aliases associate a new <KW>type identity</KW> with the <KW>schema</KW> of an existing [<KW>built-in type</KW>](#built-in-types).

A Thrift **<KW>opaque alias</KW>** is defined by:
* A [<KW>Thrift URI</KW>](#thrift-uri)
* A <KW>target type</KW>, which must not be a [<KW>user-defined type</KW>](#user-defined-types).
* A set of <KW>user-specified properties</KW>, which are:
  * an <KW>annotation map</KW>

:::info Restricting Opaque Aliases to non-user-defined types
Thrift <KW>opaque alias types</KW> are restricted to <KW>built-in types</KW> because defining such aliases for <KW>user-defined types</KW> would be redundant.
For any given <KW>user-defined type</KW> `T`, users can define another <KW>type</KW> `U` that has the same [<KW>schema</KW>](#schema) (and, therefore, [<KW>dataset</KW>](#dataset)) but a different <KW>URI</KW>.
There are no other mechanisms for doing so with <KW>built-in types</KW>.
:::

### Type Identifiers (typeid)

All Thrift <KW>types</KW> have exactly one, unique **<KW>type identifier</KW>** (or **<KW>typeid</KW>** for short).

The <KW>typeid</KW> of a given <KW>type</KW> is immutable — <KW>types</KW> whose <KW>typeids</KW> differ are distinct, even if their <KW>schemas</KW> are identical.

The main purpose of <KW>typeids</KW> is to associate <KW>types</KW> across two different <KW>type systems</KW>, for example, to assess [<KW>schema</KW> compatibility](#schema-change).

#### Thrift URI

The <KW>type identifier</KW> of any [<KW>user-defined type</KW>](#user-defined-types) is Uniform Resource Identifier (URI) that conforms to [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986) and must be unique within a <KW>type system</KW>.

* The <KW>path</KW> component of the URI must not be empty.
* If the <KW>scheme</KW> component of the URI is absent, then `fbthrift` is [implied](/features/universal-name.md).

An example of a <KW>Thrift URI</KW> is `meta.com/search/NewSearchRequest`.

:::info <KW>URI</KW> uniqueness
<KW>Thrift URIs</KW> must be unique across all <KW>type systems</KW>. Two distinct type definitions with the same <KW>URI</KW> can never form a valid <KW>type system</KW> with each other.
:::

#### Typeid Names

Every <KW>typeid</KW> has a canonical, stable, and human-readable textual representation, referred to as its <KW>name</KW>.

<CenterHorizontally>

| Thrift <KW>type</KW> | <KW>typeid name</KW> |
| ----- | ----- |
| Boolean | `“bool”` |
| 8-bit signed integer | `“byte”` |
| 16-bit signed integer | `“i16”` |
| 32-bit signed integer | `“i32”` |
| 64-bit signed integer | `“i64”` |
| 32-bit floating point number | `"float"` |
| 64-bit floating point number | `"double"` |
| Unicode text | `“string”` |
| Byte array | `“binary”` |
| <KW>User-defined type `T`</KW> | [<KW>Thrift URI</KW> of `T`](#thrift-uri)<br /><br />e.g. `"meta.com/search/NewSearchRequest"` |
| <KW>list of `V`</KW> | `“list<"` <KW>typeid name</KW> of `V` `">”`<br /><br />e.g. `"list<meta.com/search/NewSearchRequest>"` |
| <KW>set of `K`</KW> | `“set<"` <KW>typeid name</KW> of `K` `">”`<br /><br />e.g. `"set<i64>"` |
| <KW>map of `K` to `V`</KW> | `“map<"` <KW>typeid name</KW> of `K` `“, "` <KW>typeid name</KW> of `V` `">”`<br /><br />e.g. "`map<i64, string>"` |
| Any | `“any”` |

</CenterHorizontally>


### Annotation Maps

An <KW>annotation map</KW> is a (possibly empty) collection of <KW>values</KW> (the <Bookmark id="annotation"><KW>annotations</KW></Bookmark>), with the following properties:
1. all <KW>values</KW> are <KW>structs</KW>
2. no two <KW>values</KW> have the same <KW>type</KW>

The term "annotation map" follows from these properties, as it can be thought of as a collection of <KW>annotations</KW> indexed by their (unique) <KW>type</KW>'s <KW>Thrift URI</KW>.

Annotation maps can be attached to [<KW>user-specified properties</KW>](#user-specified-properties). They typically serve as a means to attach application-level information to <KW>user-defined types</KW> without affecting the outcome of Thrift Object Model [operations](#operations), except for those explicitly related to retrieving <KW>annotations</KW>.

:::note **Implementation Detail** - `RuntimeAnnotation`

Not all annotations specified in Thrift IDL correspond to <KW>Thrift Object Model</KW> <KW>annotations</KW>.

Only annotations whose (`struct`) definition is annotated with `@thrift.RuntimeAnnotation` are considered part of the <KW>user-specified properties</KW> when applied to a <KW>user-defined type</KW>, and therefore part of the [<KW>type system</KW>](#type-system).

Thrift IDL annotations that are not `RuntimeAnnotation`s are meant to only be consumed by the Thrift compiler. For example, the `@cpp.Type` annotations alter the (C++) code generated by the Thrift compiler for a given schema, but is not retained in the <KW>type system</KW>.

This is similar to Java's [`RetentionPolicy.RUNTIME`](https://docs.oracle.com/javase/8/docs/api/java/lang/annotation/RetentionPolicy.html#RUNTIME).
:::

## Records

### Motivation

To ensure that the semantics of the *Thrift Object Model* are not tied to a specific implementation, most defining characteristics of Thrift <KW>types</KW> presented above are, with the notable exception of <KW>typeid names</KW>, abstract by design.
However, in order to reason about and formally define operations on values of these types, a concrete, machine-friendly representation of Thrift <KW>datums</KW> is desirable.

**<KW>Records</KW>** are a definition of such a representation, which capture the structure of **<KW>datums</KW>**, **without association to a Thrift <KW>type system</KW>**.
This structure derives from de-facto standard constructs in the computing ecosystem.
Most (if not all) useful programming languages and computer hardware can efficiently support these structures.

The <Bookmark id="record-datum-association">lack of association between <KW>records</KW> and the Thrift <KW>type system</KW></Bookmark> allows the former to capture a significantly wider range of data, which include some that can never correspond to a valid Thrift <KW>type</KW>.
For example, <KW>records</KW> can capture heterogeneous lists, whereas Thrift lists are always homogeneous.
This is intentional and desirable, as it allows the specifications of operations to clearly distinguish steps that operate on Thrift-compliant values from those that deal with arbitrary, and potentially non-compliant, data (especially during deserialization).

The following propositions, both of which are true, follow from the definition above:
* any valid Thrift <KW>datum</KW> (i.e., any <KW>datum</KW> that conforms to the <KW>schema</KW> of a Thrift <KW>type</KW>) can be represented by a <KW>record</KW>.
* not every <KW>record</KW> corresponds to a valid Thrift <KW>datum</KW>.

Finally, the formal definition of <KW>records</KW> hereafter provides a **consistent human-readable notation** to conveniently and unambiguously define operations on Thrift [<KW>values</KW>](#value).

### Definition

A **<KW>record</KW>** is defined by:
* A <KW>datum</KW>, which [may or may not correspond to a Thrift <KW>datum</KW>](#record-datum-association).
* A **<KW>record kind</KW>**, which limits the set of <KW>datums</KW> that can be held by this <KW>record</KW>.

While the definition of <KW>record kinds</KW> echoes that of <KW>type schemas</KW>, the former are far more concrete, and map almost directly to well-known, ubiquitous computer memory representations.
Consequently, the <KW>record kinds</KW> definitions below reflect the corresponding Thrift <KW>datasets</KW> whenever applicable (e.g., for primitive types), but differ when necessary to provide a concrete representation for abstract Thrift <KW>datums</KW> (e.g. for <KW>structured types</KW>).

:::info <KW>record</KW> = machine and human-readable <KW>datum</KW>
The human-readable representation of each <KW>record kind</KW> is specified below.
Its syntax is heavily inspired by [Python dataclass](https://docs.python.org/3/library/dataclasses.html) initialization expressions, with only keyword or positional arguments (but not both), depending on the <KW>record kind</KW>.

Henceforth, **this specification will use records to represent all <KW>datums</KW>**.
:::

### Record Kinds

#### Bool

A **`Bool`-kind <KW>record</KW>** represents a Thrift [<KW>boolean</KW>](#boolean-type) <KW>datum</KW>, and can be one of:

```python
✅ Bool(True)
✅ Bool(False)
```

#### Int`{N}`

An **`Int{N}`-kind <KW>record</KW>** represents a <KW>datum</KW> from the range of Thrift [fixed-width signed integers](#fixed-width-signed-integer-types) of corresponding width:
* 8 bits — **`Int8`**-kind
* 16 bits — **`Int16`**-kind
* 32 bits — **`Int32`**-kind
* 64 bits — **`Int64`**-kind

Examples:
```python
✅ Int8(55)
✅ Int16(0)
✅ Int16(-42)
✅ Int32(30000)
❌ Int8(255) # 255 is outside the range [-128, 127]
❌ Int64(0.5) # 0.5 is not an integer
```

#### Float`{N}`

A **`Float{N}`-kind <KW>record</KW>** represents a <KW>datum</KW> from the set of Thrift [fixed-width floating-point numbers](#floating-point-number-types) of corresponding precision:
* single precision (i.e. 32 bits) — **`Float32`**-kind
* double precision (i.e. 64 bits) — **`Float64`**-kind

:::note **Reminder**
`Float{N}`-kind <KW>records</KW> exclude `NaN` and signed zeros, [matching the corresponding Thrift <KW>datum</KW>](#nan).
:::

Examples:
```python
✅ Float32(1.5)
✅ Float32(1.0)
✅ Float32(0.0)
✅ Float64(-0.0) # same as Float64(0.0)
✅ Float32(∞)
✅ Float64(-∞)
❌ Float64(NaN)
❌ Float64(0.1) # 0.1 is not representable in IEEE 754 binary32 or binary64
❌ Float64(π)
```

#### Text

A **`Text`-kind <KW>record</KW>** represents a finite sequence of [Unicode code points](https://en.wikipedia.org/wiki/List_of_Unicode_characters), which is the <KW>datum</KW> for the Thrift [<KW>Unicode text type</KW>](#text-type).

Examples:
```python
✅ Text(“hello”)
✅ Text(“👋”)
✅ Text(“”)
```

#### ByteArray

A **`ByteArray`-kind *record*** represents a finite sequence of 8-bit bytes, which is the <KW>datum</KW> for the Thrift [<KW>Byte array type</KW>](#byte-array-type).

Examples:
```python
✅ ByteArray(b“hello”)
✅ ByteArray(b“👋”)
✅ ByteArray(b“”)
✅ ByteArray([1, 2, 3, 4])
✅ ByteArray(b"hello")
✅ ByteArray(b"wo\0rld")
```

#### List

A **`List`-kind <KW>record</KW>** represents a finite, **ordered sequence of <KW>records</KW>**.

Examples:
```python
✅ List(Int32(42), Int32(42))
✅ List() # empty
```

#### Set

A **`Set`-kind <KW>record</KW>** represents a finite, **unordered collection of unique <KW>records</KW>**.

Examples:
```python
✅ Set(Bool(True), Bool(False))
✅ Set() # empty
✅ Set(Set(Text(“hello”)))
❌ Set(Int32(0), Int32(0)) # duplicate element
```

#### Map

A **`Map`-kind <KW>record</KW>** represents a finite, **unordered collection of <KW>record</KW> key-value pairs (`K` → `V`). in which keys (`K`) are unique**.

Examples:
```python
✅ Map(Text("world") → ByteArray(“bar”))
✅ Map() # empty
❌ Map(Int32(0) → Int32(0), Int32(0) → Int32(1)) # duplicate K
```

#### FieldSet

A **`FieldSet`-kind <KW>record</KW>** represents the set of [<KW>field values</KW>](#field-value) for a Thrift [<KW>structured type</KW>](#structured-types).

It is defined as a finite, **unordered collection of ([<KW>field identity</KW>](#field-identity) → *record*) associations**, in which <KW>field identities</KW> are unique.

Examples:
```python
✅ FieldSet((Int16(1), Text(“foo”)) → Int32(-42))
✅ FieldSet() # empty
❌ FieldSet(
     (Int16(1), Text(“foo”)) → Int32(-42),
     (Int16(2), Text(“foo”)) → Int32(0)
   ) # duplicate field name
```

#### Any

An **`Any`-kind <KW>record</KW>** is a “box” that contains a dynamically-typed <KW>value</KW>.
It represents the <KW>datum</KW> for the Thrift [<KW>Any type</KW>](#any-type).

The box may be *empty*.
A non-empty box contains:
* a `Text`-kind record that holds the [<KW>type identifier name</KW>](#typeid-names) for the enclosed value.
* a `ByteArray`-kind record: the opaque descriptor of a [<KW>protocol</KW>](#serialization-protocols) `P`.
* a `ByteArray`-kind record, the [<KW>`P`-cipher</KW>](#cipher).

Examples:
```python
✅ Any() # empty
✅ Any(
     type=Text("i32"),
     protocol=ByteArray(<binary protocol>),
     cipher=ByteArray(...)
   )
✅ Any(
     type=Text("meta.com/foo/Bar"),
     protocol=ByteArray(<custom protocol>),
     cipher=ByteArray(...)
   )
```

#### Equality

Two <KW>records</KW> are considered equal if they have the same <KW>record kind</KW> and represent the same <KW>datum</KW>.
For *record kinds* that contain other <KW>records</KW> (`List`, `Set`, `Map` & `FieldSet`), the contained <KW>records</KW> must also be equal.

Note that <KW>record</KW> equality is not the same as Thrift [<KW>value</KW> equality](#operation-areequal).
Most notably, <KW>Thrift type</KW> information is not considered in <KW>record</KW> equality, so while two <KW>Thrift values</KW> of different <KW>types</KW> can never be equal, their corresponding <KW>datums</KW> (i.e., `FieldSet`-kind <KW>records</KW>) may be equal.

#### Thrift type ↔ Record Kind

The previous section showed how different <KW>record kinds</KW> capture the structure needed to represent every <KW>datum</KW> in Thrift.
For quicker reference, the table below summarizes the correspondence between Thrift <KW>types</KW> and <KW>record kinds</KW>.

<CenterHorizontally>

| Thrift <KW>type</KW> | <KW>Record Kind</KW> |
| ----- | ----- |
| `bool` | **`Bool`** |
| `byte` | **`Int8`** |
| `i16` | **`Int16`** |
| `i32` | **`Int32`** |
| `i64` | **`Int64`** |
| `float` | **`Float32`** |
| `double` | **`Float64`** |
| `string` | **`Text`** |
| `binary` | **`ByteArray`** |
| <KW>struct</KW> | **`FieldSet`** |
| <KW>union</KW> | **`FieldSet`** |
| <KW>enum</KW> | **`Int32`** |
| <KW>opaque alias</KW> | <KW>record kind</KW> of <KW>target type</KW> |
| `list<V>` | **`List`** |
| `set<K>` | **`Set`** |
| `map<K, V>` | **`Map`** |
| `any` | **`Any`** |

</CenterHorizontally>

## Operations

### Inputs and Outcomes

Operations accept two kinds of input:
* an <KW>environment</KW> that captures elements typically provided implicitly in a running environment (such as the <KW>type system</KW> in a running process).
  * Environment parameters are appended to operation names as subscripts, but can be omitted when irrelevant or obvious from the surrounding context.
* explicit *inputs*, similar to formal parameters in most programming languages.

Operations either complete successfully by **producing a result**, or fail. They do not have side-effects.

The semantics of *failed* operations are left implementation-defined.

### Notations

<KW>Notations</KW> are concise, unambiguous symbolic representations of concepts.
This section defines notations to make it convenient to define operations later.

#### Notation: <code>T<sub>S</sub></code> or `T`

Represents the Thrift <KW>type</KW> `T`, that exists in the <KW>type system</KW> `S`.
When the <KW>environment</KW> implies a <KW>type system</KW> `S`, the subscript may be omitted (i.e., `T` instead of <code>T<sub>S</sub></code>).

:::note **Reminder**
Within a <KW>type system</KW> there is a [one-to-one correspondence](#type-typeids-typeid-names) between a <KW>type</KW> and its <KW>typeid name</KW>.
The definitions below use the <KW>typeid name</KW> in lieu of the <KW>type</KW>.
For example, a "`bool` <KW>value</KW>" means a "<KW>value</KW> whose <KW>type</KW> is the Thrift <KW>boolean type</KW>".

In the absence of a <KW>type system</KW>, the distinction between <KW>type</KW> and <KW>typeid name</KW> becomes relevant (and, typically, the absence of a <KW>type system</KW> implies that only the <KW>typeid name</KW> is available).
Examples where <KW>type</KW> information is relevant but a <KW>type system</KW> may not be available include the enclosed <KW>type</KW> of an `any` value.
:::

#### Notation: `Value(T, d)` or `Value(T, r)`

`Value(T, d)` — Represents a Thrift <KW>value</KW> of <KW>type</KW> `T`, with <KW>datum</KW> `d`.
`Value(T, r)` — Represents a Thrift <KW>value</KW> of <KW>type</KW> `T`, with <KW>record</KW> `r` corresponding to some <KW>datum</KW>.

The two notations are interchangeable, provided that the <KW>record</KW> `r` is the canonical <KW>record</KW> representation of a valid <KW>datum</KW> for <KW>type</KW> `T` (see [`record-of(v)`](#notation-record-ofv) below).

#### Notation: `record-of(v)`

The "canonical" <KW>record</KW> representation of the <KW>datum</KW> of <KW>value</KW> `v`.

<!-- We use components here because multiple lines are not allowed in a Markdown table -->

export const RecordOfNotationStructDatumDescription = () => {
  return (
    <>
      Collection of N <KW>field values</KW>: {"{"}
      <br />
      <Indent />
      (<KW><b>field-identity<sub>1</sub></b></KW>) → <KW><b>value<sub>1</sub></b></KW>,
      <br />
      <Indent />
      ...
      <br />
      {"}"}
    </>
  );
};

export const RecordOfNotationStructRecordDescription = () => {
  return (
    <>
      <pre>
        FieldSet(
        <br />
        <Indent />
        (Int16(<KW><b>field-identity<sub>1</sub></b>.id</KW>),
        <br />
        <Indent />&nbsp;
        Text(<KW><b>field-identity<sub>1</sub></b>.name</KW>))
        <br />
        <Indent times={4} />→ record-of(<KW><b>value<sub>1</sub></b></KW>),
        <br />
        <Indent />
        ...
        <br />
        )
      </pre>
    </>
  );
};

export const RecordOfNotationUnionDatumDescription = () => {
  return (
    <>
      Collection of 1 <KW>field values</KW>: {"{"}
      <br />
      <Indent />
      (<KW><b>field-identity<sub>1</sub></b></KW>) → <KW><b>value<sub>1</sub></b></KW>,
      <br />
      {"}"}
    </>
  );
};

export const RecordOfNotationUnionRecordDescription = () => {
  return (
    <>
      <pre>
        FieldSet(
        <br />
        <Indent />
        (Int16(<KW><b>field-identity<sub>1</sub></b>.id</KW>),
        <br />
        <Indent />&nbsp;
        Text(<KW><b>field-identity<sub>1</sub></b>.name</KW>))
        <br />
        <Indent times={4} />→ record-of(<KW><b>value<sub>1</sub></b></KW>),
        <br />
        )
      </pre>
    </>
  );
};

export const RecordOfNotationListDatumDescription = () => {
  return (
    <>
      [ <KW><b>value<sub>1</sub></b></KW>, ..., <KW><b>value<sub>N</sub></b></KW> ]
    </>
  );
};

export const RecordOfNotationListRecordDescription = () => {
  return (
    <pre>
      List(
      <br />
      <Indent />
      record-of(<KW><b>value<sub>1</sub></b></KW>),
      <br />
      <Indent />
      ...,
      <br />
      <Indent />
      record-of(<KW><b>value<sub>N</sub></b></KW>)
      <br />)
    </pre>
  );
};

export const RecordOfNotationSetDatumDescription = () => {
  return (
    <>
      {"{"} <KW><b>value<sub>1</sub></b></KW>, ..., <KW><b>value<sub>N</sub></b></KW> {"}"}
    </>
  );
};

export const RecordOfNotationSetRecordDescription = () => {
  return (
    <pre>
      Set(
      <br />
      <Indent />
      record-of(<KW><b>value<sub>1</sub></b></KW>),
      <br />
      <Indent />
      ...,
      <br />
      <Indent />
      record-of(<KW><b>value<sub>N</sub></b></KW>)
      <br />)
    </pre>
  );
};

export const RecordOfNotationMapDatumDescription = () => {
  return (
    <>
      {"{"} (<KW><b>key<sub>1</sub></b></KW> → <KW><b>value<sub>1</sub></b></KW>),
      <br />
      <Indent />
      ...,
      <br />
      <Indent />
      (<KW><b>key<sub>N</sub></b></KW> → <KW><b>value<sub>N</sub></b></KW>) {"}"}
    </>
  );
};

export const RecordOfNotationMapRecordDescription = () => {
  return (
    <pre>
      Map(
      <br />
      <Indent />
      record-of(<KW><b>key<sub>1</sub></b></KW>) → record-of(<KW><b>value<sub>1</sub></b></KW>),
      <br />
      <Indent />
      ...,
      <br />
      <Indent />
      record-of(<KW><b>key<sub>N</sub></b></KW>) → record-of(<KW><b>value<sub>N</sub></b></KW>)
      <br />)
    </pre>
  );
};

export const RecordOfNotationAnyDatumDescription = () => {
  return (
    <>
      (<KW><b>typeid name</b></KW>, <KW><b>cipher</b></KW>, <b><KW>protocol</KW> descriptor</b>)
    </>
  );
};

export const RecordOfNotationAnyRecordDescription = () => {
  return (
    <pre>
      Any(
      <br />
      <Indent />
      typeid=Text(<KW><b>typeid name</b></KW>),
      <br />
      <Indent />
      protocol=ByteArray(<b><KW>protocol</KW> descriptor</b>),
      <br />
      <Indent />
      cipher=ByteArray(<KW><b>cipher</b></KW>)
      <br />)
    </pre>
  );
};

<CenterHorizontally>

| <KW>type</KW> of `v` | <KW>datum</KW> | `record-of(v)` |
| ----- | ----- | ----- |
| `bool` | <KW><b>true</b></KW> | <code>Bool(<b>True</b>)</code> |
| <CenteredDitto/> | <KW><b>false</b></KW> | <code>Bool(<b>False</b>)</code> |
| `byte` | Integer value ***n*** from the corresponding set of valid [Thrift <KW>fixed-width signed integers</KW>](#fixed-width-signed-integer-types). | <code>Int8(<b><KW>n</KW></b>)</code> |
| `i16` | <CenteredDitto/> | <code>Int16(<b><KW>n</KW></b>)</code> |
| `i32` | <CenteredDitto/> | <code>Int32(<b><KW>n</KW></b>)</code> |
| `i64` | <CenteredDitto/> | <code>Int64(<b><KW>n</KW></b>)</code> |
| `float` | Number ***n*** from the set of valid [Thrift <KW>floating point numbers</KW>](#floating-point-number-types). | <code>Float32(<b><KW>n</KW></b>)</code> |
| `double` | <CenteredDitto/> | <code>Float64(<b><KW>n</KW></b>)</code> |
| `string` | Finite sequence ***t*** of Unicode code points | <code>Text(<b><KW>t</KW></b>)</code> |
| `binary` | Finite sequence ***b*** of bytes | <code>ByteArray(<b><KW>b</KW></b>)</code> |
| <KW>struct</KW> | <RecordOfNotationStructDatumDescription /> | <RecordOfNotationStructRecordDescription /> |
| <KW>union</KW> | *Empty* | `FieldSet()` |
| <CenteredDitto/> | <RecordOfNotationUnionDatumDescription /> | <RecordOfNotationUnionRecordDescription /> |
| <KW>enum</KW> | 32-bit signed integer <KW>enum-value</KW>: ***n*** | <code>Int32(<b><KW>n</KW></b>)</code> |
| <KW>opaque alias</KW> |  | <KW>record kind</KW> of <KW>target type</KW> |
| `list<V>` | <RecordOfNotationListDatumDescription /> | <RecordOfNotationListRecordDescription /> |
| `set<K>` | <RecordOfNotationSetDatumDescription /> | <RecordOfNotationSetRecordDescription /> |
| `map<K, V>` | <RecordOfNotationMapDatumDescription /> | <RecordOfNotationMapRecordDescription /> |
| `any` | *Empty* | `Any()` |
| <CenteredDitto/> | <RecordOfNotationAnyDatumDescription /> | <RecordOfNotationAnyRecordDescription /> |

</CenterHorizontally>

#### Notation: <code>type<sub>S</sub>(v)</code> → `T`

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* `v` — a <KW>value</KW> of <KW>type</KW> `T` that exists in `S`

**Outputs**:
* `T` — the <KW>type</KW> of `v` (typically [represented by its <KW>typeid name</KW>](#typeid-names))

### Value Creation

#### Operation: `createStandardDefault`

<Operation>

> **<code>createStandardDefault<sub>S</sub>(T)</code> → <code>Value(T, ?)</code>**
>
> Creates the “standard” default <KW>value</KW> for a given <KW>type</KW>, with the “intrinsic” default value or the user-provided defaults where applicable.

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* `T` — a <KW>type</KW> within `S`

**Outputs**:
* a <KW>value</KW> of <KW>type</KW> `T`

**Outcome**:
* Produces `Value(T, r)` where...

export const CreateStandardDefaultFieldSetDescription = () => {
  return (
    <>
      <code>FieldSet(fields...)</code> where <code>fields</code> is the set of <KW>field values</KW> where for each field <code>f</code> in <code>T</code>...
      <br />
      <ul>
        <li>
          if <code>f</code> has a <KW>custom default value</KW> <code>d</code>, then <code>fields</code> has the entry
          <ul>
            <li>
              (<KW>field identity</KW> of <code>f</code> → <code>d</code>), else
            </li>
          </ul>
        </li>
        <li>
          if <code>f</code> is <KW>always-present</KW>, then <code>fields</code> has the entry
          <ul>
            <li>
              (<KW>field identity</KW> of <code>f</code> → <code>createStandardDefault<sub>S</sub>(<i>field type</i> of f)</code>)
            </li>
          </ul>
          <li>
            if <code>f</code> is <KW>optional</KW> (without <KW>custom default value</KW>)
            <ul>
              <li>
                <code>fields</code> does not have any entry for <KW>field identity</KW> of <code>f</code>
              </li>
            </ul>
          </li>
          <li>
            <code>fields</code> has no other entries.
          </li>
        </li>
      </ul>
    </>
  );
};

<CenterHorizontally>

| If `T` is... | <KW>Record</KW> (`r`) |
| ----- | ----- |
| `bool` | `Bool(False)` |
| `byte` | `Int8(0)` |
| `i16` | `Int16(0)` |
| `i32` | `Int32(0)` |
| `i64` | `Int64(0)` |
| `float` | `Float32(0)` |
| `double` | `Float64(0)` |
| `string` | `Text(“”)` — *empty* |
| `binary` | `ByteArray(b””)` — *empty* |
| <KW>struct</KW> | <CreateStandardDefaultFieldSetDescription/> |
| <KW>union</KW> | `FieldSet()` — *empty* |
| <KW>enum</KW> | `Int32(0)` |
| <KW>opaque alias</KW> | <code>createStandardDefault<sub>S</sub>(<i>target type</i>)</code> |
| `list<V>` | `List()` — *empty* |
| `set<K>` | `Set()` — *empty* |
| `map<K, V>` | `Map()` — *empty* |
| `any` | `Any()` — *empty* |

</CenterHorizontally>

</Operation>

### Comparison

#### Operation: `areEqual`

<Operation>

> **<code>areEqual<sub>S</sub>(lhs, rhs) → Value(bool, ?)</code>**
>
> Checks whether two Thrift <KW>values</KW> are **<KW>equal</KW>**.

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* <code>lhs = Value(L<sub>S</sub>, l)</code> — a <KW>value</KW> whose <KW>type</KW> <code>L</code> exists in <code>S</code>, with <KW>datum</KW> <code>l</code>.
* <code>rhs = Value(R<sub>S</sub>, r)</code> — a <KW>value</KW> whose <KW>type</KW> <code>R</code> exists in <code>S</code>, with <KW>datum</KW> <code>r</code>.

**Outputs**:
* a boolean <KW>value</KW>

**Outcome**:
* If `L` is not the same as `R`, i.e. the input values do not have the same type, produces `False`.
* if the <KW>type</KW> of `lhs` and `rhs` is not `any`...
  * Produces `True` if `record-of(lhs)` [equals](#equality) `record-of(rhs)`, otherwise `False`.
  * if the <KW>type</KW> of `lhs` and `rhs` is `any`...
  * Produces `True` if `lhs` and `rhs` are both empty.
  * Produces `False` if only one of `lhs` or `rhs` is empty.
  * Produces `False` if `lhs.typeid` and `rhs.typeid` are not equal.
  * Otherwise, given <code>v<sub>lhs</sub></code> = <code>anyUnpack<sub>S</sub>(lhs)</code>, <code>v<sub>rhs</sub></code> = <code>anyUnpack<sub>S</sub>(rhs)</code>...
    * FAILS if the aforementioned [<code>anyUnpack<sub>S</sub></code>](#operation-anyunpack) fails.
      * Otherwise, produces <code>areEqual<sub>S</sub>(v<sub>lhs</sub>, v<sub>rhs</sub>)</code>.
    * Note how comparison of `any` values may succeed even if the unpacking of the underlying value would have failed: for example, if the `typeid`s are different, or only one of the values is not empty, `areEqual` can return `False` even if the current <KW>type system</KW> did not have the corresponding <KW>type</KW> (which would have caused `anyUnpack` to fail).

</Operation>

### Type Erasure

#### Operation: `anyUnpack`

<Operation>

> **<code>anyUnpack<sub>S</sub>(a) → Value(?, ?)</code>**
>
> “Unpacks” the <KW>cipher</KW> stored inside an `Any` into a <KW>value</KW> of the <KW>type</KW> corresponding to the enclosed <KW>typeid</KW> in the given <KW>type system</KW>.

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* `a` — a [non-empty `Any` <KW>value</KW>](#any-type) with components (`typeid`, <KW>protocol</KW> `P`, <code>cipher<sub>P</sub></code>)

**Outputs**:
* <KW>value</KW> — of <KW>type</KW> matching the <KW>typeid</KW> of <code>v</code>

**Outcome**:
* if `typeid` exists in `S`, then produces [`deserialize`](#operation-deserialize) (`P`, `S`, `typeid`, <code>cipher<sub>P</sub></code>).
  * FAILS if the aforementioned `deserialize` fails.
* FAILS otherwise.

</Operation>

## Schema Evolution

Thrift <KW>type systems</KW> are meant to evolve over time — new <KW>user-defined types</KW> may be introduced, and <KW>schemas</KW> of existing <KW>types</KW> may change.
**Enabling safe schema evolution is Thrift's raison d'être.**

### Schema Change

A <KW>schema change</KW> is defined as an action to a <KW>type</KW> <code>T<sub>0</sub></code> to produce a new <KW>type</KW> <code>T<sub>1</sub></code> with the same <KW>type identity</KW> but different <KW>schema</KW>.
It is denoted as <code>T<sub>0</sub></code> → <code>T<sub>1</sub></code>.
The Object Model provides deterministic semantics around certain classes of <KW>schema changes</KW> that can be used to reason about the (application-specific) safety of <KW>schema changes</KW>.

:::info <KW>schema change</KW> ⇒ new <KW>type system</KW>
When a <KW>schema change</KW> <code>T<sub>0</sub></code> → <code>T<sub>1</sub></code> is applied on a <KW>type</KW> <code>T<sub>0</sub></code> from a <KW>type system</KW> <code>S<sub>0</sub></code> to produce <code>T<sub>1</sub></code>, the resultant <KW>type</KW> is not part of the same <KW>type system</KW> (since a <KW>type system</KW> must have unique <KW>type identity</KW> for all <KW>types</KW>).

It follows that **a <KW>schema change</KW> always forms a new <KW>type system</KW>**. Consequently, the schema change <code>T<sub>0</sub></code> → <code>T<sub>1</sub></code> can also be denoted as <code>S<sub>0</sub></code> → <code>S<sub>1</sub></code> .
:::

The possibility of <KW>values</KW> in distinct <KW>type systems</KW>, whose <KW>types</KW> have the same identity but different <KW>schemas</KW> naturally raises the question of their relationship. In particular, producers (writers) and consumers (readers) must be able to exchange <KW>values</KW>, even if their <KW>type systems</KW> differ.

The *Thrift Object Model* enables them to safely do so by specifying the semantics and conditions under which <KW>values</KW> can be mapped from one <KW>type system</KW> to another.

### Value Mapping

The relationship between a given <KW>value</KW> in two distinct <KW>type systems</KW> is defined through a pair of transformations: <KW>projection</KW> and <KW>embedding</KW>.

These transformations use <KW>partial records</KW> as intermediate <KW>value</KW> representations that are not associated with any <KW>type system</KW>.

A <Bookmark id="partial-record"><KW>partial record</KW></Bookmark> is a [<KW>record</KW>](#records) where [`FieldSet`-kind <KW>records</KW>](#fieldset) may have missing or extraneous <KW>field values</KW> when compared to the canonical record representation (i.e., `record-of`) of the same <KW>value</KW>.
Notably, like all <KW>records</KW>, **<KW>partial records</KW>** **are not associated with a <KW>type system</KW>**.

First, a <KW>value</KW> (in a <KW>type system</KW>) is **<KW>projected</KW>** to a <KW>partial record</KW>.
Then, the <KW>partial record</KW> can be **<KW>embedded</KW>** in a (potentially different) <KW>type system</KW>, producing a new <KW>value</KW>.

<!-- https://lucid.app/lucidchart/03314913-6de3-4bf7-892c-b9bc6c58f3bd/edit?viewport_loc=-361%2C-687%2C1768%2C1779%2Crs1_rGd7Sxn~&invitationId=inv_395eb116-6626-4d5b-bb58-fb973bf5c3ca -->
<Figure image={valueMappingImageUrl} caption="Mapping of a Thrift value across type systems" width="100%" />

### Requirements

#### Common Field Preservation

A <KW>common field preserving schema change</KW> (<S0 /> → <S1 />) does not affect the interpretation of <KW>field values</KW> of corresponding <KW>structured types</KW> <T0 /> and <T1 /> for <KW>field identities</KW> that remain in common between them.

<Requirement>

**Given**,

* <S0 /> and <S1 /> — <KW>type systems</KW>
* <T0 /> and <T1 /> — <KW>structured types</KW> in <S /> and <S1 /> respectively with matching <KW>type identity</KW>.
* <V0 /> ∈ <code>dataset(T<sub>0</sub>)</code> and
* <V1 /> = <code><a href="#operation-embed">embed<sub>S1</sub></a>(T<sub>1</sub>, <a href="#operation-project">project<sub>S0</sub></a>(v<sub>0</sub>))</code>

We define a **<KW>schema change</KW> <T0 /> → <T1 /> as <KW>common field preserving</KW>** if,

* For each <KW>field value</KW> <code>f<sub>0</sub></code> and <code>f<sub>1</sub></code> in the intersection of <code>record-of(v<sub>0</sub>)</code> and <code>record-of(v<sub>1</sub>)</code> by <KW>field identity</KW> respectively
  * <code>embed<sub>S1</sub>(<i>type</i> of f<sub>1</sub>, f<sub>0</sub>)</code> succeeds
  * <code><a href="#operation-areequal">areEqual<sub>S1</sub></a>(f<sub>1</sub>, embed<sub>S1</sub>(<i>type</i> of f<sub>1</sub>, f<sub>0</sub>))</code> produces <code>Value(bool, True)</code>

</Requirement>

#### Sealed Types

A Thrift <KW>type</KW> is **<KW>sealed</KW>** if any <KW>schema change</KW> implies that the *project-embed* round-trip should fail. In practice, this means that any schema change to a <KW>sealed type</KW> is considered "backwards-incompatible".

Only <KW>sealed types</KW> may be used as the element of a [<KW>set type</KW>](#set-of-v) or the key of a [<KW>map type</KW>](#map-of-k-to-v).
This restriction guarantees that the *project-embed* round-trip always preserves the cardinality of `Set`-kind and `Map`-kind <KW>records</KW> respectively, should it succeed.

The following table encodes whether a <KW>type</KW> is <KW>sealed</KW> or not:

<CenterHorizontally>

| Type | Sealed? |
| ----- | ----- |
| [<KW>primitive</KW>](#primitive-types), except `any` | Yes |
| [`any`](#any-type) | No (since the contained value may not be <KW>sealed</KW>) |
| [<KW>enum</KW>](#enum-types) | Yes (since unnamed <KW>values</KW> are still part of its <KW>dataset</KW>) |
| [<KW>structured</KW>](#structured-types) | Yes, if [explicitly <KW>sealed</KW> by its definition](#structured-type-sealed) |
| [<KW>opaque alias</KW>](#opaque-alias-types) | Yes, if <KW>target type</KW> is <KW>sealed</KW> |
| [<KW>list of `V`</KW>](#list-of-v) | Yes, if `V` is <KW>sealed</KW> |
| [<KW>set of `V`</KW>](#set-of-v) | Yes (since `V` must be <KW>sealed</KW>) |
| [<KW>map of `K` to `V`</KW>](#map-of-k-to-v) | Yes, if `V` is <KW>sealed</KW> (note that `K` must be <KW>sealed</KW>) |

</CenterHorizontally>

A Thrift [*structured type*](#structured-types) may be <KW>sealed</KW> if and only if the <KW>type</KW> of each of its <KW>fields</KW> are also <KW>sealed</KW>.

:::note **Implementation Detail** — <KW>sealed</KW> in Thrift IDL
As of May 20, 2025... The Thrift IDL does not provide a mechanism to define <KW>sealed types</KW>.
Despite this, it allows (with varying levels of target language support) using (non-<KW>sealed</KW>) <KW>structured types</KW> as <KW>map</KW> keys and <KW>set</KW> elements.
:::

### Operations

#### Operation: `project`

<Operation>

> **<code>project<sub>S</sub>(v)</code> → <KW>partial record</KW>**
>
> Transforms a Thrift <KW>value</KW> into a <KW>partial record</KW>.

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* `v = Value(T, r)` — a <KW>value</KW> whose <KW>type</KW> `L` exists in `S`, with <KW>datum</KW> `r`

**Outputs**:
* a <KW>partial record</KW> — with omissions that are recoverable with schema information in [<code>embed<sub>S</sub></code>](#operation-embed).

**Outcome**:
* If `T` is not a <KW>structured type</KW>, produces `r` unchanged.
* If `T` is a <KW>structured type</KW>, produces a `FieldSet`-kind record `o`, where for each <KW>field</KW> `f` in `T`...
  * If `f` is <KW>always-present</KW>, then `o` contains the corresponding <KW>field value</KW> from `r`.
  * If `f` is <KW>optional</KW>, then `o` contains the corresponding <KW>field value</KW> from `r` if it is present.
    Otherwise, `o` does not contain a <KW>field value</KW> for `f`.
  * If `f` is <KW>terse</KW>, then given `e` = <code>project<sub>S</sub>(<i>field value</i> of f)</code>...
    * If the <KW>type</KW> of `f` is a <KW>structured type</KW>...
      * If `e` is the empty `FieldSet`, then `o` does not contain a <KW>field value</KW> for `f`.
      * Otherwise, `o` contains the <KW>field value</KW> `e` for `f`.
    * Otherwise, `o` contains the corresponding <KW>field value</KW> from `r` iff <code><a href="#operation-areequal">areEqual<sub>S</sub></a>(e, <a href="#operation-createstandarddefault">createStandardDefault<sub>S</sub></a>(<i>type</i> of f))</code> produces `Value(bool, False)`.

</Operation>

#### Operation: `embed`

<Operation>

> **<code>embed<sub>S</sub>(T, r)</code> → <code>Value(T, ?)</code>**
>
> Transforms a <KW>partial record</KW> into a Thrift <KW>value</KW>.

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* `T` — a <KW>type</KW> which exists in `S`
* `r` — a <KW>partial record</KW> of a Thrift <KW>value</KW>.

**Outcome**:

If `T` is...
* `bool`...
  * If `r` is `Bool`-kind, produces `Value(bool, r)`.
    * FAILS otherwise.
  * a [<KW>signed integer type</KW>](#fixed-width-signed-integer-types) (`byte`, `i16`, `i32`, `i64`)...
    * If `r` is a <KW>signed integer kind record</KW> (`Int{N}`) , and <KW>kind</KW> and `T` match (as defined by [this table](#thrift-type--record-kind)), produces `Value(T, r)`.
    * FAILS otherwise.
  * `float`...
    * If `r` is `Float32`-kind, produces `Value(float, r)`.
    * FAILS otherwise.
  * `double`...
    * If `r` is `Float64`-kind, produces `Value(double, r)`.
    * FAILS otherwise.
  * `string`...
    * If `r` is `Text`-kind, produces `Value(string, r)`.
    * FAILS otherwise.
  * `binary`...
    * If `r` is `ByteArray`-kind, produces `Value(binary, r)`.
    * FAILS otherwise.
  * `any`...
    * If `r` is `Any`-kind, produces `Value(any, r)`.
    * FAILS otherwise.
  * `list<V>`...
    * If `r` is `List`-kind, produces `Value(list<V>, result)` where...
      * For each <KW>record</KW> `e` in `r` (in order),
        * `result` has the element <code>embed<sub>S</sub>(V, e)</code>.
      * `result` has no other entries.
      * FAILS if the aforementioned `embed` fails.
    * FAILS otherwise.
  * `set<V>`...
    * If `r` is `Set`-kind, produces `Value(set<V>, result)` where...
      * For each <KW>record</KW> `e` in `r`,
        * `result` has the element <code>embed<sub>S</sub>(V, e)</code>.
      * `result` has no other entries.
      * FAILS if the aforementioned `embed` fails.
    * FAILS otherwise.
  * `map<K, V>`...
    * If `r` is `Map`-kind, produces `Value(map<K, V>, result)` where...
      * `result` is a `Map` where for each element (<code>m<sub>key</sub></code>, <code>m<sub>value</sub></code>) in `r`...
        * `result` contains the entry (<code>result<sub>key</sub></code>, <code>result<sub>value</sub></code>) where,
          * <code>result<sub>key</sub></code> = <code>embed<sub>S</sub>(K, m<sub>key</sub>)</code>
          * <code>result<sub>value</sub></code> = <code>embed<sub>S</sub>(V, m<sub>value</sub>)</code>
          * FAILS if either aforementioned `embed` fails.
      * `result` has no other entries.
    * FAILS otherwise.
  * an [<KW>enum</KW>](#enum-types)...
    * Given `v` = <code>embed<sub>S</sub>(i32, r)</code> succeeds, produces `Value(T, record-of(v))`.
    * FAILS if the aforementioned `embed` fails.
  * an [<KW>opaque alias</KW>](#opaque-alias-types) with <KW>target type</KW> `V`...
    * Given `v` = <code>embed<sub>S</sub>(V, r)</code> succeeds, produces `Value(T, record-of(v))`.
    * FAILS if the aforementioned `embed` fails.
  * a [<KW>struct</KW>](#struct)...
    * If `r` is `FieldSet`-kind, produces `Value(T, result)` where...
      * `result` is a `FieldSet` where for each <KW>field</KW> `f` in `T`...
        * If `r` contains <KW>field identity</KW> of `f`, with <KW>record</KW> `u`,
          * Given `v` = <code>embed<sub>S</sub>(<i>type</i> of f, u)</code>, `result` contains the entry:
            * (<KW>field identity</KW> of `f`, `v`).
          * FAILS if the aforementioned `embed` fails.
        * If `r` does not contain <KW>field identity</KW> of `f`,
          * If `f` is <KW>optional</KW>, then `f` is absent in `result`.
          * If `f` is <KW>always-present</KW>, then `result` contains the entry:
            * (<KW>field identity</KW> of `f`, <code><a href="#operation-createstandarddefault">createStandardDefault<sub>S</sub></a>(<i>type</i> of f)</code>).
      * FAILS if `T` is [<KW>sealed</KW>](#sealed-types) and there are <KW>field values</KW> in `r` that are not in `T`.
      * Otherwise, remaining <KW>field values</KW> in `r` are dropped.
    * FAILS otherwise.
  * a [<KW>union</KW>](#union)...
    * Produces a <KW>value</KW> with the same rules as <KW>struct</KW> except:
      * FAILS if `r` is `FieldSet`-kind and has more than one element.

</Operation>

## Serialization / Deserialization

### Serialization Protocols

[Serialization](https://en.wikipedia.org/wiki/Serialization) is the process that translates structured data into a format, which can be stored or transmitted (e.g. over a network) and reconstructed later.

A Thrift <Bookmark id="protocol-definition-serialize-deserialize">**<KW>protocol</KW>**</Bookmark> is defined by two operations: **`serialize`** and **`deserialize`**.

They define the transformation of a Thrift <KW>value</KW> to and from an opaque [<KW>byte array</KW>](#byte-array-type), denoted as its <Bookmark id="cipher">**<KW>cipher</KW>**</Bookmark>.
The <KW>cipher</KW> produced by a <KW>protocol</KW> `P` is called a <KW>`P`-cipher</KW>.

<!-- https://lucid.app/lucidchart/03314913-6de3-4bf7-892c-b9bc6c58f3bd/edit?viewport_loc=-356%2C-687%2C1768%2C1779%2CE4I.wEjay-se&invitationId=inv_395eb116-6626-4d5b-bb58-fb973bf5c3ca -->
<Figure image={serializeDeserializeImageUrl} caption="Serialization & deserialization of a Thrift value - Overview" width="100%" />

#### Operation: `serialize`

<Operation>

> **<code>serialize<sub>S</sub>(P, v)</code> → <code>cipher<sub>P</sub></code>**
>
> Transforms a Thrift <KW>value</KW> to a <KW>`P`-cipher</KW>.

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* `P` — a <KW>protocol</KW>
* `v` — a <KW>value</KW> of <KW>type</KW> `T,` where `T` is a <KW>type</KW> that exists in `S`

**Outputs**:
* <code>cipher<sub>P</sub></code> — (<KW>byte array</KW>) <KW><code>P</code>-cipher</KW>, which is a <KW>protocol</KW>-specific representation of <code>v</code>.

</Operation>

#### Operation: `deserialize`

<Operation>

> **<code>deserialize<sub>S</sub>(P, T, cipher<sub>P</sub>)</code> → <code>v</code>**
>
> Transforms a <KW>`P`-cipher</KW> into a Thrift <KW>value</KW>.

**Environment**:
* `S` — a <KW>type system</KW>

**Inputs**:
* `P` — a <KW>protocol</KW>
* `T` — a <KW>type</KW> which exists in `S`
* <code>cipher<sub>P</sub></code> — a <KW>byte array</KW> holding the (<KW>protocol</KW>-specific) <KW><code>P</code>-cipher</KW> of a Thrift <KW>value</KW>

**Outcome**:
* FAILS if the input <code>cipher<sub>P</sub></code> is invalid for `T` & `P`
* Otherwise, produces a <KW>value</KW> of <KW>type</KW> `T`, as a result of deserializing <code>cipher<sub>P</sub></code>

</Operation>

### Requirements

#### Value Marshaling Consistency

A conforming <KW>protocol</KW> must produce the same <KW>value</KW> via a <KW>serialize-deserialize</KW> round-trip as an [<KW>project</KW>](#operation-project)-[<KW>embed</KW>](#operation-embed) round-trip across two <KW>type systems</KW> where all <KW>schema changes</KW> between them are [<KW>common field preserving</KW>](#common-field-preservation).

<Requirement>

**Given**,

* <S0 /> and <S1 /> — <KW>type systems</KW>
* <T0 /> and <T1 /> — <KW>types</KW> in <S0 /> and <S1 /> respectively with matching <KW>type identity</KW>.
* For all <V0 /> ∈ <code>dataset(T<sub>0</sub>)</code>

**When**,

* <V1 /> = <code>embed<sub>S1</sub>(T<sub>1</sub>, project<sub>S0</sub>(v<sub>0</sub>))</code> succeeds
* <T0 /> → <T1 /> can be described as a <KW>common field preserving schema change</KW>
  * Note that this is vacuously true for non-<KW>structured types</KW>
* <code>cipher = serialize(P, S<sub>0</sub>, v<sub>0</sub>)</code> succeeds

**Then**,

* <code>r = deserialize(P, S<sub>1</sub>, T<sub>1</sub>, cipher)</code> must succeed, and
* <code><a href="#operation-areequal">areEqual<sub>S1</sub></a>(r, v<sub>1</sub>)</code> must produce <code>Value(bool, True)</code>

</Requirement>

##### Corollary — Round-trip Consistency

Within one <KW>type system</KW> (i.e. <S0 /> = <S1 />), a conforming <KW>protocol</KW> must preserve all information required to reconstruct a <KW>value</KW> via a <KW>serialize</KW>-<KW>deserialize</KW> round-trip.

<Requirement>

**Given**,

* `P` — a <KW>protocol</KW>
* `S` — a <KW>type system</KW>
* <code>v<sub>0</sub></code> — a <KW>value</KW> whose <KW>type</KW> <code>T</code> exists in <code>S</code>

**When**,

* <code>cipher = serialize(P, S, v<sub>0</sub>)</code> succeeds

**Then**,

* <code>v<sub>1</sub> = deserialize(P, S, T, cipher)</code> must succeed, and
* <code><a href="#operation-areequal">areEqual<sub>S</sub></a>(v<sub>0</sub>, v<sub>1</sub>)</code> must produce <code>Value(bool, True)</code>

</Requirement>

#### Cipher and Value Equality

Equality of <KW>ciphers</KW> is a **sufficient condition** for <KW>value</KW> equality.
That is, for a given <KW>protocol</KW> and <KW>type</KW>, **identical** <KW>ciphers</KW> **must** deserialize to the same <KW>value</KW>.

Equality of <KW>ciphers</KW> is **not a required condition** for <KW>value</KW> equality.
That is, for a given <KW>protocol</KW> and <KW>type</KW>, **different** <KW>ciphers</KW> **may** deserialize to the same <KW>value</KW>.

<Requirement>

**Given**,

* `P` — a <KW>protocol</KW>
* `S` — a <KW>type system</KW>
* <V0 /> and <V1 /> — <KW>values</KW> of the same <KW>type</KW> <code>T</code>, that exists in <code>S</code>

**When**,

* <code>cipher<sub>0</sub> = serialize(P, S, v<sub>0</sub>)</code> succeeds, and
* <code>cipher<sub>1</sub> = serialize(P, S, v<sub>1</sub>)</code> succeeds, and

**Then**,

* If:
  * **<code>cipher<sub>0</sub> = cipher<sub>1</sub></code>**
  * Then, the following **must** produce `Value(bool, True)`
    <pre>
      <a href="#operation-areequal">areEqual<sub>S</sub></a>(
      <br />
      <Indent />
        deserialize(P, S, T, cipher<sub>0</sub>),
      <br />
      <Indent />
        deserialize(P, S, T, cipher<sub>1</sub>)
      <br />
      )
    </pre>
* Else, if:
  * **<code>cipher<sub>0</sub> ≠ cipher<sub>1</sub></code>**
  * Then, the following **may** produce `Value(bool, True)`
    <pre>
      <a href="#operation-areequal">areEqual<sub>S</sub></a>(
      <br />
      <Indent />
        deserialize(P, S, T, cipher<sub>0</sub>),
      <br />
      <Indent />
        deserialize(P, S, T, cipher<sub>1</sub>)
      <br />
      )
    </pre>

</Requirement>

### Decomposition

#### Motivation

The requirements above leave significant aspects of the serialization/deserialization behavior undefined, particularly in the face of unexpected or invalid inputs.
Experience has shown that a careful decomposition of these operations into more granular, well-defined steps can result in significantly more predictable behavior.

This section presents such a possible decomposition. While technically not required, in practice all supported <KW>protocol</KW> implementations can be shown to fit in the *Thrift Object Model*, at least conceptually.

#### Overview

The <KW>serialize</KW> operation can be seen as the **composition of two operations: `project` • `encode`**.

The <KW>deserialize</KW> operation can be seen as the **composition of two operations: `decode` • `embed`**.

The decomposed operations allow us to clearly distinguish behavior that is fully defined by the *Thrift Object Model*, and that which is specific to a given <KW>protocol</KW>:
* The [`project`](#operation-project) and [`embed`](#operation-embed) operations are **fully defined in the *Thrift Object Model*** — their inputs, outputs, success and failure conditions apply to all <KW>protocols</KW>.
* The [`encode`](#operation-encode) and [`decode`](#operation-decode) operations capture **<KW>protocol</KW>-specific behavior**: while their expected behavior (i.e., their API) is described below, their details are specific to the corresponding <KW>protocol</KW>.

The <KW>protocol</KW>-specific `decode` operation can itself be seen as the composition of two operations: `decipher` • `materialize`. These sub-operations partition `decode` into <KW>type</KW>-aware and <KW>type</KW>-agnostic components:

* The `decipher` operation **is not <KW>type-system</KW>-aware** — it does not have access to the <KW>schema</KW> of any (user-defined) Thrift <KW>type</KW>.
* The `materialize` operation **is <KW>type-system</KW>-aware**, and can access the full <KW>schema</KW> of a given Thrift <KW>type</KW>, as long as that <KW>type</KW> exists within the <KW>type system</KW>.

With this decomposition, the [previous definition](#protocol-definition-serialize-deserialize) of a Thrift **<KW>protocol</KW>** `P` can be narrowed to:
* (<code>encode<sub>P</sub></code>, <code>decode<sub>S,P</sub></code>), or equivalently
* (<code>encode<sub>P</sub></code>, (<code>decipher<sub>P</sub></code>, <code>materialize<sub>S,P</sub></code>))

<!-- https://lucid.app/lucidchart/03314913-6de3-4bf7-892c-b9bc6c58f3bd/edit?viewport_loc=-307%2C-537%2C1768%2C1779%2CqTI.m6gPhYXj&invitationId=inv_395eb116-6626-4d5b-bb58-fb973bf5c3ca -->
<Figure image={serializeDeserializeDecomposedImageUrl} caption="Serialization & deserialization of a Thrift value" width="100%" />

#### Protocol Intermediate Representation

The <KW>protocol IR</KW> is an abstract intermediate representation that captures the stage of deserialization between the input (opaque) <KW>cipher</KW> and the [<KW>partial record</KW>](#partial-record).

Most notably, the <KW>protocol IR</KW> for a given <KW>cipher</KW> (and <KW>protocol</KW>) is produced without access to the Thrift <KW>type system</KW>, and depending on the <KW>protocol</KW> may need to account for missing or ambiguous data that will be resolved by the subsequent <KW>type-system</KW>-aware operations.
This decomposition enables us to further define and analyze the expected behavior of `decode` under various conditions.
By clearly identifying the steps that require a <KW>type system</KW> from the ones that do not, this approach also clarifies what can be done in a "schemaless" environment, for a given <KW>protocol</KW>.

For example, consider a <KW>protocol</KW> whose <KW>cipher</KW> does not distinguish between different types of numbers (such as JSON).
After [`decipher`](#operation-decipher)-ing (and assuming a valid <KW>cipher</KW> input), such values could be mapped to any of the `Int{N}`, `Float{N}` record-kinds.
The <KW>protocol IR</KW> should capture such information until it can be resolved by [`materialize`](#operation-materialize), which leverages the actual <KW>type</KW> information to produce the appropriate <KW>partial record</KW>.

:::note **Reminder**
The decomposition described in this section, along with any data structure and operation introduced as part of that description, are not mandatory.
In practice, <KW>protocols</KW> may or may not implement (partially or completely) these concepts.
:::

:::note **Implementation Detail** — <KW>protocol-IR</KW>
Binary & Compact <KW>protocols</KW> expose an API in C++ called [<KW>Protocol Object</KW>](/fb/languages/cpp/protocol-object.md) to interact with its <KW>protocol-IR</KW>.

Note that the <KW>protocol-IR</KW> does not necessarily represent a Thrift <KW>value</KW>.
:::

### Serialization Operations

#### Operation: `encode`

:::note **Reminder**
**<code>serialize<sub>S,P</sub> = project<sub>S</sub> • encode<sub>P</sub></code>**
:::

<Operation>

> **<code>encode<sub>P</sub>(r)</code> → <code>Value(binary, ByteArray(<<i>P-cipher</i>>))</code>**
>
> Transforms a [<KW>partial record</KW>](#partial-record) to its corresponding <KW>`P`-cipher</KW>.

**Environment**:
* `P` — a <KW>protocol</KW>

**Inputs**:
* `r` — a <KW>partial record</KW> of a Thrift <KW>value</KW>

**Outputs**:
* a `binary` <KW>value</KW> containing a <KW>`P`-cipher</KW> with the <KW>protocol</KW>-specific opaque representation of `v`

</Operation>

### Deserialization Operations

#### Operation: `decipher`

:::note **Reminder**
**<code>deserialize<sub>S,P</sub> = decode<sub>S,P</sub> • embed<sub>S</sub></code>** where
* **<code>decode<sub>S,P</sub> = decipher<sub>P</sub> • materialize<sub>S,P</sub></code>**
:::

<Operation>

> **<code>decipher<sub>P</sub>(cipher<sub>P</sub>)</code> → <code><i>protocol-IR</i></code>**
>
> Transforms (without access to a Thrift <KW>type system</KW>) a <KW>`P`-cipher</KW> into an abstract, <KW>protocol</KW>-specific intermediate representation suitable as an input for the [`materialize`](#operation-materialize) operation.

**Environment**:
* `P` — a <KW>protocol</KW>

**Inputs**:
* <code>cipher<sub>P</sub></code> — a byte array, the opaque representation of a <KW>value</KW> as a <KW><code>P</code>-cipher</KW>.

**Outputs**:
* <KW>protocol-IR</KW> — a <KW>protocol</KW>-specific, potentially ambiguous, intermediate representation of a Thrift <KW>record</KW>.

</Operation>

#### Operation: `materialize`

<Operation>

> **<code>materialize<sub>S,P</sub>(T, ir)</code> → <KW>partial record</KW>**
>
> Resolve ambiguities in the deciphered <KW>protocol-IR</KW>, based on the now available <KW>type system</KW>.

**Environment**:
* `S` — a <KW>type system</KW>
* `P` — a <KW>protocol</KW>

**Inputs**:
* `T` — a <KW>type</KW> which exists in `S`
* `ir` — the potentially ambiguous <KW>protocol-IR</KW> produced by [<code>decipher<sub>P</sub></code>](#operation-decipher).

**Outputs**:
* <KW>partial record</KW> — a protocol-agnostic, unambiguous (but with potentially omitted fields) <KW>datum</KW> of a Thrift <KW>value</KW>

</Operation>

#### Operation: `decode`

<Operation>

> **<code>decode<sub>S,P</sub>(T, cipher<sub>P</sub>)</code> → <KW>partial record</KW>**
>
> Transforms the opaque, protocol-specific <KW>`P`-cipher</KW> into a <KW>partial record</KW>.

Equivalent to **<code><a href="#operation-materialize">materialize<sub>S,P</sub></a>(T, <a href="#operation-decipher">decipher<sub>P</sub></a>(cipher<sub>P</sub>))</code>**.

</Operation>

## Changelog

| Date released | Version | Description
|---------------|---------|----------------
| May 20, 2025  | 1.0.0   | Initial version
| June 16, 2025 | 1.1.0   | [`MINOR`](#versioning-minor):<ol><li>Added [<KW>Annotation Map</KW>](#annotation-maps) concept, and updated <KW>user-specified properties</KW> to include annotations.</li></ol>[`PATCH`](#versioning-patch): <ol><li>Changed [<KW>presence qualifiers</KW>](#presence-qualifier): renamed <KW>unqualified</KW> to <KW>always-present</KW>.<br />Rationale: the term "unqualified" conflates a Thrift IDL concept (i.e., the lack of a qualifier in the `.thrift` source) with a semantic one in the object model (of a field always having a value). Indeed, the lack of a qualifier in IDL may actually correspond to different *semantic presence qualifiers*: in a <KW>struct</KW>, it corresponds to <KW>always-present</KW>, whereas in a <KW>union</KW> it corresponds to <KW>optional</KW>.</li><li>Various typos and style fixes.</li></ol>

### Versioning

Releases use [Semantic Versioning](https://semver.org/), with a 3-component version number: `MAJOR.MINOR.PATCH`, where:

* <Bookmark id="versioning-major"><code>MAJOR</code></Bookmark> version bumps indicate changes that are *backwards incompatible*.
  * Previously compliant implementations and uses may no longer be compliant.
  * For example, this could be due to new semantics that directly contradict previous versions.
  * Such changes should be *extremely rare*.
* <Bookmark id="versioning-minor"><code>MINOR</code></Bookmark> version bumps indicate changes that *do not contradict previous versions, but may extend them*.
  * Previously compliant implementations and uses remain compliant.
  * Previously undefined behavior may become compliant or non-compliant.
  * Such changes should be relatively common, as the result of an explicit review by the Thrift team of formal change proposals.
* <Bookmark id="versioning-patch"><code>PATCH</code></Bookmark> version bumps *do not impact compliance in any way.*
  * They are typically non-semantic changes to the document, such as examples, clarifications, typographical fixes, etc.
  * All previous assumptions and semantics remain unchanged.
  * Such changes are extremely common.
