---
title: list-capabilities
category: Commands
---

_Since 3.8._

This command returns the full list of supported
[capabilities](/watchman/docs/capabilities.html) offered by the watchman server.
The intention is that client applications will use the
[expanded version command](/watchman/docs/cmd/version.html) to check
compatibility rather than interrogating the full list.

Here's some example output. The actual capabilities list is in unspecified order
and is much longer than is reproduced here:

```bash
$ watchman list-capabilities
{
    "version": "3.8.0",
    "capabilities": [
        "field-mode",
        "term-allof",
        "cmd-trigger"
    ]
}
```
