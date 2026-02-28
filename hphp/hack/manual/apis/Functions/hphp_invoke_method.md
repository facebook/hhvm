
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_invoke_method() - Used by ReflectionMethod to invoke a method and by
ReflectionFunction to invoke a closure




``` Hack
function hphp_invoke_method(
  object $obj,
  string $cls,
  string $name,
  Traversable $params,
): mixed;
```




## Parameters




+ ` object $obj ` - An instance of the class or null for a
  static method.
+ ` string $cls ` - The name of the class.
+ ` string $name ` - The name of the method.
+ [` Traversable `](/apis/Interfaces/HH/Traversable/)`` $params `` - The parameters to pass to the method.




## Returns




* ` mixed ` - - The result of the invoked method.
<!-- HHAPIDOC -->
