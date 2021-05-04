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
  an `int`; as well as aiding type safety, this prevents requests from interfering
  with resources that belong to another request.
- Avoid `inout` parameters; return tuples instead. For example, prefer
  `function mkstemp(string $pattern): (FileDescriptor, string)` to
  `function mkstemp(inout string $in_pattern_out_path): FileDescriptor`
  - this can aid for common use by allowing string literals, rather than
    requiring otherwise-unused locals
  - if the primary purpose of a function is to mutate its parameters, this
    should be considered case-by-case; for example, if we were to add something
    similar to `array_pop()`, it probably should take an `inout` container - but
    probably shouldn't be in `OS\`

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
