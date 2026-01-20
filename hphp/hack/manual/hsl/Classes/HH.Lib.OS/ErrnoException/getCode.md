
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

:::warning
**Deprecated:** Use [`getErrno()`](/hsl/Classes/HH.Lib.OS/ErrnoException/getErrno/) instead.
:::




Deprecated for clarity, and potential future ambiguity




``` Hack
final public function getCode(): HH\Lib\OS\Errno;
```




In the future, we may have exceptions with multiple 'codes', such as an
` errno ` and a getaddrinfo `` GAI `` constant.




Keeping logging rate at 0 so that generic code that works on any exception
stays happy.




## Returns




+ ` HH\Lib\OS\Errno `
<!-- HHAPIDOC -->
