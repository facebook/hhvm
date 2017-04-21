# Suites

Tests are grouped into "suites". They are just directories. Suites can have
subdirectories if you want to group them even more. Running a suite will run
all sub-suites.

* quick - High quality high signal tests. No duplicated logic. If you aren't
  sure, your test doesn't belong here.
* slow - Slower full featured tests. Grouped into sub-suites. By default put
  your test here.
* spec - Tests associated with the official PHP language specification.
  https://github.com/php/php-langspec/
* zend/good - Passing tests from Zend's suite.
* zend/bad - Failing tests from Zend. Fix these and move them to zend/good.
* zend/flakey - Tests which mostly pass but have race conditions or can't be
  parallelized.

# Examples how to run them

* Quick suite with the JIT on -
`test/run test/quick`

* Zend tests just with the interpreter in RepoAuthoritative mode -
`test/run test/zend/good -m interp -r`

* Slow tests with the JIT in PGO mode -
`test/run test/slow -m pgo`

* PHP Specification tests with JIT -
`test/run test/spec`

* Run everything that is supposed to pass -
`test/run all`

* Run just the slow Hack typechecker tests -
`test/run --typechecker slow`

# File Layout

The format is the same as Zend's `.phpt` but instead of sections it is
separate files with the section name converted to an extension. This allows
you to easily run the `.php` file without first running the test suite.

These are the allowed extensions:

* `.php` - The source of the test.
* `.php.expect` - The exact string expected output.
* `.php.expectf` - The exact string expected output with formating characters.
* `.php.hhvm.expect` - The exact string expected output. Same as `.php.expect`.
* `.php.hhvm.expectf` - The exact string expected output with formating characters. Same as `.php.expectf`.
* `.php.typechecker.expect` - The exact string expected output for typechecker tests.
* `.php.typechecker.expectf` - The exact string expected output for typechecker tests with formatting characters.
* `.php.expectregex` - A regex that matches the output.
* `.php.in` - When you run the test, the input will be obtained from here.
* `.php.out` - When you run the test, the output will be stored here.
* `.php.opts` - Runtime options to pass to hhvm.
* `.php.cli_args` - Command line arguments to the test file (e.g., `$argv` options).
* `.php.serial` - The test will always be put in the serial bucket to be run sequentially with other serial tests so as to avoid any timing problems or collisions.
* `.php.hphp_opts` - Options passed to hphp when generating a bytecode repo.
* `.php.diff or hhas.diff` - The diff for .expect tests.
* `.hhas` - HipHop Assembly.
* `.php.norepo` - don't run the test in repo mode
* `.php.noserver` - don't run the test in server mode
* `.php.hhconfig` - A blank or syntactically valid Hack typechecker configuration file if you want the test to be able to be run in typechecker mode.
* `inc.php` - Use this extension for `require` or `include` files if you are going to have a typechecker test that uses them. For now, make sure they are in the same directory as the test. They will be copied along with the core test files when the test runner is executing.

You must have one `.php` (or `.php.type-errors` file -- see below); if you are
running tests against HHVM, you must have one and only one of `.php.expect`,
`.php.hhvm.expect`, `.php.expectf`, `.php.hhvm.expectf`, or
`.php.expectregex`; if you are running tests against the  typechecker, you
must have one and only one of `.php.typechecker.expect` or
`.php.typechecker.expectf`, and you must have a `.php.hhconfig` file as well;
and the rest are optional.

NOTE: If you are using a `.php.type-errors` file, then all the files suffixes listed in the paragraph above will include `type-errors` (e.g., `.php.type-errors.hhvm.expectf`).

NOTE: You can have both a `.php.[hhvm].expect[f]` and a 
`.php.typechecker.expect[f]`.

Any suite can have a `config.hdf` file in it that will be used. If one isn't
present, then the parent suite it checked recursively until we use
test/config.hdf.

If a suite contains an `hphpd.ini` file, all of the files in the suite will be
run with the `-m debug` and `--debug-config _dir_/hphpd.ini` switches added to
the command line. (`_dir_` will be replaced by path of the suite directory.)

Name your test in a descriptive manner and when in doubt break your test into
many files. You can use comments too so future engineers know if it is a real
breakage or they need to change the expected output.

## `.php.type-errors` Source File Suffix

The test runner will look for source code files with the `.php.type-errors`
suffix as well as `.php` files. The use case for this type of file is create a
test runner compliant source file, but to also allow the actual typechecker
`hh_client` ignore the file when run on its own. For example, in our doc repo
https://github.com/hhvm/user-documentation, we use these files to have a clean
`hh_client` run on our examples, but still can have examples showing
typechecker errors as well.

## Format Characters

These can appear in `.expectf` or `typechecker.expectf` files.

| Char    | Description                                | Regex
|---------|--------------------------------------------|-------
| %e      | Path separator                             | \/
| %s      | Any characters except newlines             | [^\r\n]+
| %S      | Optionally any characters except newlines  | [^\r\n]*
| %a      | Any characters                             | .+
| %A      | Optionally any characters                  | .*
| %w      | Optional whitespace                        | \s*
| %i      | Integer with optional sign                 | [+-]?\d+
| %d      | Digits                                     | \d+
| %x      | Hex                                        | [0-9a-fA-F]+
| %f      | Float                                      | [+-]?\.?\d+\.?\d|(?:[Ee][+-]?\d+)?
| %c      | Character                                  | .
| %r...%r | The ... is a regex                         | The part that is ...
