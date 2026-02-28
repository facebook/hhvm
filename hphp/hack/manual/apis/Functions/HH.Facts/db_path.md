
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the DB path corresponding to the given directory of Hack code




``` Hack
namespace HH\Facts;

function db_path(
  string $root,
): ?string;
```




The given directory must be a valid path containing a ` .hhvmconfig.hdf `
file at its root, and this `` .hhvmconfig.hdf `` file must contain either the
``` Autoload.TrustedDBPath ``` or the ```` Autoload.Query ```` setting. Otherwise, this
function will return ````` null `````.




If the ` Autoload.TrustedDBPath ` setting points to a valid path, this
function will just return that path.




Otherwise, if the ` Autoload.Query ` setting exists, this function calculates
the DB's location for that repo based on the query, path, version, and Unix
user.




## Parameters




+ ` string $root `




## Returns




* ` ?string `
<!-- HHAPIDOC -->
