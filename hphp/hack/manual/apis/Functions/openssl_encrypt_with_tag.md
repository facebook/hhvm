
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
function openssl_encrypt_with_tag(
  string $data,
  string $method,
  string $password,
  int $options,
  string $iv,
  inout mixed $tag_out,
  string $aad = '',
  int $tag_length = 16,
): mixed;
```




## Parameters




+ ` string $data `
+ ` string $method `
+ ` string $password `
+ ` int $options `
+ ` string $iv `
+ ` inout mixed $tag_out `
+ ` string $aad = '' `
+ ` int $tag_length = 16 `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
