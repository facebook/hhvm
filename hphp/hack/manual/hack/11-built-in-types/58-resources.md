A resource is a descriptor to some sort of external entity. (Examples include files, databases, and sockets.) Resources are only created or
consumed by the implementation; they are never created or consumed by Hack code. Each distinct resource has a unique ID of some unspecified form.

When scripts execute in a mode having a command-line interface, the following resources
that correspond to file streams are automatically opened at program start-up:
-   `HH\\stdin()` or `HH\\try_stdin()`, which map to standard input
-   `HH\\stdout()` or `HH\\try_stdout()`, which map to standard output
-   `HH\\stderr()` or `HH\\try_stderr()`, which map to standard error

These streams have some unspecified type, which behaves like a subtype of type `resource`.

The `try` variants return null when executed without a command-line interface,
while the non-`try` functions throw an exception.

**Resources are a carryover from PHP, and their use is discouraged in Hack code.**
