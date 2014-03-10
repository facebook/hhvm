# Suites

Tests are grouped into "suites". They are just directories. Suites can have
subdirectories if you want to group them even more. Running a suite will run
all sub-suites.

* quick - The most common. Put your test here by default.
* slow - Slower full featured tests. Grouped into sub-suites.
* zend/good - Passing tests from Zend's suite.
* zend/bad - Failing tests from Zend. Fix these and move them to zend/good.
* vm-perf - Some performance tests that aren't commonly run.

# Examples how to run them

* Quick suite with the JIT on -
`test/run test/quick`

* Zend tests just with the interpreter in RepoAuthoritative mode -
`test/run test/zend/good -m interp -r`

* Slow tests with the JIT in IR mode -
`test/run test/slow -m hhir`

* Slow tests with the JIT, using pseudomain_wrapper.php to ensure that
  statements in global scope get jitted (may have false positives due to,
  e.g. backtraces changing) -
`test/run test/slow -m automain`

* Run everything that is supposed to pass -
`fbmake runtests`

# File Layout

The format is the same as Zend's `.phpt` but instead of sections it is
separate files with the section name converted to an extension. This allows
you to easily run the .php file without first running the test suite.

These are the allowed extensions:

* .php - The source of the test.
* .php.expect - The exact string expected output.
* .php.expectf - The exact string expected output with formating characters.
* .php.expectregex - A regex that matches the output.
* .php.in - When you run the test, the input will be obtained from here.
* .php.out - When you run the test, the output will be stored here.
* .php.opts - Runtime options to pass to hhvm.
* .php.hphp_opts - Options passed to hphp when generating a bytecode repo.
* .php.diff or hhas.diff - The diff for .expect tests.
* .hhas - HipHop Assembly.
* .php.norepo - don't run the test in repo mode

You must have one `.php`; one and only one of `.php.expect`, `.php.expectf`, and
`.php.expectregex`; and the rest are optional.

Any suite can have a `config.hdf` file in it that will be used. If one isn't
present, then the parent suite it checked recusrivly until we use
test/config.hdf.

If a suite contains an `hphpd.ini` file, all of the files in the suite will be
run with the -m debug and --debug-config _dir_/hphpd.ini switches added to the
command line. (_dir_ will be replaced by path of the suite directory.)

Name your test in a descriptive manner and when in doubt break your test into
many files. You can use comments too so future engineers know if it is a real
breakage or they need to change the expected output.

## Format Characters

These can appear in `.expectf` files.

| Char | Description                                | Regex
|------|--------------------------------------------|-------
| %e   | Path separator                             | \/
| %s   | Any characters except newlines             | [^\r\n]+
| %S   | Optionally any characters except newlines  | [^\r\n]*
| %a   | Any characters                             | .+
| %A   | Optionally any characters                  | .*
| %w   | Optional whitespace                        | \s*
| %i   | Integer with optional sign                 | [+-]?\d+
| %d   | Digits                                     | \d+
| %x   | Hex                                        | [0-9a-fA-F]+
| %f   | Float                                      | [+-]?\.?\d+\.?\d|(?:[Ee][+-]?\d+)?
| %c   | Character                                  | .
