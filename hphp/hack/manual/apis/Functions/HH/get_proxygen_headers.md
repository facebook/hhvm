
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Fetch all HTTP request names in the order they were received from proxygen







``` Hack
namespace HH;

function get_proxygen_headers(): vec<(string)>;
```




## Returns




+ ` array ` - - An array of all the HTTP header names in the
  current request.
  Note: if the same header name appears more than once in the request headers
  sent by client, it will appear more than once in this array.
<!-- HHAPIDOC -->
