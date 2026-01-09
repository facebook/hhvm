---
title: DebuggerClientCmdUser
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
class DebuggerClientCmdUser {...}
```




### Public Methods




+ [` ->__construct() `](/docs/apis/Classes/DebuggerClientCmdUser/__construct/)
+ [` ->addCompletion($list) `](/docs/apis/Classes/DebuggerClientCmdUser/addCompletion/)
+ [` ->arg($index, $str) `](/docs/apis/Classes/DebuggerClientCmdUser/arg/)
+ [` ->argCount() `](/docs/apis/Classes/DebuggerClientCmdUser/argCount/)
+ [` ->argValue($index) `](/docs/apis/Classes/DebuggerClientCmdUser/argValue/)
+ [` ->args() `](/docs/apis/Classes/DebuggerClientCmdUser/args/)
+ [` ->ask($format, ...$args) `](/docs/apis/Classes/DebuggerClientCmdUser/ask/)
+ [` ->code($source, $highlight_line = 0, $start_line_no = 0, $end_line_no = 0) `](/docs/apis/Classes/DebuggerClientCmdUser/code/)
+ [` ->error($format, ...$args) `](/docs/apis/Classes/DebuggerClientCmdUser/error/)
+ [` ->getCode() `](/docs/apis/Classes/DebuggerClientCmdUser/getCode/)
+ [` ->getCommand() `](/docs/apis/Classes/DebuggerClientCmdUser/getCommand/)
+ [` ->getCurrentLocation() `](/docs/apis/Classes/DebuggerClientCmdUser/getCurrentLocation/)
+ [` ->getFrame() `](/docs/apis/Classes/DebuggerClientCmdUser/getFrame/)
+ [` ->getStackTrace() `](/docs/apis/Classes/DebuggerClientCmdUser/getStackTrace/)
+ [` ->help($format, ...$args) `](/docs/apis/Classes/DebuggerClientCmdUser/help/)
+ [` ->helpBody($str) `](/docs/apis/Classes/DebuggerClientCmdUser/helpBody/)
+ [` ->helpCmds($cmd, $desc, ...$args) `](/docs/apis/Classes/DebuggerClientCmdUser/helpCmds/)
+ [` ->helpSection($str) `](/docs/apis/Classes/DebuggerClientCmdUser/helpSection/)
+ [` ->helpTitle($str) `](/docs/apis/Classes/DebuggerClientCmdUser/helpTitle/)
+ [` ->info($format, ...$args) `](/docs/apis/Classes/DebuggerClientCmdUser/info/)
+ [` ->lineRest($index) `](/docs/apis/Classes/DebuggerClientCmdUser/lineRest/)
+ [` ->output($format, ...$args) `](/docs/apis/Classes/DebuggerClientCmdUser/output/)
+ [` ->printFrame($index) `](/docs/apis/Classes/DebuggerClientCmdUser/printFrame/)
+ [` ->quit() `](/docs/apis/Classes/DebuggerClientCmdUser/quit/)
+ [` ->send($cmd) `](/docs/apis/Classes/DebuggerClientCmdUser/send/)
+ [` ->tutorial($str) `](/docs/apis/Classes/DebuggerClientCmdUser/tutorial/)
+ [` ->wrap($str) `](/docs/apis/Classes/DebuggerClientCmdUser/wrap/)
+ [` ->xend($cmd) `](/docs/apis/Classes/DebuggerClientCmdUser/xend/)
<!-- HHAPIDOC -->
