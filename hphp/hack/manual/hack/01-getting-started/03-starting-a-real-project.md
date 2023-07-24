## Starting A Real Project

Real projects generally aren't a single file in isolation; they tend to have
dependencies such as the [Hack Standard Library], and various optional tools.

A good starting point is to:
- [install Composer]
- create an `.hhconfig` file
- create `src/` and `tests/` subdirectories
- configure autoloading
- use Composer to install the common dependencies and tools

### Autoloading

In HHVM, there is no 'build' step as such; each file is processed as needed.
Currently, HHVM needs to be given a map of what files define which classes,
functions and so on - for example, to execute the code `new Foo()`, HHVM needs
to know that the class `Foo` is defined in `src/Foo.hack`.

[hhvm-autoload] generates this map. To add it to your project, run:

```
$ php /path/to/composer.phar require hhvm/hhvm-autoload
```

hhvm-autoload needs an `hh_autoload.json` configuration
file. For most projects, a minimal example is:

```JSON
{
  "roots": [
    "src/"
  ],
  "devRoots": [
    "tests/"
  ],
  "devFailureHandler": "Facebook\\AutoloadMap\\HHClientFallbackHandler"
}
```

The "roots" key provides folders that need to be loadable in a production environment.

The "devRoots" key is for folders that you want to be autoloaded during
development or testing, but not when you are running your code in production.

The "devFailureHandler" key is the fully qualified name of a fallback strategy.
When you add a new class or function and don't run `hh-autoload`, the autoloadmap is not automatically updated.
The fallback is called when hhvm can't find your type, constant or function in the autoloadmap.

The fallback then may attempt to load the type, constant or function at runtime.
(This process will slow down your execution considerably and should therefore not used in production.)
Not all constants and functions can / will be found by HHClientFallbackHandler, see the [repository](https://github.com/hhvm/hhvm-autoload) for more details.

Once this configuration file is created, `vendor/bin/hh-autoload` can be executed
to generate or update the map, which is created as `vendor/autoload.hack`

### An Example

The following sequence of commands could be used to fully initialize a Hack
project with the most common dependencies:

```
$ curl https://raw.githubusercontent.com/hhvm/hhast/master/.hhconfig > .hhconfig
$ mkdir bin src tests
$ cat > hh_autoload.json
{
  "roots": [
    "src/"
  ],
  "devRoots": [
    "tests/"
  ],
  "devFailureHandler": "Facebook\\AutoloadMap\\HHClientFallbackHandler"
}
$ composer require hhvm/hsl hhvm/hhvm-autoload
$ composer require --dev hhvm/hhast hhvm/hacktest facebook/fbexpect
```

You may need to use the full path to Composer, depending on how it's installed.

We curl an existing hhconfig from hhast from github. The reason for this is that starting with hhvm version [4.62](https://hhvm.com/blog/2020/06/16/hhvm-4.62.html), it is no longer enough for projects that use external dependencies. Almost all packages you pull in using composer will have a suppression comment in them somewhere. You must whitelist these suppression comments in order to use these packages.

The hhast `.hhconfig` file whitelists all suppression comments used by hsl, hhvm-autoload, hacktest, fbexpect, hhast. Hhast depends on these packages itself, so this should stay up to date. If the result of `hh_client restart && hh_client` does not end with `No errors!` after the last step, please refer to the [error suppression docs](https://docs.hhvm.com/hack/silencing-errors/introduction).

The same commands with their expected output:

```
$ curl https://raw.githubusercontent.com/hhvm/hhast/master/.hhconfig > .hhconfig
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
xxx   xxx  xxx   xxx    x     x   xxxx      x --:--:-- --:--:-- --:--:--  xxxx
$ mkdir bin src tests
$ cat > hh_autoload.json
{
  "roots": [
    "src/"
  ],
  "devRoots": [
    "tests/"
  ],
  "devFailureHandler": "Facebook\\AutoloadMap\\HHClientFallbackHandler"
}
$ composer require hhvm/hsl hhvm/hhvm-autoload
Using version ^4.0 for hhvm/hsl
Using version ^2.0 for hhvm/hhvm-autoload
./composer.json has been created
Loading composer repositories with package information
Updating dependencies (including require-dev)
Package operations: 2 installs, 0 updates, 0 removals
  - Installing hhvm/hsl (v4.0.0): Loading from cache
  - Installing hhvm/hhvm-autoload (v2.0.3): Loading from cache
Writing lock file
Generating autoload files
/var/folders/3l/2yk1tgkn7xdd76bs547d9j90fcbt87/T/tmp.xaQwE1xE/vendor/autoload.hack
$ composer require --dev hhvm/hhast hhvm/hacktest facebook/fbexpect
Using version ^4.0 for hhvm/hhast
Using version ^1.4 for hhvm/hacktest
Using version ^2.5 for facebook/fbexpect
./composer.json has been updated
Loading composer repositories with package information
Updating dependencies (including require-dev)
Package operations: 7 installs, 0 updates, 0 removals
  - Installing facebook/difflib (v1.1): Loading from cache
  - Installing hhvm/hsl-experimental (v4.0.1): Loading from cache
  - Installing hhvm/type-assert (v3.3.1): Loading from cache
  - Installing facebook/hh-clilib (v2.1.0): Loading from cache
  - Installing hhvm/hhast (v4.0.4): Loading from cache
  - Installing hhvm/hacktest (v1.4): Loading from cache
  - Installing facebook/fbexpect (v2.5.1): Loading from cache
Writing lock file
Generating autoload files
/private/var/folders/3l/2yk1tgkn7xdd76bs547d9j90fcbt87/T/tmp.xaQwE1xE/vendor/autoload.hack
```

### Adding Functions or Classes

As a toy example, we're going to create a function that squares a vector of
numbers; save the following as `src/square_vec.hack`:

```Hack error
use namespace HH\Lib\Vec;

function square_vec(vec<num> $numbers): vec<int> {
  return Vec\map($numbers, $number ==> $number * $number);
}
```

If you then run `hh_client`, it will tell you of a mistake:

```
src/square_vec.hack:4:10,57: Invalid return type (Typing[4110])
  src/square_vec.hack:3:53,55: This is an int
  src/square_vec.hack:4:40,56: It is incompatible with a num (int/float) because this is the result of an arithmetic operation with a num as the first argument, and no floats.
  src/square_vec.hack:3:35,35: Here is why I think the argument is a num: this is a num
```

To fix this, change the return type of the function from `vec<int>` to `vec<num>`.

We now have a function that is valid Hack, but it's not tested, and nothing calls it.

### Adding an Executable

Save the following as `bin/square_some_things.hack`:

```Hack no-extract
#!/usr/bin/env hhvm

<<__EntryPoint>>
async function main(): Awaitable<void> {
  require_once(__DIR__.'/../vendor/autoload.hack');
  \Facebook\AutoloadMap\initialize();

  $squared = square_vec(vec[1, 2, 3, 4, 5]);
  foreach ($squared as $square) {
    printf("%d\n", $square);
  }
}
```

This program:
 - requires and initializes the autoloader so that the function we just defined
   can be found
 - calls the function
 - prints the results

You can now execute your new program, either explicitly with HHVM, or by
marking it as executable:

```
$ hhvm bin/square_some_things.hack
1
4
9
16
25
$ chmod +x bin/square_some_things.hack
$ bin/square_some_things.hack
1
4
9
16
25
```

### Linting

Most projects use a linter to enforce some stylistic choices that are not
required by the language, but help make the project consistent; [HHAST] is the
recommended linter for Hack code. HHAST's linter is enabled by an
`hhast-lint.json` file in the project root. A good starting project is to enable
all linters for all directories that contain source code - to do this, save
the following as `hhast-lint.json`:

```json
{
  "roots": [ "bin/", "src/", "tests/" ],
  "builtinLinters": "all"
}
```

When you ran `composer require` earlier, HHAST was installed into the `vendor/`
subdirectory, and can be executed from there:

```
$ vendor/bin/hhast-lint
Function "main()" does not match conventions; consider renaming to "main_async"
  Linter: Facebook\HHAST\Linters\AsyncFunctionAndMethodLinter
  Location: /private/var/folders/3l/2yk1tgkn7xdd76bs547d9j90fcbt87/T/tmp.xaQwE1xE/bin/square_some_things.hack:5:0
  Code:
  >
  ><<__EntryPoint>>
  >async function main(): Awaitable<void>
```

### Unit Testing

[HackTest] is used to create unit test classes, and [fbexpect] is used to
express assertions. Let's create a basic test as `tests/MyTest.hack`:

```hack no-extract
use function Facebook\FBExpect\expect;
use type Facebook\HackTest\{DataProvider, HackTest};

final class MyTest extends HackTest {
  public function provideSquaresExamples(): vec<(vec<num>, vec<num>)> {
    return vec[
      tuple(vec[1, 2, 3], vec[1, 4, 9]),
      tuple(vec[1.1, 2.2, 3.3], vec[1.1 * 1.1, 2.2 * 2.2, 3.3 * 3.3]),
    ];
  }

  <<DataProvider('provideSquaresExamples')>>
  public function testSquares(vec<num> $in, vec<num> $expected_output): void {
    expect(square_vec($in))->toBeSame($expected_output);
  }
}
```

We can then use HackTest to run the tests:

```
$ vendor/bin/hh-autoload
$ vendor/bin/hacktest tests/
..

Summary: 2 test(s), 2 passed, 0 failed, 0 skipped, 0 error(s).
```

Regenerating the autoloadmap (with hh-autoload) is not always required,
but if classes are not in the autoloadmap,
you may get exceptions about reflected classes not existing.
It is generally recommended to make sure that the autoloadmap is complete,
before running the test suite.

If we intentionally add a failure, such as `tuple(vec[1, 2, 3], vec[1, 2, 3])`,
HackTest reports this:

```
$ vendor/bin/hacktest tests/
..F

1) MyTest::testSquares with data set #3 (vec [
  1,
  2,
  3,
], vec [
  1,
  2,
  3,
])

Failed asserting that vec [
  1,
  4,
  9,
] is the same as vec [
  1,
  2,
  3,
]

/private/var/folders/3l/2yk1tgkn7xdd76bs547d9j90fcbt87/T/tmp.xaQwE1xE/tests/MyTest.hack(15): Facebook\FBExpect\ExpectObj->toBeSame()


Summary: 3 test(s), 2 passed, 1 failed, 0 skipped, 0 error(s).
```

## Configuring Git

The `vendor/` directory should not be committed; to make dependencies available
on another system or checkout, use `composer install`. This will use the
generated `composer.lock` file (which should generally be committed) to install
the exact same versions.

```
$ echo vendor/ > .gitignore
```

If you're creating a library, users of your library probably don't want your
unit tests - and if they have them, they will need to have `fbexpect` and
`hacktest` installed in compatible versions to not get Hack errors.

As Composer uses GitHub releases which are automatically generated via
`git export`, the simplest solution is to configure `git export` to ignore
the `tests/` directory:

```
$ echo 'tests/ export-ignore' > .gitattributes
```

## Configuring TravisCI

We recommend using Docker on TravisCI for continuous integration of Hack
projects. This is usually done by creating a separate `.travis.sh` which
executes in the container. For example, a `.travis.yml` might look like this:

```
sudo: required
language: generic
services: docker
env:
- HHVM_VERSION=latest
- HHVM_VERSION=nightly
install:
- docker pull hhvm/hhvm:$HHVM_VERSION
script:
- docker run --rm -w /var/source -v $(pwd):/var/source hhvm/hhvm:$HHVM_VERSION ./.travis.sh
```

... and a corresponding `.travis.sh`:

```
#!/bin/sh
set -ex
apt update -y
DEBIAN_FRONTEND=noninteractive apt install -y php-cli zip unzip
hhvm --version
php --version

(
  cd $(mktemp -d)
  curl https://getcomposer.org/installer | php -- --install-dir=/usr/local/bin --filename=composer
)
composer install

hh_client
vendor/bin/hacktest tests/
if !(hhvm --version | grep -q -- -dev); then
  vendor/bin/hhast-lint
fi
```

With this configuration, TravisCI runs will check for hack errors, unit test
failures - and on release builds, run `hhast-lint`. We do not run `hhast-lint`
on `-dev` builds as `hhast-lint` depends on implementation details of HHVM and
Hack which change frequently.

[fbexpect]: https://github.com/hhvm/fbexpect
[HackTest]: https://github.com/hhvm/hacktest
[HHAST]: https://github.com/hhvm/hhast
[Hack Standard Library]: https://github.com/hhvm/hsl/
[hhvm-autoload]: https://github.com/hhvm/hhvm-autoload
[install Composer]: https://getcomposer.org/doc/00-intro.md#installation-linux-unix-macos
