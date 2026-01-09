
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retreive a record and its metadata







``` Hack
public function gets(
  string $key,
): Awaitable<shape('value' => string, 'cas' => int, 'flags' => int), darray>;
```




## Parameters




+ ` string $key ` = Name of the key to retreive




## Returns




* ` array ` - - Value retreived and additional metadata
  array(
  'value' => 'Value retreived',
  'cas'   => 1234567890,
  'flags' => 0x12345678,
  )
<!-- HHAPIDOC -->
