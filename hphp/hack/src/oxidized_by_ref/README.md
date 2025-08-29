# Oxidized By Ref - Complete Guide

This document provides comprehensive information about the `oxidized_by_ref`
crate and the Hack oxidation system for generating arena-allocated Rust types.

## Overview

The `oxidized_by_ref` crate contains Rust type definitions generated from OCaml
source code using the `hh_oxidize` tool. Unlike the regular `oxidized` crate
which uses owned data (`Box`, `Vec`, `String`), this crate uses references and
arena allocation for better performance in the Hack type checker.

## Key Differences: ByRef vs ByBox Mode

| Aspect                  | ByBox Mode (`oxidized`) | ByRef Mode (`oxidized_by_ref`)           |
| ----------------------- | ----------------------- | ---------------------------------------- |
| **Recursive Types**     | `Box<T>`                | `&'a T`                                  |
| **Collections**         | `Vec<T>`                | `&'a [T]`                                |
| **Strings**             | `String`                | `&'a str`                                |
| **Memory Management**   | Heap allocation         | Arena allocation                         |
| **Lifetime Parameters** | None                    | `<'a>` where needed                      |
| **Performance**         | More allocations        | Fewer allocations, better cache locality |

## Configuration Files

The oxidation process is controlled by three main configuration files:

### 1. `copy_types.txt` (290 types)

**Purpose**: Types that implement `Copy` and should **not be put behind
references** in ByRef mode.

**Effect**: Instead of `&'a MyType`, use `MyType` directly.

**Criteria for inclusion**:

- ✅ Type implements `Copy` trait
- ✅ Small size (typically `#[repr(u8)]` or simple structs)
- ✅ No benefit from being behind a reference

**Example**:

```rust
// Without copy_types entry: &'a Tprim
// With copy_types entry: Tprim (direct copy)
pub enum Tprim {
    Tnull, Tvoid, Tint, Tbool, // Simple, copyable
}
```

### 2. `owned_types.txt` (58 types)

**Purpose**: Types that **do not need lifetime parameters** in ByRef mode.

**Effect**: Generate `MyType` instead of `MyType<'a>`.

**Criteria for inclusion**:

- ✅ No lifetime parameters in the type definition
- ✅ No reference fields (`&'a T`)
- ✅ Simple, self-contained data
- ✅ Implements `TrivialDrop`

**Example**:

```rust
// Without owned_types entry: MyEnum<'a>
// With owned_types entry: MyEnum (no lifetime parameter)
pub enum MyEnum {
    Variant1,
    Variant2,
}
```

### 3. `extern_types.txt` (131 types)

**Purpose**: Types defined elsewhere that should be **re-exported instead of
generated**.

**Effect**: Generate `pub use other_crate::Type;` instead of defining the type.

**Format**: Each line must be fully qualified: `oxidized::module::TypeName`

**Mapping Logic**: Automatically strips crate prefix:

- `oxidized::ast_defs::Tprim` → maps `ast_defs::Tprim` to
  `oxidized::ast_defs::Tprim`

**Criteria for inclusion**:

- ✅ Type already exists in another crate (usually `oxidized`)
- ✅ Doesn't need lifetime parameters
- ✅ Simple enough to be shared between variants
- ✅ No arena allocation features needed

**Example**:

```rust
// Instead of generating a new type definition:
// Emits: pub use oxidized::ast_defs::Tprim;
```

## How Lifetime Parameters Are Determined

The lifetime parameter assignment follows this decision tree in
`convert_type_decl.ml`:

```ocaml
let lifetime =
  match Configuration.mode () with
  | Configuration.ByRef ->
    if Configuration.owned_type name then
      []  (* No lifetime - type is in owned_types.txt *)
    else
      [Rust_type.lifetime "a"]  (* Add 'a lifetime *)
  | Configuration.ByBox -> []  (* ByBox mode never uses lifetimes *)
```

### Special Cases

1. **Nullary Variant Types**: Lifetimes removed to prevent unused parameters

   ```ocaml
   let lifetime =
     if all_nullary then [] else lifetime
   ```

2. **Arena Deserialization**: All lifetimes mapped to "arena" for
   deserialization macros
   ```ocaml
   let lts = List.map lifetime ~f:(fun _ -> Rust_type.lifetime "arena") in
   ```

## Type Safety Guidelines

### ✅ Safe for `copy_types.txt`

Check these criteria:

1. **Has `Copy` trait**: `#[derive(Copy, ...)]`
2. **Small size**: `#[repr(u8)]` or similar
3. **No references**: No `&'a T` fields
4. **Simple structure**: Basic enums or small structs

```rust
#[derive(Copy, Clone, Debug)]  // ✅ Has Copy
#[repr(u8)]                   // ✅ Small
pub enum SafeType {
    Variant1,                 // ✅ No fields
    Variant2,
}
```

### ✅ Safe for `owned_types.txt`

Check these criteria:

1. **No lifetime parameters**: Type definition has no `<'a>`
2. **No reference fields**: No `&'a T` anywhere
3. **Simple data**: Self-contained, no complex relationships
4. **TrivialDrop**: `impl TrivialDrop for Type {}`

```rust
#[derive(Clone, Debug)]
pub enum SafeOwnedType {      // ✅ No <'a> parameter
    Simple,                   // ✅ No reference fields
    WithData(u32),           // ✅ Owned data only
}
impl TrivialDrop for SafeOwnedType {}  // ✅ TrivialDrop
```

### ✅ Safe for `extern_types.txt`

Check these criteria:

1. **Exists externally**: Already defined in another crate
2. **No lifetime needs**: Doesn't require arena allocation
3. **Stable interface**: Won't change between variants
4. **Fully qualified**: Format `crate::module::Type`

### ❌ Unsafe Examples

```rust
// ❌ NOT safe for owned_types - has lifetime parameter
pub struct UnsafeType<'a> {
    field: &'a SomeOtherType<'a>,  // ❌ Contains references
}

// ❌ NOT safe for copy_types - no Copy trait
#[derive(Clone, Debug)]  // ❌ Missing Copy
pub struct LargeStruct {
    many_fields: [u64; 100],  // ❌ Too large
}

// ❌ NOT safe for extern_types - needs arena features
pub enum ComplexType<'a> {
    WithRef(&'a OtherType<'a>),  // ❌ Has lifetime dependencies
}
```

## File Analysis: Type Distribution

Based on the current configuration:

- **Total copy types**: 290
- **Total owned types**: 58
- **Total extern types**: 131
- **Copy types NOT in owned/extern**: 152

This shows that many types can be efficiently copied but still need lifetime
management for arena allocation consistency.

## Common Patterns

### 1. Simple Enums (Safe for all categories)

```rust
#[derive(Copy, Clone, Debug)]
#[repr(u8)]
pub enum SimpleEnum {
    Option1,
    Option2,
}
```

### 2. Complex AST Types (Copy but not owned)

```rust
#[derive(Copy, Clone)]  // In copy_types.txt
pub enum Expr_<'a> {    // NOT in owned_types.txt (has lifetime)
    Literal(Value),
    Ref(&'a Expr<'a>),  // Contains references
}
```

### 3. External Re-exports

```rust
// In extern_types.txt: oxidized::ast_defs::Bop
pub use oxidized::ast_defs::Bop;  // Re-exported, not generated
```

## Development Workflow

### Adding a Type to Configuration Files

1. **Identify the type's characteristics**:
   - Does it implement `Copy`?
   - Does it have lifetime parameters?
   - Is it defined elsewhere?

2. **Check safety criteria** (see guidelines above)

3. **Add to appropriate file(s)**:
   - Most types go in `copy_types.txt` if they're copyable
   - Only add to `owned_types.txt` if absolutely certain no lifetimes needed
   - Only add to `extern_types.txt` if it exists elsewhere and is simple

4. **Test the changes**:
   ```bash
   buck build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen
   ```

### Debugging Configuration Issues

**Problem**: Type has unnecessary lifetime parameters

- **Solution**: Add to `owned_types.txt` if safe

**Problem**: Type causes compilation errors about references

- **Solution**: Remove from `owned_types.txt`, ensure it's in `copy_types.txt`

**Problem**: Duplicate type definitions

- **Solution**: Add to `extern_types.txt` if it exists elsewhere

## Generated Code Structure

```
oxidized_by_ref/
├── gen/                    # Generated Rust files
│   ├── aast_defs.rs       # AST definitions with lifetimes
│   ├── typing_defs.rs     # Type system definitions
│   └── ...
├── copy_types.txt         # Types that implement Copy
├── owned_types.txt        # Types without lifetime parameters
├── extern_types.txt       # Types re-exported from other crates
└── README.md             # This file
```

## Command Line Usage

The oxidation is controlled via command-line arguments to `hh_oxidize`:

```bash
hh_oxidize \
  --by-ref \
  --extern-types-file extern_types.txt \
  --owned-types-file owned_types.txt \
  --copy-types-file copy_types.txt \
  --out-dir gen/ \
  source_file.ml
```

## Key Takeaways

1. **`Copy` ≠ `Owned`**: A type can be copyable but still need lifetime
   management
2. **Conservative approach**: Only add to `owned_types.txt` when absolutely
   certain
3. **Arena consistency**: Even simple types may need lifetimes for arena
   allocation
4. **External reuse**: Use `extern_types.txt` to avoid duplication with
   `oxidized` crate
5. **Performance optimization**: `copy_types.txt` prevents unnecessary
   indirection for small types

## Further Reading

- `hh_oxidize/convert_type_decl.ml` - Core type conversion logic
- `hh_oxidize/configuration.ml` - Configuration handling
- `oxidized/` - The ByBox variant for comparison
- Arena allocation documentation for lifetime management concepts
