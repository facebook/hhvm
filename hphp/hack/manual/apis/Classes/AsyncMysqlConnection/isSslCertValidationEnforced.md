
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a boolean value indicating if server cert validation was enforced
for this connection




``` Hack
public function isSslCertValidationEnforced(): bool;
```




This information can be used while troubleshooting TLS handshake
failures happening on connect stage.




## Returns




+ ` bool ` - "true" if server cert validation was enforced during TLS
  handshake for this connection, "false" otherwise.
<!-- HHAPIDOC -->
