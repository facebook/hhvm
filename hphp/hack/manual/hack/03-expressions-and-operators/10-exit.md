This intrinsic function terminates the current script, optionally specifying an *exit value* that is returned to the execution environment.  For example:

```Hack
exit ("Closing down\n");
exit (1);
```

If the exit value is a string, that string is written out. If the exit value is an integer, that represents the script's *exit status code*,
which must be in range 0-254. Value 255 is reserved by Hack. Value 0 represents *success*.

`exit` performs the following operations, in order:
-   Writes an optional string to standard output.
-   Calls any functions registered via the library function  [`register_shutdown_function`](http://www.php.net/register_shutdown_function)
in their order of registration.
