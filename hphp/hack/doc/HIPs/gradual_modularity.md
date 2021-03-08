# Gradual Modularity in Hack

Status: draft, not actively being worked on. If this changes, this pre-HIP proposal should be updated to match
the current HIP template.

Last updated: 2019-10-09

Shared as a HIP for external visibility

## The Problem

In production builds of Facebook WWW, certain directories are dropped, such as internal tools and test directories.
This means that any code attempting to access symbols in those directories will fatal at runtime with an autoloader fatal. Hack is not currently able to enforce this.

As a related problem, due to the global nature of our repo, framework developers struggle to hide the implementation details of their frameworks. Often, we rely on naming conventions like `_DO_NOT_USE`. But, again, there is no static analysis to enforce this, and these symbols end up getting used anyway.

Then, there are similar problems that arise when attempting to define 'black box' APIs - we may want hard isolation while maintaining access to core infrastructure: for example, there may be a blessed library to interact with a
data store, and direct access should be banned except by the library.

Lastly, there's the classic problem of open-sourcing Hack frameworks with a source of truth as part of a proprietary monorepo: for example, all of Facebook WWW can use the HSL, but the HSL can not depend on any proprietary code.

These problems are manifestations of a more general problem: the lack of modularity in the Hack language—everything is globally accessible. Upon closer inspection, we can see that there are multiple granularities to this problem: two examples are the "environment" level and the "library" level.

* Environments represent build-time partitioning of the repository. The 'intern' (internal tools and libraries)/production boundary is a classic example: code in intern may access code in prod, but not vice-versa. This is most akin to the workspaces feature of modern package managers (see Prior Art).
* Libraries are logically related groups of code. These are like modules/packages that we see in most modern languages. Libraries may selectively hide their internals from their dependencies.

Note that these granularities are not mutually exclusive—for example, HSL is both a library and an environment.

In the same way that we slowly introduced types into an untyped Hack, we can introduce modularity gradually, from coarsest to finest granularity. Accordingly, **this document explores a solution to the problem of "environments,"** while leaving the possibility open to solving the "libraries" problem in the future.

## Solution Requirements

* **Discreet.** We desire a limited number of environments in Facebook's WWW codebase, to retain the advantaged of monorepo development.  Environment definitions should be hidden from developers, and should be hard for developers to subvert. Essentially, this means that any per-file syntax is undesirable.
* **Hierarchical.** Environments should be able to depend on others, where "environment A depends on environment B" is defined as: code from A may reference code from B, but code from B may *not* reference code from A. This allows, for example, Intern to access Prod code but not vice-versa. Dependencies are not transitive: i.e. if A depends on B and B depends on C, A does not implicitly depend on C.
* **Permeable.** While environment dependency is generally one-way, we must have a type-safe way of piercing that boundary (for example, *some* Intern symbols *can* be accessed from Prod). This implies that the feature should be first-class-ish enough for the typechecker/compiler/runtime to interact with it. This requirement is intended to be a WWW-only compromise, so we can tolerate it being clunky to use as a deterrent.
* **Single Source of Truth.** Environment specifications should be able to be consumed by the typechecker (to statically enforce the boundaries), the runtime (to dynamically enforce the boundaries), and the build system (to selectively include code in the build). This way, the typechecker has an accurate view of reality.

## Abstract Proposal: Environments

*This section describes the environments feature in an abstract sense, using pseudocode, to bring the semantics of the feature to the forefront. Later sections propose a concrete syntax for the feature.*

An "environment" is an isolated subdivision of a monorepo, where "isolated" means that code defined outside of a particular environment may not access code within that environment (barring certain exceptions). The boundaries are enforced both by the typechecker and the runtime.

The build system will select environments to include in the build, but all environments do not have to correspond to a hard runtime boundary. This allows environments to be used for massive sections of a repository (e.g. intern/prod), but also for libraries (e.g. HSL).

### Defining Environments and Builds

An environment is specified inside a special configuration file in a directory. A special filename isn't strictly necessary, but it makes environments easy to find within a codebase containing extremely large numbers of files, and makes it harder for users to accidentally define environments.

```
// Example: in ~/www/flib/environment.x:

environment {
  name = Prod
}
```

All code in source files in said directory and recursive subdirectories belongs to this environment. A source file belongs to exactly one environment: the nearest one defined. For example, if `flib/` defines an environment "Prod" and `flib/intern/` defines an environment "Intern", then all code under `flib/intern/` belongs only to Intern. Source files which do not have a nearest environment definition belong to a “default environment”.

Environments can depend on multiple other environments. For migration purposes, all environments implicitly depend on the “default environment,” with the assumption that all code will eventually move to a defined environment. To define an environment that depends on another, simply define the environment normally and include its dependencies:

```
// Example: in ~/www/flib/intern/environment.x:

environment {
  name = Intern
  dependencies = [Prod]
}
```

As with the first example, all code under `flib/intern/` belongs only to Intern. However, now that Intern depends on Prod, code belonging to Intern may access code in Prod, but not vice-versa! The dependency relationship is not transitive; for example, if there were an environment "Scripts" that depended on Intern, it would only be able to access code in Prod, if it also depended on Prod.

That `flib/intern/` is a subdirectory of `flib/` is irrelevant—dependent environments don't have to be defined in subdirectories of the environments they depend on. Conversely, Intern doesn't have to depend on Prod just because `flib/intern/` is a subdirectory of `flib/`. In fact, the relationship can be inverted—for example, Prod could depend on an "HSL" environment defined in `flib/core/hack/lib/`!

All environments implicitly depend on the aforementioned “default environment”. This allows for environments to be gradually migrated into a monorepo, without having to do it all at once. It also handles the case of HHVM builtins, which are currently provided by HHIs. Eventually, they may all be inside of a “builtin” environment, but initially they will simply live in the default environment and be accessible everywhere.

Given that `__tests__` are scattered around the codebase currently, any proposed feature would be restricted to running regexes on filenames, which would hamstring its ability to integrate into the language. **Ideally, this feature assumes that tests in WWW are moved from `__tests__` directories to one top-level `tests/` directory.** But, we may be able to reach a compromise if one environment.x file is able to define multiple environments via regular expressions that only apply to files in that directory.

```
// Example: in ~/www/flib/environment.x:

environments {
  ProdTests {
    regex = "#/__tests__/#"
    dependencies = [Prod]
  }
  Prod {
    regex = "#.*#"
  }
}
```

A “build” is a collection of environments that indicate the code that’s available in a particular build of the repository. It is specified inside a special configuration file in the root directory of the repository. For example, Facebook’s builds might look like this. Note that our builds evolved to be hierarchical, allowing us to only specify one environment per build, but it's not necessary.

```
 // Example: in ~/www/builds.x:

build {
  name = Prod
  environments = [Prod]
}

build {
  name = Intern
  environments = [Intern]
}

build {
  name = Scripts
  environments = [Scripts]
}
```

### Permeability

There are two cases of issues in which existing boundaries in WWW are violated. The first issue is simpler than the other: certain classes are defined in Intern, but really should be in Prod. The solution to this problem is to make environments typechecker-only initially, and use the static analysis to move definitions to where they need to be before turning on runtime enforcement.

The second issue is trickier. Sometimes, definitions can't be moved, explicit checks on the current environment at runtime, which explicitly break the abstraction. To support this permeability, we need to be able to introspect on which environments are available in the current build.

```
// Example: in some prod file:

if (<current build includes Intern>) {
  // Access intern code here...
}
```

Or, if we’re refining with some other mechanism (for example, being is a script context implies that the Scripts environment is available), then users can assert an environment is available with `invariant` to provide a useful error message.

```
// Example: in some prod file:

if (Environment::isScript()) {
  invariant(
    <current build includes Scripts>,
    'Being in a script implies that Scripts is available',
  );
  // Access scripts code here...
}
```

### Enforcement & User Experience

Environment accessibility is enforced whenever an identifier is referenced. Enforcement is based entirely on the source files of the “caller” and “callee”—the permeability construct doesn’t affect functions called within a permeability block.

```
// Example: in some prod file:

class ProdClass extends InternClass // Error
  implements InternInterface { // Error
  use InternTrait; // Error
}

function f(mixed $x): void {
  intern_function(); // Error
  intern_function<>; // Error
  InternClass::SOME_CONST; // Error
  h<InternClass>(); // Error
  new InternClass(); // Error
  $x is InternClass; // Error
  if (<current build includes Intern>) {
    intern_function(); // OK
    intern_function<>; OK
    InternClass::SOME_CONST; // OK
    h<InternClass>(); // OK
    new InternClass(); // OK
    $x is InternClass; // OK
    g(); // Note that the usages in g() are still errors
  }
}

function g(): void {
  intern_function(); // Error
  echo InternClass::SOME_CONST; // Error
  // etc...
}

function h<reify T>(): void {}
```

**When typechecking a file**, the typechecker maintains a list of available environments. Initially, the list has exactly one element. Whenever a symbol is referenced, the typechecker compares the environment of the current source file against the environment of the referenced symbol. If they’re incompatible, an error is raised. The permeability construct will add the checked environment to the otherwise singleton list of current environments, and the typechecker will iterate over the current environments within the permeating block.

When a boundary violation is found, the typechecker's error message will list the two environments, including an explanation as to why each source file is in its respective environment. The explanation can be computed from the two ways environments can be defined (directory plus optional string pattern). However, the error message will *not* point to the environment file, as we do not want to advertise this feature to WWW users (but they can easily find it by looking in the directory specified).

For example, consider the environments defined above (Prod, ProdTests, Intern). If code from Intern attempted to access code from ProdTests, the error message would say:

```
Cannot reference a symbol in another environment
  This use-site is in the Intern environment because it is in directory flib/intern/
  The symbol X is in the ProdTests environment because it is in directory flib/ and matches the pattern __tests__
  Intern does not depend on ProdTests
```

**In repo-authoritative mode,** the current build should be known when the repository is compiled (e.g. we know if we are going to deploy to prod, intern, etc). This means that environments that aren’t part of the build will simply be dropped from the repository, any environment checks on identifiers can be elided, and permeability conditions can be statically checked and compiled out (akin to `ifdef`). However, if a function is annotated with the `__DynamicallyCallable` attribute, then the checks must remain, because we don’t know which environments it may be called from.

**In sandbox mode,** the available environments must be defined per-request, so that they behave identically to how such a request would behave in production. For example, in development, a user should not be able to access scripts in a web request. The native autoloader can be environment-aware, and refuse to load symbols if the environments are incompatible. Similarly to the typechecker, the permeability construct will “refine” the current environment to the one checked within the scope of the checked block.

Some ideas on how to determine the available environments:

* Could introspect on WebController and related frameworks to create a build object
* Could use native autoloader to determine the right build object
* __EntryPoint could have a way of specifying which environments are available

## Application in WWW

### Philosophy

Facebook has benefited tremendously by betting on the monorepo model. This feature is intended to mitigate some shortcomings of the monorepo without cutting into the benefits. We want to help users understand when they’re writing code that will behave differently in production, but we do not want to encourage carving up the WWW repo into silos. We want to avoid a world in which developers do not feel empowered to contribute to parts of the codebase because it “belongs” to someone else. We also don’t want to enable an intractable dependency graph to develop in the WWW repo.

Therefore, for the initial rollout of this feature, **we will only allow environments that correspond to a form of hard isolation (e.g. a build boundary, or an OSS library).** Framework maintainers who wish to hide implementation details will have to wait—environments are not intended to be a packages feature.

### Rollback Plan

What if we decide that environments aren't the right solution for WWW? Perhaps they'd proliferate too quickly or make the WWW developer experience too cumbersome. Recall this property of environment definitions discussed above:


> Source files which do not have a nearest environment definition belong to a “default environment”.


It follows that to roll back to a pre-environment state in WWW, it would be sufficient to delete all environment files from WWW. Then, all declarations would be in the default environment, and would be accessible to each other, both in the typechecker and the runtime (if we get that far before rolling back).

The environment checking code could then be removed from the typechecker, followed by the code that records environments into saved states.

### Sample Configuration

This is one possible initial configuration which adheres to the principle defined above:

* **HSL: **The Hack Standard Library, which should depend on nothing except builtins provided by HHVM.
    * Depends on: <NONE>
* **HSL-Experimental: **Components that are in development for eventual addition into the HSL, but not ready to be used in WWW.
    * Depends on: HSL
* **HSL-FB:** Components that may be promoted to HSL eventually, but are useful for some FB use-cases.
    * Depends on: HSL
* **TestInfra: **Components of our test frameworks, such as UnitTest, ExpectObj, etc.
    * Depends on: HSL, HSL-FB
* **Build** Build steps that we may want to isolate from WWW at large.
    * Depends on: HSL, HSL-FB
* **Prod:** WWW at large, anything under `flib/` but not `flib/intern/`.
    * Depends on: HSL, HSL-FB
    * Test environment: **Prod-Tests**
        * Depends on: HSL, HSL-FB, Prod, TestInfra
* **Intern:** Internal code, anything under `flib/intern/`.
    * Depends on: HSL, HSL-FB, Prod
    * Test environment: **Intern-Tests**
        * Depends on: HSL, HSL-FB, Intern, Prod, TestInfra
* **Scripts:** Scripts, anything under `scripts/`.
    * Depends on: HSL, HSL-FB, Intern, Prod

Then, our three builds would be:

* Prod: includes Prod and dependencies
* Intern: includes Intern and dependencies
* Scripts: includes Scripts and dependencies

## Other Considerations

### Incremental Typechecking

There are a few fundamental points that determine how environments will affect Hack's incremental typechecking model.

1. Environments do not create implicit namespaces. Each name in the monorepo is still globally unique, and is defined in exactly one file.
2. We already maintain a best-effort dependency graph between every file in the codebase, so that if a particular file changes, we re-typecheck it and its dependents.
3. All source files belong to exactly one environment (either in a user-defined environment or the "default" environment).

Then, _most working copy changes related to environments may be reduced down to one or more files moving from environment A to B_, in which case we re-typecheck those files and their dependents. This means that the only machinery we must add to the server is watching for environment changes, and mapping those changes to a list of files that moved environments. Consider the following situations:

* **If a file moves between environments,** that file must be re-typechecked, along with its dependent files.
* **If the name of an environment changes, **it's analogous to every file in the environment moving to the new one. Therefore, all files in that environment must be re-typechecked, along with their dependent files.
* **If an environment file is added,** it's analogous to all files in that directory moving to the newly-added environment. Therefore, all files under the directory containing the new environment file must be re-typechecked, along with their dependent files.
* **If an environment file is removed,** it's analogous to all files in that directory moving to the next-closest environment. Therefore everything under the directory containing the deleted environment file must be re-typechecked, along with their dependent files.
* **If an environment file is moved,** it's analogous to zero or more files being added and/or removed from the environment. Therefore everything under the new and old directories containing the environment file must be re-typechecked, along with their dependent files.
* **If the environment's dependencies change,** then all files in that environment must be re-typechecked, along with their dependent files.

Another concern is how the addition of environments affect saved states. TODO:

* add an environments table along with the dependencies between them
* add a reference to environment for each file in the naming table.
* Need to do performance testing for O(repo) updates to the naming table when environments change.

### Open Source

If each of our OSS libraries will be an environment internally, there is potential to integrate environment specification files with package managers. In order to do so specifications would have to be extended with at least the following information:

* Dependency version: allowing for multiple semver constraints
* Dependency source: e.g. npm, GitHub, this repo (in the case of local environments)
* Separation of production vs. development dependencies

For a specific package manager, the current frontrunner is Esy, Reason's package manager. One large factor in choosing Esy over Yarn is that it's a binary (versus Yarn requiring Node on the machine, or us having to package Node with HHVM). We'd need to implement a plugin to read HDF and make sure environment definitions are flexible enough to be used by package managers, and the developers of Esy have been open to collaborating on the design process.

A pitfall to consider is that Esy and Yarn install packages in a global cache on the system (instead of in the project root). This means that the typechecker will need to be taught about this cache in some way to still work in OSS.

## Concrete Syntax Proposal: HDF

*For a concrete proposal for environments, we’ll choose HDF. It’s consistent with HHVM’s current configuration format, and it supports dictionaries and lists. Other options considered were YAML, TOML, and JSON, but each were either inappropriate or suboptimal.*

To define environments, define named nodes inside an environments field in a file named `__environments.hdf` in a directory. The special filename isn't strictly necessary, but it makes environments easy to find within a 2.5M file codebase, and makes it harder for users to accidentally define environments. To define an environment that depends on another, simply define the environment normally add a `dependencies` field:

```
// Example: in ~/www/flib/__environments.hdf:

environments {
  ProdTests {
    regex = "#/__tests__/#"
    dependencies {
      Prod
      TestInfra
    }
  }
  Prod {
    regex = "#.*#"
  }
}

// Example: in ~/www/flib/intern/__environments.hdf:

environments {
  InternTests {
    regex = "#/__tests__/#"
    dependencies {
      Intern
      TestInfra
    }
  }
  Intern {
    regex = "#.*#"
    dependencies {
      Prod
    }
  }

```

A "build" represents a concrete subdivision of a monorepo which is composed of environments. To define builds, define named nodes inside a `builds` field inside a file named `__builds.hdf` in the root directory of the repository.

```
// Example: in ~/www/__builds.hdf:

builds {
  Prod {
    environments {
      Prod
    }
  }
  Intern {
    environments {
      Intern
    }
  }
  Scripts {
    environments {
      Scripts
    }
  }
}
```

Note that our builds evolved to be hierarchical, allowing us to only specify one environment per build, but it's not necessary. A build may contain multiple completely separate environments.

To implement runtime permeability, we use a new builtin function `HH\environment_available`, which takes a string representing an environment name as defined above. If the environment is included in the current build, it returns true, or false otherwise. The typechecker understands this function and will include the listed environment for the duration of the block. It follows that the argument passed to the function must be a string literal.

```
// Example: in some prod file:

if (HH\environment_available('Intern')) {
  // Access intern code here...
}
```

Or, we’re refining with some other mechanism (for example, being in a script implies that the Scripts environment is available), then we can assert an environment is available with `invariant` to provide a better error message.

```
// Example: in some prod file:

if (Environment::isScript()) {
  invariant(
    HH\environment_available('Scripts'),
    'Being in a script implies that Scripts is available',
  );
  // Access scripts code here...
}
```

## Prior Art

### [Cargo Workspaces](https://doc.rust-lang.org/book/ch14-03-cargo-workspaces.html) (Rust)

A *Cargo workspace* is a set of packages that share the same `Cargo.lock` and output directory. Packages within a workspace may depend on each other, and depend on the same set of external dependencies.

This is similar to this proposal in which the repo is the workspace, and each “package” is an environment. At Facebook, there is no lock file, but externally, a repo would have just one `composer.lock`, and each environment in the repo would depend on the same external dependencies.

One key difference is that there isn’t a notion of transitive dependencies (there was at some point, [but it was a bug](https://github.com/rust-lang/cargo/issues/1037)); dependencies must be declared explicitly:


> The top-level *Cargo.lock* now contains information about the dependency of `add-one` on `rand`. However, even though `rand` is used somewhere in the workspace, we can’t use it in other crates in the workspace unless we add `rand` to their *Cargo.toml* files as well.

### [Yarn Workspaces](https://yarnpkg.com/lang/en/docs/workspaces/) (JS)

The nomenclature is different here, but the concepts are similar to Cargo workspaces. A *Yarn workspace* is what Cargo would call a package (or we’d call an environment), but otherwise works similarly for interdependencies and external dependencies:


> Requiring `workspace-a` from a file located in `workspace-b` will now use the exact code currently located inside your project rather than what is published on npm, and the `cross-env` package has been correctly deduped and put at the root of your project to be used by both `workspace-a` and `workspace-b`.


Some key differences between Yarn workspaces and this proposal include:

* *“Workspaces must be descendants of the workspace root in terms of folder hierarchy. You cannot and must not reference a workspace that is located outside of this filesystem hierarchy.”*
    * In our proposal, the environments could be anywhere, as long as both the typechecker and the autoloader know about them.
* *“Nested workspaces are not supported at this time.”*
    * Our proposal does support nested environments, with the restriction that a source file may only be in exactly one environment.
* *“Be careful when publishing packages in a workspace. If you are preparing your next release and you decided to use a new dependency but forgot to declare it in the `package.json` file, your tests might still pass locally if another package already downloaded that dependency into the workspace root.”*
    * This is more than a transitive dependency problem. This means if any other workspace in the project has downloaded an external dependency, the current workspace can import it (even if it doesn’t depend on one of those other workspaces). This is a limitation of the package resolution algorithm. Our proposal requires dependencies to be either explicitly declared transitive or imported in each environment, and this will be enforced both by the typechecker and runtime.

### [Assemblies](https://docs.microsoft.com/en-us/dotnet/standard/assembly/index) (C#)

> You can think of an assembly as a collection of types and resources that form a logical unit of functionality and are built to work together. In .NET Core and .NET Framework, an assembly can be built from one or more source code files. In .NET Framework, assemblies can contain one or more modules.


Assemblies are analogous to environments in our proposal, and modules would be analogous to packages, which we may design in the future.

One key difference between assemblies and our proposal is the notion of [friend assemblies](https://docs.microsoft.com/en-us/dotnet/standard/assembly/friend-assemblies): a *friend assembly* is an assembly that can access another assembly's [internal](https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/internal) (in C# or [Friend](https://docs.microsoft.com/en-us/dotnet/visual-basic/language-reference/modifiers/friend) in Visual Basic) types and members. For V1 of this proposal, we have omitted environment-level visibility and punted until we choose to design a packages feature.

### [MODULES](https://blog.golang.org/using-go-modules) (Go)

Probably something to look into as well, because:

* it’s Google scale
* fairly new (mid-2019)
* there were mixed opinions around vendoring & dependencies before

* * *

# Design Meeting Notes

## Discussion with Dwayne (9/10)

* Permeability needs to be fleshed out more
    * There appear to be two kinds of permeability
        * 1: Explicitly introspecting on the current build to see which environments are available
        * 2: Certain files simply being included in the prod build, seems arbitrary. The proposal kind of hand-waves these away by saying we’ll use the typechecker to move them to the right environment. But why haven’t we done that already? Is there a deeper reason?
    * Is it intended to be a permanent feature?
        * If so, it should be designed like a real language feature, not a special function with a string. It should support autocomplete, jump-to-definition, etc.
        * If not, we need to deeply understand the current use-cases and discuss a 100% migration story. Possibly post in Intern/Prod Boundary Working Group to gain more specific use cases to discuss
    * Needs more code examples
        * How to break up switch cases that match on symbols within and outside of the current environment?
* How does this fit into our dependency tracking model?
    * What if environment files change? Must we re-typecheck that environment and all of its dependencies?
    * What if a file moves between environments?
    * How will this feature affect saved state (e.g. will it increase the size on disk)?
* Since we intend for this feature to be used for OSS libraries, how does this feature interact with package managers?

## Brainstorm 3 (8/19)

* Scala packages appear to be pretty close to Hack namespaces already
    * note the “agnostic to file layout”: https://docs.scala-lang.org/tour/packages-and-imports.html
    * If code is not declared in a package, it’s in the empty package (e.g. root namespace in Hack)
    * But does Scala have the problem of people injecting their code into packages?
        * Idea: Environments define which namespaces(packages) can be defined within them. If code inside another environment declares that namespace(package), raise a typechecker error
* Entering a permeability block does NOT change the current environment, it just gives access to another environment
    * This distinction is important when we have non-transitive dependencies
* TODO: Go back and revisit assumptions that may have changed due to existence of non-transitive dependencies
    * Also explicitly list use-cases for non-transitive dependencies
* Tests
    * Problem 1: We never want code in intern/prod etc. to depend on code in tests (environments problem)
    * Problem 2: We want test code to have access to namespace internals (namespaces problem

## Brainstorm 2 (8/05)

* We should find a way to model the needs of libraries with environments as well
    * Fundamentally, namespaces can be declared everywhere, so they do not fit well into notion of “packages”
        * Could namespaces be converted to a “packages” feature?
        * Scala packages appear to be pretty close to Hack namespaces already; note the “agnostic to file layout”: https://docs.scala-lang.org/tour/packages-and-imports.html
            * If code is not declared in a package, it’s in the empty package (e.g. root namespace in Hack)
    * Transitivity should be configurable (transitive by default). Or, ability to seal environments
    * Could have an internal (or export) keyword to control access to declarations within an environment
* Tests: might need some notion of “Friend” environment
    * Could replace <<__TestsBypassVisibility>> attribute
* Decision: not ready for design review
    * Want to have an explicit plan on how to support libraries
        * What about method-level granularity?
    * Should this replace namespaces?

## Brainstorm 1 (7/29)

* Research other languages
    * Scala has a relatively fine-grained visibility system
    * Research Rust crates
    * JS (package managers like Yarn)
* ✅ How to enforce environments at runtime in sandbox mode?
    * Could you construct builds at runtime?
    * Sandbox enforcement should be identical to production environment
    * Will need to integrate with autoloader and facts
        * Symbols would need to track the environment that they’re in
    * That may be an argument against is expressions, because introspecting on the current build would change this runtime enforcement!
* ✅ Are Hack classes/interfaces the right DSL to model environments? Are they markedly better than just regular config files (e.g. package.json, etc.)?
    * If we decide Hack syntax is not appropriate, what’s the alternative? HDF? TOML?
    * Maybe this is a question for design review.
    * Resolution: split proposal into abstract and concrete, will discuss syntax at design review.
* ✅ Do we REALLY need to centralize tests? Is there a middle ground between a first class feature and just regexes on file paths?
* ✅ The idea of hiding framework internals seems more like a language-level module problem (i.e. written in syntax, potentially via improved namespace support), while the problem at hand appears more to be a package management problem. Reflect that in the problem statement.
