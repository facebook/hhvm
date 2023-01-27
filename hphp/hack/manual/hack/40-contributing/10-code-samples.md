All code samples are automatically type checked and executed, to ensure they're correct.

## Basic Usage

Code samples require triple backticks, and a file name.

~~~
```example.hack
<<__EntryPoint>>
function main(): void {
  echo "Hello world!\n";
}
```
~~~

The build script will create a file of this name, type check it, run it, and include the output in the rendered HTML.

## Boilerplate

For small examples, the wrapper function can be distracting. You can use standalone statements and they will be automatically wrapped in an `<<__EntryPoint>>` function.

We can simplify the previous example to this.

~~~
```example.hack
echo "Hello world!\n";
```
~~~

## Opting Out

If you really need to write a code sample that isn't checked or executed, you can omit the file name. The code sample will be included in the docs without any checking.

~~~
```
// This example is not run at all.
// It doesn't even need to be hack code!
```
~~~

## Sharing Code

By default, each extracted example file has its own namespace based on the file name. This allows multiple examples to define the same class or function.

If you want to share code between examples, use a file name prefix followed by `.`. Namespaces are generated based on the first name component, so `foo.x.hack` and `foo.y.hack` will have the same namespace.

~~~
Here's an example class:
```example_class.definition.hack
class C {}
```
And its usage:
```example_class.usage.hack
$c = new C();
```
~~~

Namespaces include the name of the page they're on, e.g. `Hack\UserDocumentation\Fundamentals\Namespaces\Examples\Main`, so you cannot share code between different pages.

## Autoloading

HHVM requires an "autoloader" to be explicitly initialized whenever any Hack
file references definitions from another Hack file.

The build script will insert the necessary initialization code automatically
into any `<<__EntryPoint>>` function, so it is OK to rely on definitions from
other examples inside any `<<__EntryPoint>>` function or functions called by it,
**but not elsewhere**.

For example, HHVM can never successfully run a file containing e.g. a class
definition that references a parent class or other definition from another file
(this is not a limitation specific to the docs site).

~~~
```example_hierarchy.parent.hack
abstract class Parent {}
```

```example_hierarchy.child.hack
// This file will NEVER successfully run in HHVM.
final class Child extends Parent {}
```
~~~

In practice, this is fine because *running* a file containing a class definition
is generally not needed. However, it *does* mean that trying to add an
`<<__EntryPoint>>` function to `example_hierarchy.child.hack` won't work,
because HHVM will fail with an "Undefined class Parent" error before it even
reaches it.

~~~
```example_hierarchy.child.hack
// This file will NEVER successfully run in HHVM.
final class Child extends Parent {}

<<__EntryPoint>>
function main(): void {
  // This EntryPoint function is useless because HHVM will fail above.
}
```
~~~

The workaround is to put any code that depends on definitions from more than one
other example into a separate code block.

~~~
```example_hierarchy.usage.hack
$c = new Child();
```
~~~

This can also be more convenient because we can rely on the automatic
boilerplate addition by the build script, instead of manually writing the
`<<__EntryPoint>>` function header.

## Examples with Hack Errors

Examples that are expected to fail typechecking should use the `.type-errors`
extension:

~~~
```error_example.hack.type-errors
function missing_return_type() {}
```
~~~

The build script will run the Hack typechecker and include its output in the
rendered HTML (instead of HHVM runtime output).

## Supporting Files

An example code block may specify additional files to be extracted alongside the
example code using the following format:

~~~
```nondeterministic_example.hack
echo "Your lucky number is: ".\mt_rand(0, 99);
```.example.hhvm.out
Your lucky number is: 42
```.expectf
Your lucky number is: %d
```
~~~

Supported extensions are inherited from the
[HHVM test runner](https://github.com/facebook/hhvm/blob/master/hphp/test/README.md#file-layout):

- `.hhconfig` if the example requires specific *typechecker* flags
  (e.g. demonstrating a feature that is not yet enabled by default)
- `.ini` for *runtime* flags
- `.hhvm.expect` if you want to manually specify the expected output, instead of
  the build script doing it automatically
- `.hhvm.expectf` to specify the expected output using printf-style syntax, like
  in the example above
- `.expectregex` to specify the expected output using a regular expression
- `.example.hhvm.out` should contain one possible output (this will
  be included in the rendered HTML instead of the `expectf`/`expectregex` file;
  it is not needed for regular `expect` files)
- `.typechecker.expect`, `.typechecker.expectf`, `.typechecker.expectregex`,
  `.example.typechecker.out` are the same but for typechecker (Hack) output
  instead of runtime (HHVM) output; they should only be included if the example
  code is expected to fail typechecking *and* you don't want the build script to
  generate them automatically
- `.skipif` should contain valid Hack code that will print "skip" if the example
  should not be run (e.g. a MySQL example that should not run if there isn't a
  MySQL server running), otherwise print nothing


## Testing Changes

We have a test suite to ensure consistency across the changes we make to the guides, API references, and examples.

You can run it as follows:

```
$ vendor/bin/hacktest tests/
```

## Running the Examples

Nearly all of the code examples you see in the guides and API documentation are actual Hack or PHP source files that are embedded at site build time into the content itself.

As opposed to embedded the code examples directly within the markdown itself, this provides the flexibility of actually having running examples within this repo.

You must have HHVM installed in order to run these examples since most of them are written in Hack (e.g., `<?hh`), and HHVM is the only runtime to currently support Hack.

You will find the examples in directories named with the pattern:

```
guides/[hhvm | hack]/##-topic/##-subtopic-examples
```

e.g.,

```
$ guides/hack/23-collections/06-constructing-examples
```

### Standalone

You can run any example standalone. For example:

```
# Assuming you are in the user-documentation repo directory
$ cd guides/hack/23-collections/10-examples-examples/
$ hhvm lazy.php
```

And you will see output like:

```
object(HH\Vector)#4 (5) {
  [0]=>
  int(0)
  [1]=>
  int(2)
  [2]=>
  int(4)
  [3]=>
  int(6)
  [4]=>
  int(8)
}
Time non lazy: 0.10859489440918
object(HH\Vector)#10 (5) {
  [0]=>
  int(0)
  [1]=>
  int(2)
  [2]=>
  int(4)
  [3]=>
  int(6)
  [4]=>
  int(8)
}
Time non lazy: 0.0096559524536133
```

### Using the HHVM Test Runner

Each example is structured to be run with the [HHVM test runner](https://github.com/facebook/hhvm/blob/master/hphp/test/README.md). We use the test runner internally to ensure that any changes made to HHVM do not cause a regression. The examples in the documentation here can be used for that purpose as well.

You can run the HHVM test runner on the entire suite of examples, on one directory of examples or just one example itself.

*Normally you will use our test suite described [above](#testing-changes) to test any changes you make (because it tests our examples as well). However, sometimes it is actually faster and more explicit to test one example directly with the HHVM test runner.*

```
# Assuming you are in the user-documentation repo root

# This runs every example in the test runner.
# Won't normally need to do this; just use our test suite instead.

# Test with the typechecker
$ api-sources/hhvm/hphp/test/run --hhserver-binary-path $(which hh_server) --typechecker guides/hack/05-statements/
# Test with the runtime
$ api-sources/hhvm/hphp/test/run --hhvm-binary-path $(which hhvm) guides/hack/05-statements/
```

Here is the output you should see when you run the test runner. Assume we are running the examples in the collections topic:

```
$ hhvm api-sources/hhvm/hphp/test/run guides/hack/23-collections/
Running 32 tests in 32 threads (0 in serial)

All tests passed.
              |    |    |
             )_)  )_)  )_)
            )___))___))___)\
           )____)____)_____)\
         _____|____|____|____\\__
---------\      SHIP IT      /---------
  ^^^^^ ^^^^^^^^^^^^^^^^^^^^^
    ^^^^      ^^^^     ^^^    ^^
         ^^^^      ^^^

Total time for all executed tests as run: 11.57s
```

You can use `--verbose` to see all the tests that are running.
