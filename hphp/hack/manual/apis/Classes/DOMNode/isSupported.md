
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks if the asked feature is supported for the specified version




``` Hack
public function isSupported(
  string $feature,
  string $version,
): bool;
```




## Parameters




+ ` string $feature ` - The feature to test. See the example of
  DOMImplementation::hasFeature() for a list of features.
+ ` string $version ` - The version number of the feature to test.




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
