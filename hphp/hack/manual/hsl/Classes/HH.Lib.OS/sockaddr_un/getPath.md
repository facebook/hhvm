
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the path (if any) of a socket




``` Hack
final public function getPath(): ?string;
```




## Returns




+ ` null ` - if the socket does not have a path, for example, if created
  with `` socketpair() ``
+ ` a ` - `` string `` if the socket does have a path; this is usually - but
  not always - a filesystem path. For example, Linux supports 'abstract'
  unix sockets, which have a path beginning with a null byte and do not
  correspond to the filesystem.
<!-- HHAPIDOC -->
