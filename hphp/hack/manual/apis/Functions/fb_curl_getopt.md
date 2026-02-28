
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Gets options on the given cURL session handle




``` Hack
function fb_curl_getopt(
  resource $ch,
  int $opt = 0,
): mixed;
```




## Parameters




+ ` resource $ch ` - A cURL handle returned by curl_init().
+ ` int $opt = 0 ` - This should be one of the CURLOPT_* values.




## Returns




* ` mixed ` - - If opt is given, returns its value. Otherwise, returns an
  associative array.
<!-- HHAPIDOC -->
