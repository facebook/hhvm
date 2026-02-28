
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Specify a map containing autoload data




``` Hack
namespace HH;

function autoload_set_paths(
  KeyedContainer<string, KeyedContainer<string, string>> $map,
  string $root,
): bool;
```




The map has the form:




```
 array('class'    => array('cls' => 'cls_file.php', ...),
       'function' => array('fun' => 'fun_file.php', ...),
       'constant' => array('con' => 'con_file.php', ...),
       'type'     => array('type' => 'type_file.php', ...),
       'failure'  => callable);
```




If the 'failure' element exists, it will be called if the
lookup in the map fails, or the file cant be included. It
takes a kind ('class', 'function' or 'constant') and the
name of the entity we're trying to autoload.




If $root is non empty, it is prepended to every filename.




## Parameters




+ [` KeyedContainer<string, `](/apis/Interfaces/HH/KeyedContainer/)`` KeyedContainer<string, string>> $map ``
+ ` string $root `




## Returns




* ` Boolean ` - TRUE if successful, FALSE otherwise.
<!-- HHAPIDOC -->
