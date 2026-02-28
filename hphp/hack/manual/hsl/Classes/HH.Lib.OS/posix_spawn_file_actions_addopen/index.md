---
title: posix_spawn_file_actions_addopen
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
namespace HH\Lib\OS;

final class posix_spawn_file_actions_addopen extends \HH\Lib\_Private\_OS\PosixSpawnFileActionDecorator implements PosixSpawnFileActionsSetter {...}
```




### Public Methods




+ [` ->__construct(int $filedes, string $path, int $oflag, int $mode) `](/hsl/Classes/HH.Lib.OS/posix_spawn_file_actions_addopen/__construct/)







### Protected Methods




* [` ->forkAndExecveDecorator(\HH\Lib\_Private\_OS\TForkAndExecve $fork_and_execve): \HH\Lib\_Private\_OS\TForkAndExecve `](/hsl/Classes/HH.Lib.OS/posix_spawn_file_actions_addopen/forkAndExecveDecorator/)
<!-- HHAPIDOC -->
