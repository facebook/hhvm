
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Does the PHP style behaviour for casting when doing a mathematical operation




``` Hack
namespace HH\Lib\Legacy_FIXME;

function cast_for_arithmetic(
  mixed $value,
): dynamic;
```




That happens under the following situations

1) null converts to 0
1) bool converts to 0/1
1) numeric string converts to an int or double based on how the string looks.
1) non-numeric string gets converted to 0
1) resources get casted to int




## Parameters




* ` mixed $value `




## Returns




- ` dynamic `
<!-- HHAPIDOC -->
