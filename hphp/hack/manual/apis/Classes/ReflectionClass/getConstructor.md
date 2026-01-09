
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from
http://php.net/manual/en/reflectionclass.getconstructor.php )




``` Hack
public function getConstructor(): ?ReflectionMethod;
```




Gets the constructor of the reflected class.




## Returns




+ ` mixed ` - A ReflectionMethod object reflecting the class'
  constructor, or NULL if the class has no
  constructor.
<!-- HHAPIDOC -->
