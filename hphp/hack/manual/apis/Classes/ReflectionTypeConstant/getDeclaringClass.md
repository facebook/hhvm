
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Gets the declaring class for the reflected type constant




``` Hack
public function getDeclaringClass(): ReflectionClass;
```




This is
the most derived class in which the type constant is declared.




## Returns




+ [` ReflectionClass `](/apis/Classes/ReflectionClass/) - A ReflectionClass object of the class that the
  reflected type constant is part of.
<!-- HHAPIDOC -->
