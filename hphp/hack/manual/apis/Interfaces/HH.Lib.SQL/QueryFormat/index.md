---
title: QueryFormat
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
namespace HH\Lib\SQL;

interface QueryFormat implements ScalarFormat {...}
```




### Public Methods




+ [` ->format_0x25(): string `](/apis/Interfaces/HH.Lib.SQL/QueryFormat/format_0x25/)
+ [` ->format_0x3d(): ScalarFormat `](/apis/Interfaces/HH.Lib.SQL/QueryFormat/format_0x3d/)
+ [` ->format_upcase_c(string $s): string `](/apis/Interfaces/HH.Lib.SQL/QueryFormat/format_upcase_c/)
+ [` ->format_upcase_k(string $s): string `](/apis/Interfaces/HH.Lib.SQL/QueryFormat/format_upcase_k/)
+ [` ->format_upcase_l(): ListFormat `](/apis/Interfaces/HH.Lib.SQL/QueryFormat/format_upcase_l/)
+ [` ->format_upcase_q(Query $q): string `](/apis/Interfaces/HH.Lib.SQL/QueryFormat/format_upcase_q/)
+ [` ->format_upcase_t(string $s): string `](/apis/Interfaces/HH.Lib.SQL/QueryFormat/format_upcase_t/)







### Public Methods ([` HH\Lib\SQL\ScalarFormat `](/apis/Interfaces/HH.Lib.SQL/ScalarFormat/))




* [` ->format_d(?int $int): string `](/apis/Interfaces/HH.Lib.SQL/ScalarFormat/format_d/)
* [` ->format_f(?float $s): string `](/apis/Interfaces/HH.Lib.SQL/ScalarFormat/format_f/)
* [` ->format_s(?string $string): string `](/apis/Interfaces/HH.Lib.SQL/ScalarFormat/format_s/)
<!-- HHAPIDOC -->
