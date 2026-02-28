# Script Inclusion

When creating large applications or building component libraries, it is useful to be able to break up the source code into small,
manageable pieces each of which performs some specific task, and which can be shared somehow, and tested, maintained, and
deployed individually. For example, a programmer might define a series of useful constants and use them in numerous and
possibly unrelated applications. Likewise, a set of class definitions can be shared among numerous applications needing to create objects of those types.

An *include file* is a file that is suitable for *inclusion* by another file. The file doing the including is
the *including file*, while the one being included is the *included file*. A file can be either an including file or
an included file, both, or neither.

The recommended way to approach this is to [use an autoloader](/hack/getting-started/starting-a-real-project#autoloading) - however, first you need to include
the autoloader itself.

The `require_once()` directive is used for this:

```hack no-extract
namespace MyProject;

require_once(__DIR__.'/../vendor/autoload.hack');

<<__EntryPoint>>
function main(): void {
  \Facebook\AutoloadMap\initialize();
  someFunction();
}
```

The name used to specify an include file may contain an absolute or relative path; absolute paths are *strongly* recommended, using the `__DIR__` constant to resolve paths relative to the current file.

`require_once()` will raise an error if the file can not be loaded (e.g. if it is inaccessible or does not exist) , and will only load the file once, even if `require_once()` is used multiple times with the same file.

## Future Changes

We expect to make autoloading fully-automatic, and remove inclusion directives from the language.

## Legacy Issues

For relative paths, the configuration directive [`include_path`](/hhvm/configuration/INI-settings#supported-php-ini-settings) is used to resolve the include file's location.

It is currently possible (though strongly discoraged) for top-level code to exist in a file, without
being in a function. In this cases, including a file may execute code, not just import definitions.

Several additional directives exist, but are strongly discouraged:

- `require()`: like `require_once()`, but will potentially include a file multiple times
- `include()`: like `require()`, but does not raise an error if the file is inaccessible
- `include_once()`: like `require_once()`, but does not raise an error if the file is inaccessible.
