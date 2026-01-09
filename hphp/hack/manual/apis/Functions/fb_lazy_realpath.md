
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a canonicalized version of the input path that contains no symbolic
links, like realpath(), except uses cached information from an internal
inotify-based mechanism that may not be updated during the duration of a
request




``` Hack
function fb_lazy_realpath(
  string $filename,
): mixed;
```




## Parameters




+ ` string $filename ` - Fake path to the file.




## Returns




* ` string ` - - Real path of the file.
<!-- HHAPIDOC -->
