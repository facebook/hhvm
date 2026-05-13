# mcrouter Agent Instructions

## Flavor Settings System

Mcrouter uses a **per-flavor settings system** built on folly::Settings and Luna Settings. Each flavor (Web, Tao, TaoStandalone) has its own set of option values.

### Option definitions

Mcrouter options are defined in two header files using `MCROUTER_OPTION_TOGGLE` / `MCROUTER_OPTION_*` macros:

- **`mcrouter/mcrouter_options_list.h`** — libmcrouter options (used by all mcrouter instances, embedded and standalone)
- **`mcrouter/standalone_options_list.h`** — standalone-only options (only used when mcrouter runs as its own binary)

### Flavor configs (dual system — both required)

There are currently **two parallel config systems** for flavor options. Both are still required and must be kept in sync when changing option defaults.

1. **Legacy flavor configs** (`configerator/source/mcrouter/flavors/`):
   - `{flavor}.cconf` — libmcrouter options for a flavor
   - `{flavor}-standalone.cconf` — standalone options for a flavor
   - Compiled into materialized JSON, loaded at startup

2. **Luna Settings** (`configerator/source/mcrouter/settings/{project}/`):
   - `startup_settings.cconf` — imports defaults from `configerator/source/mcrouter/flavors/luna_options/{project}_options.cinc`
   - `runtime/runtime_settings.cconf` — runtime-updatable settings
   - Uses folly::Settings registered per-flavor in `mcrouter/facebook/settings/Settings{Flavor}.cpp`

### Priority (highest wins)

1. **CLI flags** — passed directly via TW spec arguments or systemd bootstrap
2. **Luna Settings / configerator** — loaded at startup and updated at runtime
3. **Legacy flavor configs** — loaded at startup
4. **Compiled defaults** — from the option definition macros in the `.h` files

### Key files

| File | Purpose |
|------|---------|
| `mcrouter/mcrouter_options_list.h` | Libmcrouter option definitions |
| `mcrouter/standalone_options_list.h` | Standalone option definitions |
| `mcrouter/facebook/settings/flavor_config.yaml` | Codegen metadata for per-flavor Settings classes |
| `mcrouter/facebook/settings/Settings{Flavor}.{h,cpp}` | Per-flavor folly::Settings registration |
| `mcrouter/facebook/settings/FlavorProjects.h` | Maps flavor enum → Luna project name |
| `mcrouter/facebook/settings/Init.cpp` | Loads Luna Settings from configerator for each flavor |
| `mcrouter/facebook/systemd/mcrouter_bootstrap.py` | JustKnobs-based CLI flag overrides for systemd deployments |

### Checking current option values

- **Legacy flavor configs**: `configerator/source/mcrouter/flavors/{flavor}.cconf` (libmcrouter) and `{flavor}-standalone.cconf` (standalone)
- **Luna Settings**: `configerator/source/mcrouter/flavors/luna_options/{project}_options.cinc` and `configerator/source/mcrouter/settings/{project}/`
- **JustKnobs**: `ig/mcrouter:{flavor}_{option_name}`
- **TW spec CLI args**: hardcoded `--flag` arguments in TW specs (highest priority)

## OSS Build

- OSS CMakeLists.txt is at `mcrouter/public_tld/CMakeLists.txt`
- OSS stub files (e.g., `mcrouter_sr_deps-impl.h`) provide minimal implementations for open source builds; real implementations are in `mcrouter/facebook/`
- Build output paths like `/fbcode_builder_getdeps/shipit/mcrouter/` map to `fbcode/mcrouter/` in the repo
- When `install(TARGETS ...)` includes targets with `PUBLIC_HEADER` property, add `PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}` to avoid warnings
