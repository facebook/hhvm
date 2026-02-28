---
title: MCRouterException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Any failed MCRouter action will throw an
instance of MCRouterException




## Interface Synopsis




``` Hack
class MCRouterException extends Exception {...}
```




### Public Methods




+ [` ->__construct(string $message, int $op = MCRouter::mc_op_unknown, int $reply = MCRouter::mc_res_unknown, string $key = '') `](/apis/Classes/MCRouterException/__construct/)
+ [` ->getKey(): string `](/apis/Classes/MCRouterException/getKey/)
+ [` ->getOp(): int `](/apis/Classes/MCRouterException/getOp/)
<!-- HHAPIDOC -->
