---

oncalls: ['hack_lang']

llms-gk: 'hack_team_llm_rules_building'

apply_to_regex: '.*\.(ml|mli|rs)$'

apply_to_content: '(buck|build|hh_single_type_check|ast_defs|typing)'

---

# Hack Building Rules

## Core Commands

✅ **Main build target:**
```bash
buck build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check
```

✅ **Type check single file:**
```bash
buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check -- $THE_FILE
```

✅ **View typed abstract syntax tree:**
```bash
buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check -- --tast $THE_FILE
```

✅ **Run tests:**

You can consult BUCK files and existing tests to find the best test target to run.

Examples:

`buck test @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/test/typecheck:typecheck`

`buck test @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/test/nast/...`

To update snapshots (.exp files):

`buck test @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/test/ --config hack.update=always`

❌ **NEVER run all tests unless the user asks you to**

❌ `buck test @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/test/...`

## Command Restrictions

❌ **NEVER use these (require approval):**
- `buck clean` - Buck's output is trustworthy, if the build is broken then fix it. If you doubt your changes are taking effect, just make a code change and observe the result.
- Bash redirects: `>`, `>>`
- `timeout` command
- Risky globs

## Examples

**✅ GOOD - Building before making changes:**
```bash
# Always build first to verify clean state
buck build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check
# Make your changes
# Build again to verify
buck build @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:hh_single_type_check
```

**❌ BAD - Using prohibited commands:**
```bash
# DON'T DO THIS
buck clean
buck build ... > build.log 2>&1
timeout 30 buck build ...
ocamlc ...
```

## Development Workflow

1. **Build frequently** - OCaml IDE diagnostics are often wrong, trust builds
2. You can make one-off tests and run them with `buck run`
3. **Commit working states** - Use succinct commit messages
4. **Use specific targets** - Find in BUCK files

## Success Pattern

✅ Build succeeds
✅ No unintended behavior changes
✅ Tests pass (with updated snapshots if needed)
