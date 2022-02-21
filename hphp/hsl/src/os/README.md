# `HH\Lib\OS`

The namespace is intended to contain very low-level functions, primarily as a
base layer for implementing higher-level libraries (e.g. `HH\Lib\IO`), and
direct usage should be avoided where possible.

This document is a work in progress, and consists of guidelines, not rules.
Expect us to be conservative in what is added, and we aim to expand and clarify
this document over time.

## What should be included?

- Equivalents to POSIX functions that can not be reasonably and portably
  implemented in Hack code (e.g. `open`, `write`, `socket`)
- Similar low-level functions that are portably found in libc, if there is a
  strong reason to use prefer them to POSIX functions (e.g. `mkostemps`)
- Hack-specific functions that allow interoperation between `OS\` and other
  key Hack functionality, e.g. `HH\Lib\OS\poll_async()`, allowing `await`ing
  on `HH\Lib\FileDescriptor`s.

For now, functions for use in HSL IO are high priority; it is expected that
the exposed set will grow over time.

## What should not be included?

- Unsafe functions (e.g. `malloc`)
- Functions that do not fit with HHVM/Hack's execution/request/thread models,
  such as `pthread_create`.

## API design notes

In general, we aim to make minimum changes to fit Hack; `man 2 somefunc`
(BSD and POSIX) `man 3 somefunc` (libc) should be usable as the source of
highly detailed documentation.

We recommend referring to Python's `os` and `sockets` modules; they are an
inspiration for several decisions in this library.

- throw exceptions on error. Checking `errno` is an unsafe concept in HHVM.
- if a C function's return value only indicates success or failure
  (e.g. `0` or `1`), the HSL function should return `void`, with an exception
  on failure
- do not make the errors 'nicer'; use an exception as close as practical to the
  underlying reported error, e.g. `OS\ErrnoException` or subclasses.
  - builtin code should throw an `_OS\ErrnoException`, which is final.
  - code in the HSL should use `_OS\throw_errno()` to throw an appropriate
    subclass of `OS\ErrnoException`
  - user-facing wrappers around builtins should use `_OS\wrap_impl()` to convert
    the native exceptions into the public exceptions.
- do not use `OS\ErrnoException` if the error condition would not be indicated
  by the `errno` variable in C. Consider adding another similar class, e.g.
  add `OS\HErrnoException` if you want to report an error exposed via `h_errno`
- add and use Hack classes (not type aliases) for 'handle'-like parameters and
  return values, e.g. `OS\open()` returns a `HH\Lib\FileDescriptor` instead of
  an `int`; as well as aiding type safety, this prevents requests from
  interfering with resources that belong to another request.
- Avoid `inout` parameters; return tuples instead. For example, prefer
  `function mkstemp(string $pattern): (FileDescriptor, string)` to
  `function mkstemp(inout string $in_pattern_out_path): FileDescriptor`
  - this can aid for common use by allowing string literals, rather than
    requiring otherwise-unused locals
  - if the primary purpose of a set of functions is to create, mutate and
    destroy a C data structure that acts like a value instead of a reference,
    the type, along with these utility functions, should be exposed as a `vec`
    or `shape`. See [Appendix: reference transparent C pointer
    encoding](#encoding-reference-transparent-c-pointers) section for more
    detail.
## Implementation notes

- Do as little as possible in native or other builtin code; prefer Hack code in
  the HSL wherever  possible.
- Builtin APIs should be as unopinionated/thin as possible; most API design
  decisions should only affect non-builtin Hack code.
- Do not allow requests to leak resources, such as file descriptors
- Garbage collection and reference counting should not be observable, e.g.
  unclosed FileDescriptors are closed at the end of the request, instead of
  when the object is deallocated
- Use a CLI server handler whenever the man page refers to EPERM, EAUTH,
  etc (e.g. `open()`). If a function mentions "the current process"
  (e.g. `flock()`), seek advise - this can be intrincate.
- Do not implement non-portable functionality. For example, non-portable errno
  constants are not defined, and the Linux concept of "abstract unix sockets"
  is not implemented. This is likely to be revisited in the future.
- If a function is more limited on some platforms, consider implementing the
  same limitations on all platforms to make it easier and faster to find
  and avoid non-portable code. For example, MacOS allows any number of `X`
  characters in `mkstemp` patterns, Linux requires 6. The HSL should report
  `EINVAL` for < 6 on all platforms.
- Search man pages for `EINTR` on both Linux and Mac; retrying may be unsafe
  (e.g. `close()`)

### Performance

- constructing an exception requires constructing the backtrace; it should be
  avoided for 'expected' cases. For example, if it is safe to, calls are
  retried on EINTR 5 times.
  - only do this if it is transparent - i.e. the user can't easily tell if
    the call was retried or was just slow.
- if an error will re-occur, cache the error. For example, if `poll_async()`
  fails with ENOTSUP, higher-level libraries (such as HSL `IO`) should avoid
  calling `poll_async()` on the same `FileDescriptor` again.
- if there are significant performance advantages to implementing a function
  natively, that should be evaluated case-by-case; it's currently assumed that
  the cost of leaving and re-entering the JIT outweighs benefits.
- that said, if an operation is extremely efficient and almost always wanted,
  consider doing it natively, automatically. For example, ints in HSL
  `sockaddr` are always in host byte order, not network byte order.

## Hack representation of C data types
### Cheat sheet

| C type | Hack type |
|---|---|
| `struct`s of primitive types or other `struct`s | `shape` |
| Referential transparent C pointers with unordered setters | `shape` |
| Referential transparent C pointers with ordered setters | `vec` |
| Other C handles or pointers | `HH\Lib\OS\FileDescriptor` or other wrapper classes |

### Default encoding for handles or pointers

The underlying C API commonly exposes some data type as handles or pointers,
e.g. `int filedes`, along with functions to create, manipulate, and destroy
them. By default, handles and pointers should be wrapped in HSL as Hack
classes.

### Encoding referential transparent C pointers with setters

However, as an optimization, pointers to referential transparent C `struct`s can
be encoded as a garbage-collectable mirror in Hack, which creates and destroys
the underlying C data type on demand, and destroys it when explicitly cleaned up
by a dedicated function call, or at some later undetermined time which should
not be directly observable by the same request.

Technically, the terminology "referential transparent" should be interpreted as
"referential transparent except for memory allocation" in C, while its
corresponding Hack mirror is strictly referential transparent, which means there
is no behavior change when the user replaces a variable of the referential
transparent mirror with the expression creating the variable, and vice-versa.

#### The definition of referential transparent C pointer

- A referential transparent C pointer type should be associated with a set of
utility functions, including a constructor (optional), a destructor (optional)
and several setters. These utility functions must not perform any global side
effects other than allocating memory that will be freed in the destructor.
  - Getters for the referential transparent C pointer, if any, would not used in
    HSL
- All other functions accepting a referential transparent C pointer must be not
  distinguish pointers whose addresses are different while the pointed contents
  are the same.

Referential transparent type can be used interchangeably with "value-like" type.

#### Encoding for unordered setters as a `shape`

Some C data types the setters are orthogonal to each other, and the order to
invoke setters does not matter. In that case, the data type can be encoded as a
`shape` instead of a `vec`, using the setter function name as the key in the
`shape`. For example, given the following C data type and the related utility
functions:

``` c
// The reference transparent C pointer
typedef void *posix_spawnattr_t;

// The constructor
int posix_spawnattr_init(posix_spawnattr_t *attr);

// The setters
int posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup);

int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags);

// The destructor
int posix_spawnattr_destroy(posix_spawnattr_t *attr);
```

The corresponding Hack definition should be:

``` hack
newtype pid_t = int;
type PosixSpawnFlags = ...;
type posix_spawnattr_t = shape(
  'posix_spawnattr_setpgroup' => pid_t,
  'posix_spawnattr_setflags' => PosixSpawnFlags,
);
```

#### Encoding for unordered setters as a `vec`

The general way to encode a pointer to referential transparent C `struct` is to
consider the data type as a `vec` of a setter interface implemented by setter
classes, each of which correspond to a setter function. For example, given the
following referential transparent C pointer and the related utility functions:

``` c
// The referential transparent C pointer
typedef void *posix_spawn_file_actions_t;

// The constructor
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);

// The setters
int posix_spawn_file_actions_addchdir(posix_spawn_file_actions_t *file_actions, const char *restrict path);
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions, int filedes, int newfiledes);

// The destructor
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions);
```

The corresponding Hack definition should be:

``` hack
<<__Sealed(
  posix_spawn_file_actions_addchdir::class,
  posix_spawn_file_actions_adddup2::class
)>>
interface PosixSpawnFileActionsSetter {}

final class posix_spawn_file_actions_addchdir implements PosixSpawnFileActionsSetter {
  public function __construct(public string $path) {}
}

final class posix_spawn_file_actions_adddup2 implements PosixSpawnFileActionsSetter {
  public function __construct(
    public FileDescriptor $filedes,
    public int $newfiledes,
  ) {}
}

type posix_spawn_file_actions_t = vec<PosixSpawnFileActionsSetter>;
```
