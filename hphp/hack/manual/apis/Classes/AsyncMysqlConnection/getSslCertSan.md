
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns Server Alternative Names attribute of the TLS certificate
presented by MySQL server




``` Hack
public function getSslCertSan(): Vector<string>;
```




This information can be used while troubleshooting TLS handshake
failures happening on connect stage.




## Returns




+ [` Vector<string> `](/apis/Classes/HH/Vector/) - a vector of strings containing a collection of Server
  Alternative Names values from the server certificate presented
  by MySQL.
<!-- HHAPIDOC -->
