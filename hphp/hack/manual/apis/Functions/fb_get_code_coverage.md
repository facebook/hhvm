
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns code coverage data collected so far




``` Hack
function fb_get_code_coverage(
  bool $flush,
): mixed;
```




Turn on code coverage by
Eval.RecordCodeCoverage or by using fb_enable_code_coverage and call this
function periodically to get results. Eval.CodeCoverageOutputFile allows
you to specify an output file to store results at end of a script run from
command line. Use this function in server mode to collect results instead.




## Parameters




+ ` bool $flush ` - Whether to clear data after this function call.




## Returns




* ` darray<string,mixed>|false `
<!-- HHAPIDOC -->
