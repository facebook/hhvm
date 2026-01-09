
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from http://php.net/manual/en/reflectionclass.getmethods.php )




``` Hack
public function getMethods(
  ?int $filter = NULL,
): varray<ReflectionMethod>;
```




Gets an array of methods for the class.




## Parameters




+ ` ?int $filter = NULL `




## Returns




* ` mixed ` - An array of ReflectionMethod objects reflecting each
  method.
<!-- HHAPIDOC -->
