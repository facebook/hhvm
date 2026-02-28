
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the address family of the socket




``` Hack
abstract public function getFamily(): HH\Lib\OS\AddressFamily;
```




It may be more useful to check the type of the sockaddr object instead,
e.g.




```
- if ($sa->getFamily() === OS\AddressFamily::AF_UNIX) {
+ if ($sa is OS\sockaddr_un) {
```




## Returns




+ ` HH\Lib\OS\AddressFamily `
<!-- HHAPIDOC -->
