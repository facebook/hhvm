
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from
http://php.net/manual/en/reflectionclass.getdefaultproperties.php )




``` Hack
public function getDefaultProperties(): darray<string, mixed>;
```




Gets default properties from a class (including inherited properties).




This method only works for static properties when the default value is
known statically. If it is not known statically you will get null instead
of the correct value. Do not rely on this API for default values of
static properties.




## Returns




+ ` mixed ` - An array of default properties, with the key being
  the name of the property and the value being the
  default value of the property or NULL if the
  property doesn't have a default value. The function
  does not distinguish between static and non static
  properties and does not take visibility modifiers
  into account.
<!-- HHAPIDOC -->
