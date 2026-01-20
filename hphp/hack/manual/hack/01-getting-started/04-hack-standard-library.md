# The Hack Standard Library

HHVM is distributed with a growing set of functions and classes
collectively called the [Hack Standard Library (HSL)](/hsl/overview).
These are intended to complement, or in some cases replace previous
[built-in APIs](/apis/overview).

There are two related GitHub projects/Composer packages:
- [hhvm/hsl](https://github.com/hhvm/hsl/): The Hack Standard Library for users
  of HHVM < 4.108
- [hhvm/hsl-experimental](https://github.com/hhvm/hsl-experimental/):
  Experimental features, which may be added to the Hack Standard Library in the
  future

**Note:** Before HHVM 4.108, the Hack Standard Library was distributed
in a [separate repository](https://github.com/hhvm/hsl/) from HHVM.

## HSL Namespaces
The Hack Standard Library is grouped by namespace. For a full list, see [HSL Reference](/hsl/overview).

Also included is the static class `Ref`, with `Ref::get` and `Ref::set`, which is useful for setting reference values with [anonymous functions](/hack/functions/anonymous-functions).

## Containers and Hack Arrays
A collection of functions for working with Hack Arrays and other types of containers.

|     Namespace     | Description                                                                                                                                     |
|-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| `C\`              | Functions that operate on containers, but do not return or require a specific kind of container, such as `C\contains()`.                        |
| `Vec\`            | Functions that return, create, or operate on [the vec type](/hack/arrays-and-collections/vec-keyset-and-dict#vec), such as `Vec\drop()`.        |
| `Keyset\`         | Functions that return, create, or operate on [the keyset type](/hack/arrays-and-collections/vec-keyset-and-dict#keyset), such as `Keyset\map()`.|
| `Dict\`           | Functions that return, create, or operate on [the dict type](/hack/arrays-and-collections/vec-keyset-and-dict#dict), such as `Dict\sort()`.     |

## Strings and Regular Expressions
A collection of functions for working with strings.

|     Namespace     | Description                                                                                                                                     |
|-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| `Str\`            | Functions for interacting with [the string type](/hack/built-in-types/string), such as `Str\contains()`.                                        |
| `Regex\`          | Functions that perform [regular expression matching](/hack/built-in-types/string#working-with-regex) on `string`, such as `Regex\replace`.      |
| `Locale\`         | Functions for retrieving and creating `Locale` objects, for example: `en_US.UTF-8`.                                                             |

## Math and Random Number Generation
A collection of functions for math operations and types of randomization.

|     Namespace     | Description                                                                                                                                     |
|-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| `Math\`           | Functions for common math operations, like `Math\sqrt`, `Math\min`, and `Math\max`.                                                             |
| `PseudoRandom\`   | Functions for pseudo-randomization, focusing on performance.                                                                                    |
| `SecureRandom\`   | Functions for secure randomization, focusing on security.                                                                                       |

## HSL IO
A standard API for input and output.

|     Namespace     | Description                                                                                                                                     |
|-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| `File\`           | A library for reading, writing, and creating files, such as `File\temporary_file`.                                                              |
| `IO\`             | A library for working with asynchronous [input/output streams](/hack/getting-started/input-and-output).                                         |

### HSL IO Low-Level APIs
Other low-level APIs used by HSL IO.

|     Namespace     | Description                                                                                                                                     |
|-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| `OS\`             | APIs for creating, opening, and operating on file descriptors, like `OS\open` or `OS\pipe`.                                                     |
| `Unix\`           | APIs for Unix-based client and server interaction.                                                                                              |
| `TCP\`            | APIs for client and server interaction that uses the Transmission Control Protocol (TCP).                                                       |
| `Network\`        | APIs for client and server interaction.                                                                                                         |

## Async
In addition to the content outlined in [Asynchronous Operations](/hack/asynchronous-operations/introduction), there are a few utility classes in the `Async\` namespace.

|      Namespace    | Description                                                                                                                                     |
|-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| `Async\`          | A library for controlling asynchronous behavior, with classes like `Async\Semaphore`, `Async\Poll`, and `Async\KeyedPoll`.                      |
