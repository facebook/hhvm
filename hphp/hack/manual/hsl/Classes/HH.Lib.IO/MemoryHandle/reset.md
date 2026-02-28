
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Set the internal buffer and reset position to the beginning of the file




``` Hack
public function reset(
  string $data = '',
): void;
```




If you wish to preserve the position, use [` tell() `](/hsl/Classes/HH.Lib.IO/MemoryHandle/tell/) and [` seek() `](/hsl/Classes/HH.Lib.IO/MemoryHandle/seek/),
or [` appendToBuffer() `](/hsl/Classes/HH.Lib.IO/MemoryHandle/appendToBuffer/).




## Parameters




+ ` string $data = '' `




## Returns




* ` void `
<!-- HHAPIDOC -->
