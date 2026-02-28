---
title: get-config
category: Commands
---

The `get-config` command returns the `.watchmanconfig` for the root. If there is
no `.watchmanconfig`, it returns an empty configuration field:

```bash
$ watchman get-config .
{
    "version": "2.9.9",
    "config": {}
}
```

```bash
$ watchman get-config /path/to/root
{
    "version": "2.9.9",
    "config": {
        "ignore_dirs": [
            "buck-out"
        ]
    }
}
```

Note that watchman only reads the `.watchmanconfig` file when the watch is
established. If changes are made after that point, the `get-config` response
will not reflect them.

See [Configuration Options](../config.md#configuration-options) for details on
valid contents of the `config` field. Note that the values returned by
`get-config` are passed straight through from the `.watchmanconfig` file, and
thus may contain fields that are not strictly legal.

This command is available since watchman version 2.9.9.
