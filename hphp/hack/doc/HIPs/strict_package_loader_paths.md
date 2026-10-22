# HIP: Loader paths for strict packages

**Status:** Draft

**Start date:** 2026-10-07

**Author:** David Snider

## Summary

Strict packages (today the `prod_atoms`, `intern_atoms`, `prod_projects` and
`intern_projects` implicit families) forbid outside code from using their
classes. The typechecker enforces this for static references. HHVM enforces it
for string-to-class conversions, but by target alone: it rejects every
conversion into a strict package, whoever performs it. Core frameworks (Alite
routing, GraphQL, Ent, MetaCLI, the test runner, sandbox caches) legitimately
load atom classes by name, so enabling enforcement broke them, and it is
currently off.

This proposal adds one global list, in `PACKAGES.toml`, of the source paths
whose code may load strict-package classes by name:

```toml
[strict_isolation]
loader_paths = ["//flib/graphql/core/", "//flib/entity/core/", ...]
```

HHVM allows a conversion when the function performing it is in the target
package or under a loader path. The proposal:

- replaces `HH\classname_to_class_strict_isolation_backdoor`, its wrapper and
  the per-file lint ledger;
- routes every by-name class resolution, including reflection, unserialize and
  Facts, through the same check;
- adds no cost to loader code: the grant is one bit per unit, so the JIT drops
  the check there;
- puts loader paths under mandatory steward review, since code there holds the
  grant;
- needs no new syntax and leaves modules and `__PackageOverride` unconstrained.

The one requirement on framework code is that it convert classnames itself
rather than through shared wrappers.

## Feature motivation

### Current enforcement

- **Static references** from outside a strict package are a typechecker error,
  `Typing[4472]`, covering type positions, attributes, `::class`, `nameof` and
  `catch`. 4472 is not in `allowed_fixme_codes_strict`, so it cannot be
  suppressed.
- **Dynamic references** are checked by HHVM in
  `checkStrictPackageDynamicReference`, which looks only at the target class.
  A conversion into a strict package throws, or emits a sampled notice when the
  package sets `raise_dynamic_class_load_error` to false. The caller is never
  consulted, so even a package loading its own classes fails.
  - Checked: `new $s`, `$s::m()`, `$s::C`, `$s::$p`, `HH\classname_to_class`,
    `HH\dynamic_class_meth(_force)` and the class-taking `type_structure*`
    APIs.
  - Exempt: class pointers, lazy classes (`Foo::class`) and objects.
  - Repo builds run the check only when the active deployment contains a
    strict package. `prod` and `intern` do, through the projects families.
- Caller awareness was deferred by design. The runtime design doc says: "No
  caller-package identity or package credential is added to HHBC or HHBBC in
  this phase."

### What broke

Enabling errors for `atoms`, then a single explicit package, broke:

- **Sandbox cache renders**: the XController URI map, GraphQL SDL and hash
  caches, EntSchema descriptions and MetaCLI command corpora
  ([post](https://fb.workplace.com/groups/410149103633454/permalink/1790003355648015/)).
- **Saved-state writes**
  ([S713199](https://www.internalfb.com/sevmanager/view/713199)): a PrivacyLib
  artifact under `//atom/` was loaded with `HH\classname_to_class`
  (D122307557).

D122857327 then set `raise_dynamic_class_load_error` to false on `atoms`, and
each strict family declared since sets it to false as well. With the notice
sample rate at 0, enforcement is now silent.

### The current workaround

`HH\classname_to_class_strict_isolation_backdoor()` is a pseudo-function,
lowered to `ClassGetC StrictIsolationBackdoor`, that skips the strict check.
`Classes::loadFromClassnameWithStrictIsolationBypass()` wraps it (D122606445),
and the `STRICT_ISOLATION_BYPASS_USAGE` lint caps calls per file: 53 files and
124 calls today. D122583152 (not landed) would move framework callsites onto
the wrapper. This has three problems:

- Each new framework path is found only when it breaks.
- Anyone can add their file to the ledger.
- The wrapper extends the bypass to every caller.

### Gaps in enforcement

These by-name paths never reach the strict check, so any access policy is moot
until they do:

- `new ReflectionClass($name)` → `newInstance()` → `hphp_create_object()` →
  `ExecutionContext::createObject()` → `Class::load()`
- object instantiation during unserialize
- `HH\Facts\subtypes()` and `transitive_subtypes()`, which return
  `vec<class<T>>` wherever Facts is enabled
- existence probes: `class_exists`, and `is_a` / `is_subclass_of` with strings

### Goals

1. Only the package itself and designated framework code may load a strict
   package's classes by name. New strict packages get this without
   configuration.
2. Grants are declared centrally, in `PACKAGES.toml` under Hack-team review.
   They can't be self-assigned or gained by relabelling a file.
3. New loading code inside a designated framework works without per-callsite,
   per-class or per-package registration.
4. Modules and `__PackageOverride` are unconstrained.
5. Every by-name class resolution follows the same rule.
6. No hot-path cost: whether a function may load must follow from its unit
   alone, so the JIT and HHBBC can decide it at translation time. Compatible
   with RepoAuthoritative builds.

### Non-goals

- Preventing class pointers from escaping once a designated loader has obtained
  them.
- Changing the static rules for strict packages.
- Per-package grants: a loader path may load every strict package.
- Per-function grants.

## User experience

### Configuration

```toml
[strict_isolation]
loader_paths = [
  "//flib/graphql/core/",
  "//flib/intern/graphql/core/",
  "//flib/intern/build/meerkat/steps/graphql/",
  "//flib/entity/core/",
  "//flib/intern/entity/core/",
  "//flib/alite/urimap2/",
  "//flib/alite/x/controller/",
  "//flib/intern/meta/cli/core/",
  "//flib/intern/meta/cli/platforms/metacli/",
  "//flib/intern/site/x/unittest/run/",
]
```

- The list is global. Code under a loader path may load classes from every
  strict package, including ones declared later, and strict packages carry no
  loader configuration of their own.
- It is a table rather than a top-level key because TOML assigns a key written
  after a `[packages.*]` header to that package, where it would be silently
  ignored.
- Entries use `include_paths` syntax: a directory (trailing `/`) or a single
  file.
- The `PACKAGES.toml` parser shared by hh and HHVM requires each entry to be
  normalized, to exist, and to lie outside every strict package, so no strict
  package can load another.
- Older binaries ignore the table (the config structs don't set
  `deny_unknown_fields`), so the config can land before HHVM support.

### Runtime rule

A by-name conversion whose target is in strict package P succeeds only if the
*deciding function* is defined in a file that is:

1. in P (for an implicit family, the same member: `prod_atoms.a` can't load
   `prod_atoms.b`), or
2. under a loader path.

Otherwise P's existing policy applies: throw by default, or a sampled notice if
`raise_dynamic_class_load_error` is false. The error names the target class,
the deciding function and its file.

The deciding function is:

- for bytecode conversions (`HH\classname_to_class`, `new $s`, `$s::m()`,
  `$s::C`, `$s::$p`), the function containing the instruction. A closure is
  attributed to the file that defines it.
- for native APIs (`dynamic_class_meth`, `type_structure`, reflection, …), the
  first non-builtin caller, found with `fromCaller` as module-boundary checks
  already do.

No other frame is consulted, so there is no stack walk. A helper that converts
on its caller's behalf decides by its own file, so framework code must convert
directly (see [Direct-conversion lint](#direct-conversion-lint)).

### Covered resolutions

The rule applies to every by-name class resolution:

- the operations checked today
- `ReflectionClass`, `ReflectionMethod`, `ReflectionProperty` and
  `ReflectionClassConstant` constructed from strings
- `hphp_create_object*`, which backs `ReflectionClass::newInstance*`
- object instantiation during unserialize
- `HH\Facts` APIs that return class pointers, which omit classes the caller
  can't load
- existence probes, pending [unresolved question 1](#unresolved-questions)

### Direct-conversion lint

An Aurora lint, reading the loader paths from `PACKAGES.toml`, flags calls in
loader-path files to helpers that take a classname and convert it internally:
`Classes::assertAndLoad`, `coerceAndLoad`, `parents`, `implements` and `uses`,
and the conversions in `HH_FIXME_CLASS_POINTER_MIGRATION`. The fix is to
convert at the call site and validate the resulting pointer:

```hack
// Before: converts inside Classes.php, which holds no grant.
$cls = Classes::assertAndLoad($cn, IFoo::class);
// After: converts here; Classes::as only checks the pointer.
$cls = HH\classname_to_class($cn) |> Classes::as($$, IFoo::class);
```

Point-free uses such as `Dict\map($names, Classes::assertAndLoad<>)` become
closures defined in the loader file. For untyped input, the
`HH\FIXME\UNSAFE_CAST` that the migration wrapper hides moves to the call site.

### Steward review on loader paths

Code under a loader path holds the grant, so diffs touching those paths require
steward review from the owning framework. This makes the framework team's
review an enforced gate rather than a convention:

- Adding a file under a loader path grants it, so a steward must approve.
- A diff-time *move signal* fires when a diff moves a file containing
  string-to-class conversions out of the loader paths, or moves conversion code
  from a loader-path file to a file outside them. Either change touches a
  loader path, so stewards see it.
- A path added to `loader_paths` can load every strict package, so it needs
  steward coverage as well as `PACKAGES.toml` review.

### Behavior summary

| Situation | Outcome |
| --- | --- |
| Code under a loader path converts directly | Allowed, into any strict package |
| New loading code under a loader path, in a new or existing file | Allowed without registration; the diff needs steward review |
| Code under a loader path calls a converting wrapper | Lint error |
| Loader code moves outside the loader paths | Grant lost; caught by steward review and the move signal, backed by telemetry and atom integration tests |
| A loader directory is renamed or deleted | Config error |
| A path is added to `loader_paths` | Grants every strict package; needs `PACKAGES.toml` review and steward coverage |
| A new strict package or family is declared | Loader paths can load it; no config change |
| A file changes module, or gains or loses `__PackageOverride` | No effect |
| Other code loads a strict-package class by name | Throws |
| An atom loads its own classes by name | Allowed |
| One atom or project loads another's classes | Throws |

## IDE experience

None. There is no new syntax and no new typechecker error.

## Implementation details

### `PACKAGES.toml` parser and hh

- Add the top-level `[strict_isolation]` table with `loader_paths`, validated
  as above (`hphp/hack/src/packages/types.rs`, `config.rs`).
- Thread it through the FFI bridge
  (`hphp/hack/src/packages/ffi_bridge/packages_ffi.rs`) into `PackageInfo`.
- Include it in `PackageInfo::mangleForCacheKey` and in the incremental-repo
  compatibility check (`hphp/runtime/vm/repo-file.cpp`).
- No typechecker semantics change.

### HHVM

- **Unit attributes.** `UnitEmitterAttributes` holds the `strictPackage` and
  `raiseDynamicClassLoadError` bits today. Add two fields, computed from the
  physical path at unit emission alongside
  `strictDynamicReferencePolicyForPath`:
  - a *loader* bit, set when the path is under a loader path;
  - the unit's strict package identity (for an implicit family, its member),
    for the same-package rule.
- **Serialization.** Both fields join the unit cache key and round-trip through
  repo serialization and HHBBC like the existing attributes. No runtime path
  loads `PackageInfo`, preserving the runtime design's constraint.
- **Check.** `checkStrictPackageDynamicReference(cls)` becomes
  `checkStrictPackageDynamicReference(cls, decidingFunc)`. It returns early
  when the deciding function's unit is a loader or in the target's package, and
  otherwise applies the target's policy. Interpreter call sites already have
  the frame, which they pass for module checks
  (`Class::resolve(name, vmfp()->func())`); native APIs use `fromCaller`. The
  notice adds the deciding function and its file.
- **JIT.** Today the JIT emits an inline `ClassHasAttr(AttrInStrictPackage)`
  test with an unlikely branch to the helper. The deciding function, and so its
  unit's attributes, are known at translation time, including in inlined code:
  - in a loader unit, emit nothing;
  - in a unit outside every strict package, emit today's check unchanged;
  - in a strict-package unit, compare the target's package with the unit's, a
    translation-time constant, on the unlikely branch before calling the
    helper.
- **HHBBC.** An explicit `ClassGetC` conversion counts as possibly raising when
  the class `mightBeInStrictPackage()`. Drop that term in loader units;
  elsewhere stay conservative, as today.
- **Gaps.** Route the reflection, `hphp_create_object*`, unserialize and Facts
  paths through the same check.

### www

- The `[strict_isolation]` table with the initial loader paths
- The direct-conversion lint and the move signal
- Steward review for each loader path
- Rewrites of existing wrapper calls under loader paths

## Migration and rollout

### Migration

- The initial loader paths cover 52 of the 53 ledger files
  ([Appendix B](#appendix-b-ledger-coverage)).
- 12 wrapper calls in 8 files must convert directly, plus anything else the
  lint finds under loader paths.
- `flib/intern/meta/cli/trust/privacy/PeopleDataAgentAccessCheck.php` is in
  the ledger but outside framework directories. It needs a single-file entry or
  a refactor.
- Delete `Classes::loadFromClassnameWithStrictIsolationBypass`,
  `StrictIsolationBypassUsageLintRule` and the backdoor exception in
  `NoHHFunctionsLintRule`, and abandon D122583152. Later, delete
  `HH\classname_to_class_strict_isolation_backdoor` and
  `ClassGetCMode::StrictIsolationBackdoor`.

### Rollout

1. Route every by-name resolution in HHVM through the strict check, still
   target-only. Because every strict family sets
   `raise_dynamic_class_load_error` to false, the newly covered paths produce
   only notices.
2. Add and validate `[strict_isolation]`, and land the initial loader paths.
3. Turn on steward review for every loader path.
4. Add the unit attributes and the caller-aware rule to HHVM, including the JIT
   and HHBBC changes.
5. In www, add the direct-conversion lint and the move signal, and rewrite the
   wrapper calls.
6. Raise `Eval.StrictPackageDynamicReferenceNoticeSampleRate` above 0 in
   sandboxes, CI and `intern_atoms_all`. The option is runtime-only, so no
   rebuild is needed. The notices inventory every dynamic load into strict
   packages before anything throws; fix any outside the loader paths.
7. Once HHVM has rolled, delete the wrapper and the ledger, and set
   `raise_dynamic_class_load_error` to true on each strict family as its
   notices reach zero.

## Design rationale and alternatives

### Why loader paths

The grant needs an identity the code's author can't assign themselves:

- **Modules** are self-declared (`allowed_files_for_module_declarations = *`).
  Carrying a privilege would force new membership rules onto them (see
  [Module-based grants](#module-based-grants)).
- **Packages** can be joined with `__PackageOverride`.
- **Physical paths** can't be relabelled. HHVM already uses them to decide
  strict-package membership, which `__PackageOverride` doesn't affect.

The other choices follow from the goals:

- **One global list.** Whether a function may load is then a property of its
  unit, fixed at emission. The JIT drops the check from loader code entirely
  and keeps today's target-only check in all other code outside strict
  packages. The loaders are generic: GraphQL, Ent and routing load whichever classes
  register, from any package, so per-package lists would repeat the same paths.
  See [Per-package loader lists](#per-package-loader-lists).
- **One frame.** Checking only the deciding function avoids a stack walk and
  lets the JIT and HHBBC resolve the caller side statically. The cost is that
  frameworks must convert directly instead of through shared wrappers.
- **Config, not syntax.** The implementation is parser validation, per-unit
  attributes and one caller-aware check. With no new syntax, www needs no
  HHVM-version gating.

### Alternatives considered

#### Per-package loader lists

An earlier draft declared the loader list on each strict package:

```toml
[implicit_packages.prod_atoms]
path = "//atom/prod/"
dynamically_loadable_from = ["//flib/graphql/core/", "//flib/entity/core/", ...]
```

This gives each package least privilege, and a new strict package would start
closed to every loader. It was rejected because:

- **It would almost certainly regress loader code.** Each unit would carry the
  set of packages it may load, and every conversion into a strict package from
  loader code would test the target's package against that set. The target is
  a runtime value, so the JIT can't fold the test, and in loader code those
  conversions are routine rather than the rare case the unlikely branch
  assumes. The check is live in `prod` and `intern` builds, so the cost would reach
  production. A global list makes the grant one bit per unit, which the JIT
  resolves at translation time.
- **The lists would be identical.** The loaders discover plugins generically,
  so each of the four strict families would carry the same paths, and every new
  strict package would copy them.

#### Module-based grants

This variant identifies loaders by Hack module instead of path:

```toml
[strict_isolation]
loader_modules = ["graphql", "ent", "meta_cli", "www_test", "alite_routing"]
```

A conversion into P succeeds if the deciding function's file is in P or
declares a listed module. HHVM already records each function's module for
`internal` checks (`Func::moduleName()`), so the grant can be computed at unit
emission like the path grant. The direct-conversion lint and the 12 wrapper
rewrites are unchanged.

41 of the 53 ledger files are already in a listed module. The other 12 would
change module:

- 5 MetaCLI core files join `meta_cli`.
- `GraphQLSubschemaGenerator` joins `graphql`.
- 3 Alite routing files join `alite_routing`, a new module.
- 3 files in `intern` (2) or `zones` (1) move to `graphql` and may lose access
  to `internal` symbols.

`facebook`, `intern.facebook`, `intern` and `zones` could never be listed.

| | Loader paths (proposed) | Modules |
| --- | --- | --- |
| Identity | Physical path | `module` declaration |
| Gaining a grant | Add a file under a loader path (steward review) | Join a listed module from anywhere (membership lint, if any, and review routing) |
| Losing a grant | Move the file out of the loader paths (move signal, steward review) | Drop its `module` declaration (review routing) |
| Reorganizing a framework | `PACKAGES.toml` change | No config change |
| Grant scope | Listed directories | All members of the listed modules |
| Config | About 10 paths | 5 module names |
| Migration | 12 wrapper rewrites | 12 wrapper rewrites and 12 module changes |
| Constraints on modules | None | Membership of listed modules becomes an access-control boundary |

Modules never drop a grant on a file move or reorganization, and they reuse
boundaries the framework teams already maintain: GraphQL requires
`module graphql;` in its core files and defaults new symbols to `internal`.
They were rejected because:

- **Membership is self-declared.** Anyone can gain the grant by joining a
  listed module. The only guards are membership lints (`GraphQLModuleLintRule`,
  `EntModuleLintRule`, `NoExternalWWWTestModuleMembershipLintRule`), which can
  be suppressed and which `meta_cli` lacks, and review routing through
  Butterfly's affected-Hack-modules condition. Carrying a privilege would need
  new path-containment and mandatory-membership rules on the framework modules.
  A module-scoped attribute (D117947197) is already on hold.
- **The grant is wider than the loading code.** GraphQL's membership lint
  admits Sandcastle, GraphiQL, MetaCLI and Instagram GraphQL paths, and
  `meta_cli` already includes `PeopleDataAgentAccessCheck`.
- **Grant loss is just as silent.** Leaving a module drops the grant at runtime
  without an hh error.
- **More migration.** 12 files change module before enforcement.

#### Other alternatives

| Alternative | Why not |
| --- | --- |
| Keep the backdoor wrapper and ledger (D122606445, D122583152) | Keeps the three problems listed under [The current workaround](#the-current-workaround). |
| Grant by the caller's package | `__PackageOverride` can put a file in `prod` or `intern`, so the grant could be relabelled. [HIP: Package observability](https://docs.google.com/document/d/1lnrl8UXXGq8Eat446Yjwlbj1mK0U6vZDY9dKXm8ksdk) rejected per-caller rules for this reason. A path-derived variant is under [Package-tagged loaders](#package-tagged-loaders). |
| Per-function loader marker | Least privilege, but every new loading function is discovered by failure. |
| Restrict the backdoor to allowlisted locations at compile time | Turns the ledger into an hh error but keeps per-callsite rewrites and discovery by failure. |
| Export by base type, or `<<__DynamicallyReferencedOutsidePackage>>` | No caller identity, so any code could load the exported classes. |
| Grants per execution context ("mounts") | Any code running during an atom's work could load that atom, so loads from non-core code aren't prevented. |
| Governed registries: typed lookups through module-internal handles | Statically visible and least privilege per plugin type, but every callsite must migrate, and untyped conversions can't be found statically. |
| Package-local generated manifests: class-pointer literals generated inside each package | Fully static, but requires rewriting each framework. See [Future possibilities](#future-possibilities). |
| Credentials (`__FUNCTION_CREDENTIAL__`, package credentials) | Frameworks discover plugins rather than receive registrations, and a credential can be passed on once handed out. |
| Wrapper frames that defer to their caller | Needs a stack walk. Higher-order helpers (HSL) between the loader and the wrapper force a growing trusted set. Subtle under JIT inlining and async. |

## Drawbacks

- **Moving loader code out of the loader paths drops the grant at runtime.** hh
  doesn't flag it; the move signal, telemetry and atom integration tests do.
- **Grants cover every strict package.** A loader path can load any strict
  package, including ones declared later, and a strict package can't exclude a
  loader. Adding a loader path is a decision for every strict package at once.
- **Grants are directory-wide.** Any file in a loader directory can load;
  steward review on those paths is the control.
- **Steward review adds load.** Every change under a loader path needs a
  steward, adding review latency for those teams.
- **Framework code must convert directly.** Existing wrapper calls need
  rewriting (12 today), and the lint enforces it going forward.
- **Reorganizing a framework's directories requires a `PACKAGES.toml`
  change**, and loader paths may repeat paths a package already declares (see
  [Package-tagged loaders](#package-tagged-loaders)).
- **Limits shared by any runtime scheme:**
  - Dynamic loads are checked when they execute, not statically.
  - A class pointer obtained by a loader can be passed anywhere.
  - A public framework API that converts caller-supplied names extends the
    grant to its callers, so such APIs need an audit.

## Prior art

- **Java security policy.** A policy file grants permissions by code location.
  This grant lets code under `/opt/framework/` bypass access checks on any
  class through reflection:

  ```
  grant codeBase "file:/opt/framework/-" {
    permission java.lang.reflect.ReflectPermission "suppressAccessChecks";
  };
  ```

  Loader paths have the same shape, a central path-keyed grant covering every
  target. They check one frame, where the `SecurityManager`, since disabled in
  the JDK, checked every frame up to a `doPrivileged` call.
- **Java modules.** `opens P to M` in `module-info.java` gives the named
  modules runtime-only reflective access to package P, with no compile-time
  access, and the JDK checks the immediate caller (`@CallerSensitive`). As with
  loader paths, static references stay errors while designated code may load
  by name; unlike them, each opened package names its own readers.
- **.NET friend assemblies.** `InternalsVisibleTo` is declared by the target
  assembly. For strong-named assemblies the friend must be named with its full
  public key, so another assembly can't claim the grant by taking the friend's
  name.

## Unresolved questions

1. **Existence probes.** For callers without a grant, should `class_exists`,
   string `is_a` and `is_subclass_of`, and `class_parents`, `class_implements`
   and `class_uses` return `false`, keeping a strict package's presence
   unobservable, or stay unchanged?
2. **Facts pointer APIs.** For callers without a grant, should they omit
   strict-package classes (proposed) or throw?
3. **Move signal.** Should it warn or block?
4. **Class entries.** Should `loader_paths` also accept class names? They
   survive file moves and can be validated, at the cost of listing each class.
5. **Stewards.** Which stewards cover the loader paths: each framework's own,
   or Hack's as well? Can steward coverage be derived from `loader_paths` so
   the two can't drift?

## Future possibilities

### Package-tagged loaders

A follow-up will investigate a package-level flag, working name
`allow_dynamic_loading`, meaning "files in this package may load strict-package
classes by name". It grants exactly what the global list does, through
existing package syntax: where a package already covers a framework's loading
code, `loader_paths` needn't repeat its paths. To keep [goal 2](#goals),
membership must be path-derived, as HHVM already computes it for strict
packages, so `__PackageOverride` can't confer the flag. HHVM would set the same
per-unit loader bit at emission, leaving the runtime unchanged. Strict packages
couldn't carry the flag.

### Package-local generated manifests

If frameworks referenced plugin classes through class-pointer literals
generated inside each package, loads would be fully static and grants
unnecessary. That requires rewriting each framework, so it is a long-term
direction rather than a replacement for this proposal.

## Appendix A: Evidence

| Claim | Reference |
| --- | --- |
| The strict check considers only the target | `hphp/runtime/vm/runtime.cpp:593` |
| Same-package string loads fail | `hphp/test/slow/packages/strict-dynamic-reference/bytecodes.php.expect` (`enforced:same-package-classname`) |
| Caller awareness was deferred | Runtime design doc, mdoc `808b4f71-4faf-4100-924c-e267d1abce92`, "Future work" |
| The current strict families | `www/PACKAGES.toml:67-92` |
| `prod` and `intern` deploy the projects families | `www/PACKAGES.toml:105-111` |
| Repo builds check only when the deployment has a strict package | `hphp/compiler/compiler.cpp:829-832`; `hphp/runtime/vm/runtime.cpp:594-597` |
| The JIT tests the target inline and branches to the helper | `hphp/runtime/vm/jit/irgen-call.cpp:1496-1519` |
| HHBBC treats conversions into possibly strict classes as raising | `hphp/hhbbc/interp.cpp:2579-2582` |
| The backdoor is a pseudo-function | `hphp/hack/src/hackc/emitter/emit_expression.rs:3091-3095` |
| The wrapper and the ledger | `www/flib/core/utils/Classes.php:21`; `www/flib/intern/aurora/lint_rules/ast_linters/hf/StrictIsolationBypassUsageLintRule.php` |
| Units carry two booleans today | `hphp/runtime/vm/unit-emitter-attributes.h:29-31` |
| Strict membership comes from the physical path | `hphp/runtime/base/package.cpp:206-216`; `hphp/runtime/vm/unit-emitter.cpp:72-95` |
| Resolution sites already pass the caller frame | `hphp/runtime/vm/bytecode.cpp:975`; `hphp/runtime/vm/class.cpp:5491-5510` |
| Natives find their caller with `fromCaller` | `hphp/runtime/ext/hh/ext_hh.cpp:883`; `hphp/runtime/ext/std/ext_std_classobj.cpp:35-46` |
| The reflection gap | `hphp/runtime/ext/reflection/ext_reflection.h:130-134`; `hphp/runtime/ext/reflection/ext_reflection_hni.php:1829-1838`; `hphp/runtime/ext/reflection/ext_reflection.cpp:509-520`; `hphp/runtime/base/execution-context.cpp:1108-1121` |
| The unserialize gap | `hphp/runtime/base/variable-unserializer.cpp:1205-1213` |
| Facts returns class pointers | `hphp/hack/hhi/ext_facts.hhi:183-197` |
| Unknown config keys are ignored | `hphp/hack/src/packages/types.rs`, `config.rs:21-28` (no `deny_unknown_fields`) |
| 4472 cannot be suppressed | `www/.hhconfig:77` |
| Module membership is unrestricted | `www/.hhconfig:86` |
| Validators that only check pointers | `www/flib/core/utils/Classes.php:38-64` (`as`, `nullableAs`) |
| Wrappers that convert internally | `www/flib/core/utils/Classes.php:78-111` and `193-212`; `www/flib/core/hack/migration_hacks/HH_FIXME_CLASS_POINTER_MIGRATION.php:165` |

## Appendix B: Ledger coverage

Coverage of the current `STRICT_ISOLATION_BYPASS_USAGE` ledger (53 files) by
the initial loader paths:

| Loader path | Ledger files |
| --- | --- |
| `//flib/intern/graphql/core/` | 23 |
| `//flib/graphql/core/` | 12 |
| `//flib/intern/meta/cli/core/` | 4 |
| `//flib/entity/core/` | 3 |
| `//flib/intern/meta/cli/platforms/metacli/` | 3 |
| `//flib/alite/urimap2/` | 2 |
| `//flib/intern/build/meerkat/steps/graphql/` | 2 |
| `//flib/alite/x/controller/` | 1 |
| `//flib/intern/entity/core/` | 1 |
| `//flib/intern/site/x/unittest/run/` | 1 |
| Not covered: `flib/intern/meta/cli/trust/privacy/PeopleDataAgentAccessCheck.php` | 1 |

Wrapper calls in ledger files that must convert directly:

| File | Calls |
| --- | --- |
| `flib/intern/meta/cli/core/MetaCLISubcommandRegistrationDataCache.php` | 3 |
| `flib/graphql/core/metadata/types/TGraphQLGenericFieldDefinition.php` | 2 |
| `flib/intern/graphql/core/metadata/GraphQLBuildMetadataSandboxCache.php` | 2 |
| `flib/entity/core/schema/internal/EntSchemaBaseInternal.php` | 1 |
| `flib/graphql/core/utils/GraphQLSDLTextBuilder.php` | 1 |
| `flib/intern/build/meerkat/steps/graphql/sdl/GraphQLSchemaSDLHashSandboxCache.php` | 1 |
| `flib/intern/entity/core/schema/sandbox_cache/EntSchemaMetadataSandboxCache.php` | 1 |
| `flib/intern/site/x/unittest/run/HackTestSingleTestExecutor.php` | 1 |
