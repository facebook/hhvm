
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Invokes a user handler upon calling a function or a class method




``` Hack
function fb_intercept2(
  string $name,
  mixed $handler,
): bool;
```




This
handler is expected to have signature similar to:




function intercept_handler($name, $obj, $params)




Where $name is original function's fully-qualified name ('Class::method'),
$obj is $this for an instance method call or null for static method call or
function calls and $params are original call's parameters.




This handler is expected to return a shape where if the shape contains
'value' field, then this value is returned as the result of the original
function, if the shape contains 'callback' field, then the callback is
called as the result of the original function, if the shape contains
'prepend_this' field, then 'this' or lsb class is prepended as the first
argument to the callback, if neither value nor callback is given, then the
original function is executed.




If the function does not return a shape, then a runtime exception is raised.
Signature of the callback and the original function are required to be the
same including the arity and parity of reified arguments, otherwise, the
regular error mechanism will raise an error/throw an exception accordingly.
Note that built-in functions are not interceptable.




## Parameters




+ ` string $name ` - The function or class method name to intercept. Use
  "class::method" for method name.
+ ` mixed $handler ` - Callback to handle the interception. Use null,
  false or empty string to unregister a previously registered handler. If
  name is empty, all previously registered handlers, including those that are
  set by individual function names, will be removed.




## Returns




* ` bool ` - - TRUE if successful, FALSE otherwise
<!-- HHAPIDOC -->
