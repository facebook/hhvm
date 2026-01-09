
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Ends an artificial frame that xhprof_frame_begin() started




``` Hack
function xhprof_frame_end(): void;
```




One has to make
sure there are no exceptions in between these two calls, as otherwise, it
may report incorrect timings. Also, xhprof_frame_begin() and
xhprof_frame_end() have to be paired up really well, so not to interfere
with regular function's profiling, unless that's the intention.




## Returns




+ ` void `
<!-- HHAPIDOC -->
