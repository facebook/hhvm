
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH\rqtrace;

function all_request_stats(): dict<string, dict<string, int>>;
```




## Returns




+ ` Dict ( [EVENT_NAME ` - => Dict
  (
  [duration] => int (microseconds)
  [count] => int
  )
  ...
  )
<!-- HHAPIDOC -->
