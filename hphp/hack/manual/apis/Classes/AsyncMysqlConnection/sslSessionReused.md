
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether or not the current connection reused the SSL session
from another SSL connection




``` Hack
public function sslSessionReused(): bool;
```




The session is set by MySSLContextProvider.
Some cases, the server can deny the session that was set and the handshake
will create a new one, in those cases this function will return ` false `.
If this connections isn't SSL, `` false `` will be returned as well.




## Returns




+ ` bool ` - `` true `` if this is a SSL connection and the SSL session was
  reused; ``` false ``` otherwise.
<!-- HHAPIDOC -->
