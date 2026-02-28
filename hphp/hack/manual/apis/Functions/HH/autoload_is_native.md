
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return true if we're using a native autoloader




``` Hack
namespace HH;

function autoload_is_native(): bool;
```




If we are using a native autoloader, all symbols will be loaded from the
first line, and there's no need to call ` autoload_set_paths `.




If you *do* call ` autoload_set_paths ` while natively autoloading, you'll
disable the native autoloader in favor of your userland autoloader.




```
HH\autoload_is_native(); // true
HH\autoload_set_paths(darray['class' => darray[]]); // true
HH\autoload_is_native(); // false
```




## Returns




+ ` bool `
<!-- HHAPIDOC -->
