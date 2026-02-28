
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Gathers the statistics of the file named by filename, like lstat(), except
uses cached information from an internal inotify-based mechanism that may
not be updated during the duration of a request




``` Hack
function fb_lazy_lstat(
  string $filename,
): mixed;
```




## Parameters




+ ` string $filename ` - Path to a file or a symbolic link.




## Returns




* ` mixed ` - - Same format as the normal php lstat() function.
<!-- HHAPIDOC -->
