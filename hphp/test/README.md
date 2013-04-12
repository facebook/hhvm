# Suites

Tests are grouped into "suites". They are just directories. Suites can have
subdirectories if you want to group them even more. Running a suite will run
all sub-suites.

* vm - The most common. Put your test here by default.
* zend/good - Passing tests from Zend's suite.
* zend/bad - Failing tests from Zend. Fix these and move them to zend/good.
* vm-perf - Some performance tests that aren't commonly run.

# File Layout

The format is the same as Zend's `.phpt` but instead of sections it is
separate files with the section name converted to an extension. This allows
you to easily run the .php file without first running the test suite.

These are the allowed extensions:

* .php - The source of the test.
* .expect - The exact string expected output.
* .expectf - The exact string expected output with formating characters.
* .expectregex - A regex that matches the output.
* .out - When you run the test, the output will be stored here.
* .opts - Runtime options to pass to hhvm.
* .diff - The diff for .expect tests.
* .hhas - HipHop Assembly.

You must have one `.php`; one and only one of `.expect`, `.expectf`, and
`.expectregex`; and the rest are optional.

Any suite can have a `config.hdf` file in it that will be used. If one isn't
present, then the parent suite it checked recusrivly until we use
test/config.hdf.

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
