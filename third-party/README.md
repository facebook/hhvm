hhvm-third-party
================

All of the dependencies that [hhvm](https://github.com/facebook/hhvm) needs which don't have distribution/homebrew
packages.

Updating Facebook dependencies
------------------------------

Facebook dependencies (e.g. folly, thrift, proxygen) should all be updated simultaneously, to identical versions.

Some tips:

- work in the oldest supported Linux distribution; this gets you the oldest C++ compiler and the oldest CMake, so you'll hopefully get most of the possible errors in your interactive environment, rather than hours later on CI
- it's also worth setting up an interactive build environment on your mac; if CI fails, try uninstalling the package from homebrew then figure out how to make the build pass without it
- it's easiest to do this with a separate build directory (e.g. `build/` or `obj-x86_64-linux-gnu`), as this lets you easily delete all the relevant build state/artifacts without deleting your changes to the source and cmake.
- several of these projects need patching; these can have several patch files in `patches/`, and they are applied in the order listed in `series`. [quilt](https://wiki.debian.org/UsingQuilt) can be used to manage these as a form of source control, and is usually worth configuring.
  - The build will use quilt to apply patches if you `export HHVM_TP_QUILT=$(which quilt)`; you may need to delete `$BUILD_DIR/third-party/` after setting that to get a clean build.
  - If patches fail to apply or more are needed:
    1. `cd` to the extracted source directory, e.g. `$BUILD_DIR/third-party/folly/bundled_folly-prefix/src/bundled_folly`
    2. `export QUILT_PATCHES=$SOURCE_DIR/third-party/$PROJECT/patches`
    3. `quilt push -a`
    4. If it fails because the patch has already been applied, use `quilt delete PATCH_NAME_HERE.patch`; if it is still needed but needs updates, use `quilt push -f` to produce a `.rej` (rejected changes) file, manually apply the changes, then run `quilt refresh` when you are done
    5. if you need to add additional files to the patch, use `quilt add FILE` **before** editing the file, then `quilt refresh` when you are done
    6. if you need to add an additional patch, use `quilt new MY_PATCH.patch`, `quilt add FILE`, `quilt refresh`
    7. if you want to update all the patches with current line numbers etc, run `quilt pop -a --refresh`

Some Facebook-employee-specific tips:

- you can allocate a VM with a recent prebuilt environment via `i opensource/ondemand/facebook/hhvm`; when prompted, select the most recent successfull built for the oldest Ubuntu listed.
- if the Mac CI is unable to find a dependency but it's found locally, uninstall it with `brew uninstall --force --ignore-dependencies FOO`; you can then install the same package used in the CI with `yum install`; find the package with `yum search foo` - you probably want the most recent `nix2rpm` package, though you can check exactly which is used in `hphp/facebook/autobuild/cmake/mac/cmake.sh`

Overall process:

1. Get a working build environment (ideally one on old Ubuntu, one on MacOS)
2. Most FB projects should be fetching release tarballs with tags like `vYYYY.MM.DD.00`; full download URLs are listed in third-party/foo/CMakeLists.txt. Update these tags to the version you're targetting.
3. Download all of the URLs you just modified. For some projects, this will give you `vYYYY.MM.DD.00.tar.gz`, rename to `PROJECT-vYYYY.MM.DD.00.tar.gz`
4. Update the hashes in `third-party/foo/CMakeLists.txt` to match `openssl dgst -sha256 *.tar.gz`
5. **Requires FB employee**: update our CI caches - this can be done on your laptop, it does not need a devserver.
   `for file in *; do manifold put $file hhvm_opensource_dependency_cache/flat/$file; done`
6. Try to build; fix any patches/build issues
7. Create pull request and (requires FB employee) diff
8. If you were working in Ubuntu, pull onto MacOS and do a local test build. If you were working in MacOS, pull into Ubuntu and do a test build
9. Land once everything's good :) Don't do this on a Friday.
10. Check nightly build results the next day. Fix any portability issues that weren't caught by CI.

Steps 2-4 can be done by `update-first-party.sh`
Step 5 can be done by `upload-first-party-to-manifold.sh`


Common Patterns/Targets
-----------------------

Most things here have 3 targets:
- `foo`
- `foo_deps`
- `bundled_foo`

If `bundled_foo` is used, `foo` depends on `bundled_foo`; for some projects (like folly), this is always the case, but for others (like boost), the system version of the library will be used instead if possible, and the `foo` target will be configured to point to it.

`foo_deps` is a fake target, that defines all of the direct dependencies; `foo` will always depend on `foo_deps`. This target is required because if `myapp` depends on `foo` and `foo` depends on `bar`, link order matters: the linker must know that `foo` depends on `bar`. `myapp` -> [`foo`, `bar`] is not sufficient.

Finally, we also frequently have a `FOO_DEPS` variable in addition to the `foo_deps` target; this is because:

- `foo_deps` is an `INTERFACE` library in cmake; this means that the target is fake (`.PHONY` in gmake terms) - there is no .a/.so/.dylib output. it just tracks dependency information
- `bundled_foo` is also a special target (defined by `ExternalProject_Add`); if given an INTERFACE dependency, it makes it part of its' own interface dependencies, but doesn't actually build-depend on it.
- `INTERFACE` targets are only allowed `INTERFACE` libraries; in particular, `PUBLIC` are not permitted by CMake, which is what would be correct here
- so, we make `bundled_foo` depend directly on `${FOO_DEPS}`, rather than the interface `foo_deps`, even though they're otherwise equivalent.

LICENSING
---------

Various licenses are used in this project; see the notices in relevant files and subdirectories for details.
