A resource is a descriptor to some sort of external entity. (Examples include files, databases, and sockets.) Resources are only created or
consumed by the implementation; they are never created or consumed by Hack code. Each distinct resource has a unique ID of some unspecified form.

When scripts execute in a mode having a command-line interface, the following predefined resource-like constants that correspond to file
streams are automatically opened at program start-up:
-   `STDIN`, which maps to standard input
-   `STDOUT`, which maps to standard output
-   `STDERR`, which maps to standard error

These constants have some unspecified type, which behaves like a subtype of type `resource`.

**Resources are a carryover from PHP, and their use is discouraged in Hack code.**
