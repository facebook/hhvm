
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Rename a function, so that a function can be called with the new name




``` Hack
function fb_rename_function(
  string $orig_func_name,
  string $new_func_name,
): bool;
```




When writing unit tests, one may want to stub out a function. To do so,
call fb_rename_function('func_to_stub_out', 'somename') then
fb_rename_function('new_func_to_replace_with', 'func_to_stub_out'). This
way, when calling func_to_stub_out(), it will actually execute
new_func_to_replace_with().




## Parameters




+ ` string $orig_func_name ` - Which function to rename.
+ ` string $new_func_name ` - What is the new name.




## Returns




* ` bool ` - - TRUE if successful, FALSE otherwise.
<!-- HHAPIDOC -->
