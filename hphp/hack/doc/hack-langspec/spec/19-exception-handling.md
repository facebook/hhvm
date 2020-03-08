# Exception Handling

## General

An *exception* is some unusual condition in that it is outside the
ordinary expected behavior. (Examples include dealing with situations in
which a critical resource is needed, but is unavailable, and detecting
an out-of-range value for some computation.) As such, exceptions require
special handling. This chapter describes how exceptions can be created
and handled.

Whenever some exceptional condition is detected at runtime, an exception
is *thrown*. A designated exception handler can *catch* the thrown
exception and service it. Among other things, the handler might recover
from the situation completely (allowing the script to continue
execution), it might perform some recovery and then throw an exception
to get further help, or it might perform some cleanup action and
terminate the script. Exceptions may be thrown on behalf of the Engine
or by explicit code source code in the script.

Exception handling involves the use of the following keywords:

* [`try`](11-statements.md#the-try-statement), which allows a *try-block* of code containing one or more possible exception generations, to be tried
* [`catch`](11-statements.md#the-try-statement), which defines a handler for a specific type of exception thrown from the corresponding try-block or from some function it calls
* [`finally`](11-statements.md#the-try-statement), which allows the *finally-block* of a try-block to be executed (to perform some cleanup, for example), whether or not an exception occurred within that try-block
* [`throw`](11-statements.md#the-throw-statement), which generates an exception of a given type, from a place called a *throw point*.

When an exception is thrown, an *exception object* of type [`\Exception`](19-exception-handling.md#class-exception), or of a subclass of that type, is created and made available to
the first catch-handler that can catch it. Among other things, the
exception object contains an *exception message* and an *exception
code*, both of which can be used by a handler to decide how to handle
the situation.

## Class `Exception`

Class `Exception` is the base class of all exception types. This class is
defined, as follows:

```Hack
class Exception {
  protected string $message = 'Unknown exception';
  protected int $code = 0;
  protected string $file;
  protected int $line;
  protected ?Exception $previous = null;

  public function __construct(string $message = "", int $code = 0,
    ?Exception $previous = null);

  final private function __clone(): void;

  final public function getMessage(): string;
  final public function getCode(): int;
  final public function getFile(): string;
  final public function getLine(): int;
  final public function getTrace(): array<mixed>;
  final public function getPrevious(): ?Exception;
  final public function getTraceAsString(): string;
  public function __toString(): string;
}
```

For information about exception trace-back, see [§§](19-exception-handling.md#tracing-exceptions). For information
about nested exceptions, see [§§](19-exception-handling.md#tracing-exceptions). 

The class members are defined below:

Name	| Purpose
----    | -------
`$code`	| The exception code (as provided by the constructor)
`$file`	| The name of the script where the exception was generated
`$line`	| The source line number in the script where the exception was generated
`$message`	| The exception message (as provided by the constructor)
`$previous`	| The previous exception in the chain, if this is a nested exception; otherwise, `null`
`$string`	| Work area for `__toString`
`$trace`	| Work area for function-call tracing
`__construct`	| Takes three (optional) arguments – `string`: the exception message (defaults to ""), `int`: the exception code (defaults to 0), and `\Exception`: the previous exception in the chain (defaults to `null`)
`__clone`	| Present to inhibit the cloning of exception objects
`__toString`	| Retrieves a string representation of the exception in some unspecified format
`getCode`	| Retrieves the exception code (as set by the constructor).
`getFile`	| Retrieves the name of the script where the exception was generated
`getLine`	| Retrieves the source line number in the script where the exception was generated
`getMessage`	| Retrieves the exception message
`getPrevious`	| Retrieves the previous exception (as set by the constructor), if one exists; otherwise, `null`
`getTrace`	| Retrieves the function stack trace information (see [§§](19-exception-handling.md#tracing-exceptions))
`getTraceAsString`	| Retrieves the function stack trace information formatted as a single string in some unspecified format

## Tracing Exceptions

When an exception is caught, the `get*` functions in class `\Exception`
provide useful information. If one or more nested function calls were
involved to get to the place where the exception was generated, a record
of those calls is also retained, and made available by getTrace, through
what is referred to as the *function stack trace*, or simply, `*trace*`.

Let's refer to the top level of a script as *function-level* 0.
Function-level 1 is inside any function called from function-level 0.
Function-level 2 is inside any function called from function-level 1,
and so on. The library function `getTrace` returns an array. Exceptions
generated at function-level 0 involve no function call, in which case,
the array returned by `getTrace` has zero elements.

Each element of the array returned by `getTrace` provides information
about a given function level. Let us call this array *trace-array* and
the number of elements in this array *call-level*. The key for each of
trace-array's elements has type int, and ranges from 0 to
call-level - 1. For example, when a top-level script calls function `f1`,
which calls function `f2`, which calls function `f3`, which then generates
an exception, there are four function levels, 0–3, and there are three
lots of trace information, one per call level. That is, trace-array
contains three elements, and they each correspond to the reverse order
of the function calls. For example, trace-array[0] is for the call to
function `f3`, trace-array[1] is for the call to function `f2`, and
trace-array[2] is for the call to function `f1`.

Each element in trace-array is itself an array that contains elements
with the following key/value pairs:

Key	| Value Type	| Value
--- | ----------    | -----
"args"	| `array`	| The set of arguments passed to the function
"class"	| `string` |	The name of the function's parent class
"file"	| `string` |	The name of the script where the function was called
"function"	| `string` |	The name of the function or class method
"line"	| `int` |	The line number in the source where the function was called
"object" |	`object` | The current object
"type"	| `string` |	Type of call; `->` for an instance method call, `::` for a static method call, ordinary function call, "" is returned.

As to whether extra elements with other keys are provided is
unspecified.

The key `args` has a value that is yet another array, which we shall
call *argument-array*. That array contains a set of values that
corresponds directly to the set of values passed as arguments to the
corresponding function. Regarding element order, argument-array[0]
corresponds to the left-most argument, argument-array[1] corresponds to
the next argument to the right, and so on.

Consider the case in which a function has a default argument value
defined for a parameter. If that function is called without an argument
for the parameter having the default value, no corresponding argument
exists in array-argument. Only arguments present at the function-call
site have their values recorded in array-argument. 

See also, library functions [`debug_backtrace`](http://www.php.net/debug_backtrace) and
[`debug_print_backtrace`](http://www.php.net/debug_print_backtrace).

## User-Defined Exception Classes

An exception class is defined simply by having it extend class [`\Exception`](19-exception-handling.md#class-exception). However, as that class's `__clone` method is declared [`final`](16-classes.md#methods), exception objects cannot be cloned.

When an exception class is defined, typically, its constructors call the
parent class' constructor as their first operation to ensure the
base-class part of the new object is initialized appropriately. They
often also provide an augmented implementation of
[`__toString`](http://docs.hhvm.com/manual/en/language.oop5.magic.php) 
([§§](16-classes.md#method-__tostring)).


