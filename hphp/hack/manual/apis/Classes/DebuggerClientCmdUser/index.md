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




+ [` ->__construct() `](/apis/Classes/DebuggerClientCmdUser/__construct/)
+ [` ->addCompletion($list) `](/apis/Classes/DebuggerClientCmdUser/addCompletion/)
+ [` ->arg($index, $str) `](/apis/Classes/DebuggerClientCmdUser/arg/)
+ [` ->argCount() `](/apis/Classes/DebuggerClientCmdUser/argCount/)
+ [` ->argValue($index) `](/apis/Classes/DebuggerClientCmdUser/argValue/)
+ [` ->args() `](/apis/Classes/DebuggerClientCmdUser/args/)
+ [` ->ask($format, ...$args) `](/apis/Classes/DebuggerClientCmdUser/ask/)
+ [` ->code($source, $highlight_line = 0, $start_line_no = 0, $end_line_no = 0) `](/apis/Classes/DebuggerClientCmdUser/code/)
+ [` ->error($format, ...$args) `](/apis/Classes/DebuggerClientCmdUser/error/)
+ [` ->getCode() `](/apis/Classes/DebuggerClientCmdUser/getCode/)
+ [` ->getCommand() `](/apis/Classes/DebuggerClientCmdUser/getCommand/)
+ [` ->getCurrentLocation() `](/apis/Classes/DebuggerClientCmdUser/getCurrentLocation/)
+ [` ->getFrame() `](/apis/Classes/DebuggerClientCmdUser/getFrame/)
+ [` ->getStackTrace() `](/apis/Classes/DebuggerClientCmdUser/getStackTrace/)
+ [` ->help($format, ...$args) `](/apis/Classes/DebuggerClientCmdUser/help/)
+ [` ->helpBody($str) `](/apis/Classes/DebuggerClientCmdUser/helpBody/)
+ [` ->helpCmds($cmd, $desc, ...$args) `](/apis/Classes/DebuggerClientCmdUser/helpCmds/)
+ [` ->helpSection($str) `](/apis/Classes/DebuggerClientCmdUser/helpSection/)
+ [` ->helpTitle($str) `](/apis/Classes/DebuggerClientCmdUser/helpTitle/)
+ [` ->info($format, ...$args) `](/apis/Classes/DebuggerClientCmdUser/info/)
+ [` ->lineRest($index) `](/apis/Classes/DebuggerClientCmdUser/lineRest/)
+ [` ->output($format, ...$args) `](/apis/Classes/DebuggerClientCmdUser/output/)
+ [` ->printFrame($index) `](/apis/Classes/DebuggerClientCmdUser/printFrame/)
+ [` ->quit() `](/apis/Classes/DebuggerClientCmdUser/quit/)
+ [` ->send($cmd) `](/apis/Classes/DebuggerClientCmdUser/send/)
+ [` ->tutorial($str) `](/apis/Classes/DebuggerClientCmdUser/tutorial/)
+ [` ->wrap($str) `](/apis/Classes/DebuggerClientCmdUser/wrap/)
+ [` ->xend($cmd) `](/apis/Classes/DebuggerClientCmdUser/xend/)
<!-- HHAPIDOC -->
