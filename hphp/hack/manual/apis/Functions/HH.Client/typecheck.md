
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Typecheck the currently running endpoint with a given ` hh_client `




``` Hack
namespace HH\Client;

function typecheck(
  string $client_name = 'hh_client',
): TypecheckResult;
```




Does some
caching to hopefully be pretty cheap to call, especially when there are no
errors and the code isn't changing. Relies on ` hh_server ` to poke a stamp
file to say "something has changed" to invalidate our cache.




TODO Areas for future improvement:

+ Key the cache by endpoint/hhconfig location, so that we correctly support
  more than one project per HHVM instance.
+ Populate the cache separately from this function, so that
  typecheck_and_error can have a hot path that just checks "is the world
  clean" without paying the apc_fetch deserialization cost (which is most
  of the cost of this function, if I'm benchmarking correctly).
+ Storing an array (instead of an object) in APC might be faster, due to the
  way I think HHVM can optimize COW arrays.




## Parameters




* ` string $client_name = 'hh_client' `




## Returns




- ` TypecheckResult `
<!-- HHAPIDOC -->
