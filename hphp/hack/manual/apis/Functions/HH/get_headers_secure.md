
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Fetch all HTTP request headers, including duplicates







``` Hack
namespace HH;

function get_headers_secure(): darray<string, varray<string>>;
```




## Returns




+ ` array ` - - An associative array of all the HTTP headers in the
  current request. The values in the array will be strings for uniquely
  specified headers, but arrays where a header was specified more than once.
<!-- HHAPIDOC -->
