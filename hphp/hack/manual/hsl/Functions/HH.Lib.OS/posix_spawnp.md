
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH\Lib\OS;

function posix_spawnp(
  string $file,
  posix_spawn_file_actions_t $file_actions,
  posix_spawnattr_t $attributes,
  vec<string> $argv,
  vec<string> $envp,
): pid_t;
```




## Parameters




+ ` string $file `
+ ` posix_spawn_file_actions_t $file_actions `
+ ` posix_spawnattr_t $attributes `
+ ` vec<string> $argv `
+ ` vec<string> $envp `




## Returns




* ` pid_t `
<!-- HHAPIDOC -->
