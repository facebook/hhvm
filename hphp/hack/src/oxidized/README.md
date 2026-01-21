# Oxidized - Complete Guide

This document provides comprehensive information about the `oxidized` crate and
the Hack oxidation system for generating owned Rust types from OCaml source
code.

## Overview

The `oxidized` crate contains Rust type definitions generated from OCaml source
code using the `hh_oxidize` tool. This crate uses owned data structures (`Box`,
`Vec`, `String`) for heap allocation, making it suitable for contexts where data
needs to be owned and persisted.

## Summary

| Aspect                  | Description                         |
| ----------------------- | ----------------------------------- |
| **Recursive Types**     | `Box<T>`                            |
| **Collections**         | `Vec<T>`                            |
| **Strings**             | `String`                            |
| **Memory Management**   | Heap allocation                     |
| **Lifetime Parameters** | None                                |
| **Memory Overhead**     | High (individual allocations)       |
| **Use Cases**           | Data persistence, serialization     |
| **Configuration Files** | 2 files (copy_types, safe_ints)     |
| **Generated Structure** | Rich (manual/, visitor/, impl_gen/) |

## Configuration Files

The oxidation process is controlled by two configuration files:

### 1. `copy_types.txt` (138 types)

**Purpose**: Types that implement `Copy` and should be passed by value rather
than moved.

**Effect**: Enables direct copying instead of expensive moves for small types.

**Criteria for inclusion**:

- ✅ Type implements `Copy` trait
- ✅ Small size (typically `#[repr(u8)]` or simple structs)
- ✅ Frequent usage patterns benefit from copying
- ✅ No large embedded data structures

**Example types**:

```rust
// Simple enums - perfect for copying
pub enum Visibility {
    Private, Public, Protected, Internal
}

// Primitive-like types
pub enum Tprim {
    Tnull, Tvoid, Tint, Tbool, Tfloat
}
```

### 2. `safe_ints_types.txt` (1 type)

**Purpose**: Types where OCaml `int` values should be converted to
`ocamlrep::OCamlInt` instead of `isize`.

**Single Entry**: `warnings_saved_state::ErrorHash`

**Use Case**: When you need to preserve the exact OCaml integer representation
rather than converting to Rust's native integer types. This is important for:

- Hash values that must remain stable
- Values that interface with OCaml code expecting specific integer formats
- Serialization compatibility

## Directory Structure

```
oxidized/
├── gen/                     # Generated Rust files (basic type definitions)
│   ├── aast_defs.rs        # AST definitions without lifetimes
│   ├── typing_defs.rs      # Type system definitions
│   └── ...                 # 50+ generated modules
├── manual/                  # Hand-written implementations
│   ├── aast_defs_impl.rs   # Custom methods for AST types
│   ├── errors_impl.rs      # Error handling implementations
│   ├── s_map.rs            # String map implementations
│   └── ...                 # 40+ manual implementations
├── impl_gen/               # Generated trait implementations
│   ├── aast_defs_impl_gen.rs
│   └── ast_defs_impl_gen.rs
├── aast_visitor/           # Visitor pattern for AST traversal
│   ├── visitor.rs          # Generic visitor trait
│   ├── visitor_mut.rs      # Mutable visitor trait
│   └── node.rs             # Node trait for visitable types
├── asts/                   # AST type aliases and utilities
│   ├── ast.rs              # Raw AST types
│   └── nast.rs             # Named AST types
├── stubs/                  # Stub implementations for external deps
├── copy_types.txt          # Copy optimization configuration
├── safe_ints_types.txt     # OCamlInt preservation configuration
├── lib.rs                  # Crate root and re-exports
└── Cargo.toml             # Rust package metadata
```

## Architecture Components

### 1. Generated Code (`gen/`)

**Purpose**: Basic type definitions generated from OCaml source.

**Characteristics**:

- No lifetime parameters (owned data only)
- Standard derive traits (`Clone`, `Debug`, `Serialize`, etc.)
- `FromOcamlRep` for OCaml interop
- `Box<T>` for recursive types

**Example**:

```rust
// From gen/ast_defs.rs
#[derive(Clone, Debug, Deserialize, FromOcamlRep, Serialize)]
pub struct Id(pub Pos, pub String);

pub enum Hint_ {
    Hoption(Box<Hint>),           // Box for recursion
    Htuple(Vec<Hint>),            // Vec for collections
    Happly(Sid, Vec<Hint>),       // Owned strings and vectors
}
```

### 2. Manual Implementations (`manual/`)

**Purpose**: Hand-written functionality that can't be automatically generated.

**Provides**:

- Convenience methods and constructors
- Custom trait implementations
- Domain-specific logic
- Performance optimizations

**Example from `manual/aast_defs_impl.rs`**:

```rust
impl Lid {
    pub fn new(p: Pos, s: String) -> Self {
        Self(p, (0, s))
    }

    pub fn name(&self) -> &String {
        crate::local_id::get_name(&self.1)
    }
}

impl AsRef<str> for Visibility {
    fn as_ref(&self) -> &str {
        match self {
            Visibility::Private => "private",
            Visibility::Public => "public",
            // ...
        }
    }
}
```

### 3. Generated Implementations (`impl_gen/`)

**Purpose**: Automatically generated trait implementations for complex types.

**Contains**: Generated implementations that are too complex for the main
generation pass but can be automated.

### 4. AST Visitor Pattern (`aast_visitor/`)

**Purpose**: Provides a visitor pattern for traversing and transforming AST
nodes.

**Key Components**:

- `Visitor` trait for immutable traversal
- `VisitorMut` trait for mutable transformations
- `Node` trait implemented by all visitable types
- Type-safe traversal with error handling

**Example Usage**:

```rust
// Traverse an AST and collect function names
struct FunctionNameCollector {
    names: Vec<String>,
}

impl<'node> Visitor<'node> for FunctionNameCollector {
    type Params = MyParams;

    fn visit_fun_def(&mut self, ctx: &mut Context, fun: &FunDef) -> Result<(), Error> {
        self.names.push(fun.name.1.clone());
        fun.recurse(ctx, self) // Continue traversal
    }
}
```

### 5. AST Type Aliases (`asts/`)

**Purpose**: Convenient type aliases for different AST phases.

- `ast.rs`: Raw AST types (immediately after parsing)
- `nast.rs`: Named AST types (after naming resolution)

**Example**:

```rust
// Convenient aliases for common AST instantiations
pub type Expr = aast_defs::Expr<(), ()>;
pub type Stmt = aast_defs::Stmt<(), ()>;
pub type Program = aast_defs::Program<(), ()>;
```

## Type Safety Guidelines

### ✅ Safe for `copy_types.txt`

Check these criteria:

1. **Has `Copy` trait**: `#[derive(Copy, ...)]`
2. **Small size**: `#[repr(u8)]`, simple enums, or small structs
3. **Frequent usage**: Used in hot code paths where copying is beneficial
4. **No large data**: No embedded `Vec`, `String`, or `Box` fields

```rust
#[derive(Copy, Clone, Debug)]  // ✅ Has Copy
#[repr(u8)]                   // ✅ Small size
pub enum SafeType {
    Simple,                   // ✅ No data
    WithSmallData(u32),      // ✅ Small primitive
}
```

### ✅ Safe for `safe_ints_types.txt`

Check these criteria:

1. **OCaml compatibility**: Type interfaces with OCaml code
2. **Hash stability**: Values used as keys or hashes that must be stable
3. **Serialization needs**: Must preserve exact OCaml integer representation

### ❌ Unsafe Examples

```rust
// ❌ NOT safe for copy_types - too large
pub struct LargeStruct {
    data: [u64; 1000],        // ❌ Too much data to copy
    name: String,             // ❌ Heap allocated data
}

// ❌ NOT safe for copy_types - no Copy trait
#[derive(Clone, Debug)]       // ❌ Missing Copy
pub struct NoCopyType {
    value: u32,
}
```

## Development Workflow

### Adding New Types

1. **Modify OCaml source**: Add or modify OCaml type definitions
2. **Regenerate**: Run oxidation process
   ```bash
   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen
   ```
3. **Add manual implementations**: If needed, add to `manual/` directory
4. **Update configuration**: Add to `copy_types.txt` if appropriate
5. **Test**: Verify compilation and behavior

### Working with Visitor Pattern

```rust
// Implement a visitor to analyze AST
struct TypeAnalyzer {
    type_count: HashMap<String, usize>,
}

impl<'node> Visitor<'node> for TypeAnalyzer {
    type Params = AnalysisParams;

    fn visit_hint(&mut self, ctx: &mut Context, hint: &Hint) -> Result<(), Error> {
        // Analyze type hints
        if let Hint_::Happly(name, _) = &hint.1.as_ref() {
            *self.type_count.entry(name.1.clone()).or_insert(0) += 1;
        }
        hint.recurse(ctx, self)
    }
}
```

### Manual Implementation Patterns

```rust
// Common patterns in manual/ files

// 1. Constructor convenience methods
impl MyType {
    pub fn new(field1: Type1, field2: Type2) -> Self {
        Self { field1, field2 }
    }
}

// 2. Conversion traits
impl AsRef<str> for MyEnum {
    fn as_ref(&self) -> &str {
        match self {
            MyEnum::Variant1 => "variant1",
            MyEnum::Variant2 => "variant2",
        }
    }
}

// 3. Display implementations
impl Display for MyType {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.as_ref())
    }
}
```

## Command Line Usage

The oxidation is controlled via command-line arguments to `hh_oxidize`:

```bash
# Generate in ByBox mode (default)
hh_oxidize \
  --copy-types-file copy_types.txt \
  --safe-ints-types-file safe_ints_types.txt \
  --out-dir gen/ \
  source_file.ml

# Regenerate all oxidized files
buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen
```

## Integration with Hack Type Checker

The `oxidized` crate serves as the foundation for:

1. **Serialization**: Converting OCaml types to/from various formats
2. **Inter-process communication**: Passing data between Hack processes
3. **Persistent storage**: Saving type information and analysis results
4. **External tools**: Providing Rust-native access to Hack AST and type data

## Performance Considerations

### Memory Usage

- **Pros**: No lifetime management complexity
- **Cons**: Higher memory overhead due to individual allocations
- **Best for**: Long-lived data, serialization, cross-process communication

### Compilation Speed

- **Pros**: Simpler generated code, faster compilation
- **Cons**: Larger binary size due to manual implementations
- **Best for**: Applications where runtime performance matters more than compile
  time

## Key Takeaways

1. **Owned vs Borrowed**: Use `oxidized` when you need to own and persist data
2. **Rich functionality**: Extensive manual implementations provide
   domain-specific methods
3. **Visitor pattern**: Powerful AST traversal and transformation capabilities
4. **OCaml compatibility**: Seamless interop with OCaml codebase through
   `ocamlrep`
5. **Modular design**: Clear separation between generated and manual code
6. **Type safety**: Comprehensive derive traits ensure safe serialization and
   cloning

## Further Reading

- `hh_oxidize/` - Core oxidation tool implementation
- `manual/` directory - Examples of hand-written implementations
- `aast_visitor/` - Visitor pattern documentation and examples
- OCamlRep documentation for interoperability details
