---
oncalls: ['hack_lang']
llms-gk: 'hack_team_llm_rules_parser_codegen'
apply_to_regex: '.*\.(ml|rs)$'
apply_to_content: '\b(@generated|schema_definition|token_schema|full_fidelity)\b'
---

# Hack Parser Code Generation Rules

## When to Regenerate

✅ **Run generation when you change:**
- `schema/schema_definition.ml` - Core syntax tree structure
- `schema/token_schema_definition.ml` - Token definitions
- Parser generation templates (rare)

## Essential Command

✅ **CRITICAL - Use exactly this:**
```bash
cd ~/fbsource/fbcode && buck run //hphp/hack/src:generate_full_fidelity
```

❌ **DO NOT use alternatives** mentioned in @generated files

## Test Generated Parser

✅ **Basic parser test:**
```bash
buck2 run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_parse -- ~/test.php
```

✅ **Show parse tree + errors:**
```bash
buck2 run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_parse -- --full-fidelity-json-parse-tree --full-fidelity-errors ~/test.php
```

✅ **Debug AST output:**
```bash
buck2 run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_parse -- --full-fidelity-ast-s-expression ~/test.php
```

## What Gets Generated

**Schema → Generated Files:**

**OCaml files:**
- `full_fidelity_syntax.ml` - Core syntax tree
- `full_fidelity_token_kind.ml` - Token definitions
- `smart_constructors/*.ml` - Constructor interfaces

**Rust files:**
- `syntax_type.rs` - Core syntax types
- `token_kind.rs` - Token definitions
- `smart_constructors_generated.rs` - Constructor implementations
- `syntax_by_ref/*.rs` - Reference implementations

## Examples

**✅ GOOD - Complete generation workflow:**
```bash
# 1. Edit schema files
# 2. Generate immediately
cd ~/fbsource/fbcode && buck run //hphp/hack/src:generate_full_fidelity
# 3. Build to verify
buck2 build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check
# 4. Test parsing
buck2 run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_parse -- --full-fidelity-errors ~/test.php
```

**✅ GOOD - Adding new syntax node:**
```ocaml
(* File: schema/schema_definition.ml *)
let schema = [
  (* ... existing nodes ... *)
  {
    kind_name = "MyNewStatement";
    type_name = "my_new_statement";
    description = "A new statement type";
    prefix = "my_new";
    aggregates = [Statement];
    fields = [
      Token;
      (ZeroOrOne (Aggregate Expression));
      Token;
    ]
  };
  (* ... *)
]
```

**✅ GOOD - Adding new token:**
```ocaml
(* File: schema/token_schema_definition.ml *)
let given_text_tokens = [
  (* ... existing tokens ... *)
  make_token_node "MyKeyword" "mykeyword" ~allowed_as_identifier:false ();
  (* ... *)
]
```

**❌ BAD - Editing generated files:**
```rust
// DON'T DO THIS - edit schema instead
// File: token_kind.rs
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TokenKind {
    // NEVER edit this directly
}
```

## Schema Structure

**Key aggregate types:**
- `TopLevelDeclaration` - Classes, functions
- `Expression` - Variables, calls, operators
- `Statement` - Control flow, assignments
- `Specifier` - Type specifications

**Field specifications:**
- `Token` - Single token (most common)
- `Just "NodeType"` - Specific node reference
- `Aggregate Type` - Any node of aggregate type
- `ZeroOrMore` / `ZeroOrOne` - Repetition patterns

## Common Issues

**Generation fails?**
1. OCaml syntax error in schema → Fix schema syntax
2. Duplicate node names → Check for conflicts
3. Invalid field types → Review field specifications

**Build fails after generation?**
1. New AST nodes → May need `lowerer.rs` updates
2. Parser integration → Check parsing pipeline
3. Missing implementations → Review generated vs hand-written code

## Quick Reference

- **Edit**: Schema files only (`schema/`)
- **Generate**: Run immediately after schema changes
- **Never edit**: `@generated` files
- **Test**: Use `hh_parse` for parser verification
- **Build**: Verify with `hh_single_type_check` target
