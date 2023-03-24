---
title: unsubscribe
section: Commands
---

Available starting in version 1.6

Cancels a named subscription against the specified root. The server side will no
longer generate subscription packets for the specified subscription.

```json
["unsubscribe", "/path/to/root", "mysubscriptionname"]
```
