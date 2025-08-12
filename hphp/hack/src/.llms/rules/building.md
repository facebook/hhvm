---

oncalls: ['hack_lang']

llms-gk: 'hack_team_llm_rules_building'

apply_to_regex: '.*\.(ml|mli|rs)$'

apply_to_content: '(buck|build|hh_single_type_check|ast_defs|typing)'

---

# Hack Building Rules

## Core Commands

✅ **Primary build:**
```bash
buck2 build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check
```

✅ **Type check single file:**
```bash
buck2 run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check -- ~/test.php
```

✅ **Run tests:**
```bash
buck2 test @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/test/... --config hack.update=always
```

## Command Restrictions

❌ **NEVER use these (require approval):**
- `buck clean` - Usually broken builds need regeneration, not cleaning
- Bash redirects: `>`, `>>`, `|`
- `timeout` command
- Risky globs

## Examples

**✅ GOOD - Building before making changes:**
```bash
# Always build first to verify clean state
buck2 build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check
# Make your changes
# Build again to verify
buck2 build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check
```

**❌ BAD - Using prohibited commands:**
```bash
# DON'T DO THIS
buck clean
buck2 build ... > build.log 2>&1
timeout 30 buck2 build ...
```

## Development Workflow

1. **Build frequently** - OCaml IDE diagnostics often wrong, trust builds
2. **Test immediately after changes** - Use snapshot updates: `--config hack.update=always`
3. **Commit working states** - Use succinct commit messages
4. **Use specific targets** - Find in BUCK files for faster builds

## Success Pattern

✅ Build succeeds
✅ No unintended behavior changes
✅ Tests pass (with updated snapshots if needed)

## Quick Debugging

**Build fails?** Check:
1. Did you modify AST definitions? → Run oxidation
2. Did you modify parser schema? → Run generation
3. Did you change types used across OCaml/Rust boundary? → Run oxidation
