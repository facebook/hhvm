---
title: PlainSprintf
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This type has some magic behavior: whenever it appears as a
function parameter (in a function with varargs), the argument must
be a static string, and will be parsed for % formatting specifiers
(which will determine the type of the varargs)




T is treated as a state machine. After the first %, each character
causes the corresponding method in T to be looked up. For example,
'%b' will "call" the method




function format_b(int $s) : string;




and consume an 'int' from the argument list.




Hex escapes are used for non-alphabetic characters. The '%%'
pseudo-specifier consumes nothing and appears as




```
function format_0x25() : string;
```




Modifiers and multi-char entries can be encoded by return a new
formatter instead of a string:




```
function format_upcase_l() : ListFormatter;
function format_0x2a(int $s) : PaddingFormatter;
```




Note that you *could* use an actual instance of T to do the
formatting. We don't; T is only here to provide the types.




For another example on how to implement your own format string interface,
see \\HH\\Lib\\Str\\SprintfFormat in the HSL.




## Interface Synopsis




``` Hack
interface PlainSprintf {...}
```




### Public Methods




+ [` ->format_0x20(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x20/)
+ [` ->format_0x25(): string `](/docs/apis/Interfaces/PlainSprintf/format_0x25/)
+ [` ->format_0x27(): SprintfQuote `](/docs/apis/Interfaces/PlainSprintf/format_0x27/)
+ [` ->format_0x2b(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x2b/)
+ [` ->format_0x2d(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x2d/)
+ [` ->format_0x2e(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x2e/)
+ [` ->format_0x30(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x30/)
+ [` ->format_0x31(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x31/)
+ [` ->format_0x32(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x32/)
+ [` ->format_0x33(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x33/)
+ [` ->format_0x34(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x34/)
+ [` ->format_0x35(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x35/)
+ [` ->format_0x36(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x36/)
+ [` ->format_0x37(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x37/)
+ [` ->format_0x38(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x38/)
+ [` ->format_0x39(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_0x39/)
+ [` ->format_b(int $s): string `](/docs/apis/Interfaces/PlainSprintf/format_b/)
+ [` ->format_c(?int $s): string `](/docs/apis/Interfaces/PlainSprintf/format_c/)
+ [` ->format_d(mixed $s): string `](/docs/apis/Interfaces/PlainSprintf/format_d/)
+ [` ->format_e(?float $s): string `](/docs/apis/Interfaces/PlainSprintf/format_e/)
+ [` ->format_f(mixed $s): string `](/docs/apis/Interfaces/PlainSprintf/format_f/)
+ [` ->format_g(?float $s): string `](/docs/apis/Interfaces/PlainSprintf/format_g/)
+ [` ->format_l(): PlainSprintf `](/docs/apis/Interfaces/PlainSprintf/format_l/)
+ [` ->format_o(?int $s): string `](/docs/apis/Interfaces/PlainSprintf/format_o/)
+ [` ->format_s(?arraykey $s): string `](/docs/apis/Interfaces/PlainSprintf/format_s/)
+ [` ->format_u(?int $s): string `](/docs/apis/Interfaces/PlainSprintf/format_u/)
+ [` ->format_upcase_e(?float $s): string `](/docs/apis/Interfaces/PlainSprintf/format_upcase_e/)
+ [` ->format_upcase_f(?float $s): string `](/docs/apis/Interfaces/PlainSprintf/format_upcase_f/)
+ [` ->format_upcase_x(?int $s): string `](/docs/apis/Interfaces/PlainSprintf/format_upcase_x/)
+ [` ->format_x(mixed $s): string `](/docs/apis/Interfaces/PlainSprintf/format_x/)
<!-- HHAPIDOC -->
