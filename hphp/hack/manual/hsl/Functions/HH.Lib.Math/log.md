
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the logarithm base ` $base ` of `` $arg ``




``` Hack
namespace HH\Lib\Math;

function log(
  num $arg,
  ?num $base = NULL,
): float;
```




For the exponential function, see ` Math\exp() `.




## Parameters




+ ` num $arg `
+ ` ?num $base = NULL `




## Returns




* ` float `




## Examples




```
$x = 10;
$log = Math\log($x);
echo "Log is $log \n"; // Output: Log is 2.302585092994
```
<!-- HHAPIDOC -->
