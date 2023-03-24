---
title: watch
category: Commands
---

Deprecated starting in version 3.1. We recommend that clients adopt the
[watch-project](/watchman/docs/cmd/watch-project.html) command.

Requests that the specified dir is watched for changes. Watchman will track all
files and dirs rooted at the specified path.

From the command line:

```bash
$ watchman watch ~/www
```

Note that, when you're using the CLI, you can specify the root as `~/www`
because the shell will resolve `~/www` to `/home/wez/www`, but when you use the
JSON protocol, you are responsible for supplying an absolute path.

JSON:

```json
["watch", "/home/wez/www"]
```

Watchman will `realpath(3)` the directory and start watching it if it isn't
already. A newly watched directory is processed in a couple of stages:

- Establishes change notification for the directory with the kernel
- Queues up a request to crawl the directory
- As the directory contents are resolved, those are watched in a similar fashion
- All newly observed files are considered changed

Unless the `--no-save-state` server option was used to start the watchman
service, watches and their associated triggers are saved and re-established
across a process restart.

## Case-Insensitivity

Watchman has the following level of support for case-insensitive filesystems,
starting in version 2.9.9 on macOS only:

- each watched root is queried to determine if it is case-insensitive. This is
  the common default for most Mac systems running HFS+.
- When in case-insensitive mode, Watchman will attempt to resolve the true
  canonical name of a file on the filesystem when it observes changes.
- If the case of a filename changes, Watchman will report a delete of the old
  name and a change for the new name.
- Query expressions that match names will default to case-insensitive when the
  root is on a case-insensitive filesystem.
- Watchman's case folding is ASCII case-folding. Note that the `match` and
  `pcre` query expression terms request case folding support from the containing
  library, and that their case folding behavior is not controlled by Watchman
  beyond being enabled when the root is case-insensitive.
- The `path` generator is always case sensitive.
