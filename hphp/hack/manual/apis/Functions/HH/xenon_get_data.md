
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Gather all of the stack traces this request thread has captured by now




``` Hack
namespace HH;

function xenon_get_data(): varray<XenonSample>;
```




Does not clear the stored stacks.




## Returns




+ ` array ` - - an array of shapes with the following keys:
  'time' - unixtime when the snapshot was taken
  'stack' - stack trace formatted as debug_backtrace()
  'phpStack' - an array of shapes with the following keys:
  'function', 'file', 'line'
  'ioWaitSample' - the snapshot occurred while request was in asio scheduler




It is possible for the output of this function to change in the future.
<!-- HHAPIDOC -->
