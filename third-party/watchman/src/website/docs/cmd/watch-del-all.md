---
title: watch-del-all
category: Commands
---

Available since version 3.1.1.

Removes all watches and associated triggers.

From the command line:

```bash
$ watchman watch-del-all
```

JSON:

```json
["watch-del-all"]
```

Analogous to the `watch-del` this command will remove all watches and associated
triggers from the running process, and the state file ( unless watchman service
was started with
[--no-save-state server option](cli-options.md#server-options)).
