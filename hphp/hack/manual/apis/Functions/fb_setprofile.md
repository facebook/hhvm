
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Set a callback function to be called whenever a function is entered or
exited




``` Hack
function fb_setprofile(
  mixed $callback,
  int $flags = SETPROFILE_FLAGS_DEFAULT,
  vec<string> $functions = vec [
],
): void;
```




Takes 3 args, the function name, the mode (enter or exit), and an
array describing the frame.




## Parameters




+ ` mixed $callback ` - Profiler function to call or null to disable
+ ` int $flags = SETPROFILE_FLAGS_DEFAULT ` - Controls when it should get called back and with what
+ ` vec<string> $functions = vec [ ] ` - Only receive callbacks on these functions.
  In effect only when it's not empty.




## Returns




* ` void `
<!-- HHAPIDOC -->
