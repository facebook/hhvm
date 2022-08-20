---
pageid: cmd.trigger
title: trigger
layout: docs
section: Commands
permalink: docs/cmd/trigger.html
redirect_from: docs/cmd/trigger/
---

The trigger command will create or replace a trigger.

A trigger is a saved incremental query over a watched root.  When files
change that match the query expression, Watchman will spawn a process and
pass information about the changed files to it.

Triggered processes are spawned by the Watchman server process that runs in
the background; they do not have access to your terminal and their
output is redirected (by default) to the Watchman log file.

Watchman waits for the filesystem to settle before processing any
triggers, batching the list of changed files together before invoking
the registered command.  You can adjust the settle period via the
`.watchmanconfig` file.

Note that deleted files are counted as changed files and are passed the command
in exactly the same way as changed-but-existing files.

Watchman will only run a single instance of the trigger process at a time.
That avoids fork-bomb style behavior in cases where your trigger also modifies
files.  When the process terminates, watchman will re-evaluate the trigger
criteria based on the clock at the time the process was last spawned; if
a file list is generated watchman will spawn a new child with the files
that changed in the meantime.

Unless `no-save-state` is in use, triggers are saved and re-established
across a Watchman process restart.  If you had triggeres saved prior to
upgrading to Watchman 2.9.7, those triggers will be forgotten as you upgrade
past version 2.9.7; you will need to re-register them.

There are two syntaxes for registering triggers; a simple syntax that allows
very simple trigger configuration with some reasonable defaults, and a second
extended syntax which is available since Watchman version 2.9.7.

The simple syntax is implemented in terms of the extended syntax and is
preserved for backwards compatibility with older clients.

### Extended syntax

*Since 2.9.7.*

You may use the extended JSON trigger definition syntax detailed below.  It
provides more control over how the triggered commands are invoked than was
possible in earlier versions.

JSON:

~~~json
["trigger", "/path/to/dir", <triggerobj>]
~~~

Where `triggerobj` is a trigger configuration object with the fields
defined below.

Here's an example trigger specified via the CLI that will cause `make` to
be run whenever assets or sources are changed:

~~~bash
$ watchman -j <<-EOT
["trigger", "/path/to/root", {
  "name": "assets",
  "expression": ["pcre", "\.(js|css|c|cpp)$"],
  "command": ["make"]
}]
EOT
~~~

The possible trigger object properties are:

* `name` defines the name of the trigger.  You may use this name to
  remove the trigger later.  Registering a different trigger with the same name
  as an existing trigger will implicitly delete the old trigger and then
  register the new one, causing the trigger expression to be evaluated for the
  whole tree.

* `command` specifies the command to invoke.  It must be an array of string
  values; this will form the argv array of the trigger process.  When
  the trigger is spawned, the `$PATH` of the Watchman process will be used
  to locate the command.  If you have changed your `$PATH` since the Watchman
  process was started, Watchman won't be able to see your new `$PATH`.
  If you are registering trigger that runs something from an unusual or
  non-default location, it is recommended that you specify the full path
  to that command.  If you are registering a trigger script that can
  be found in the watched root, just specify the path relative to the
  root.

* `append_files` is an optional boolean parameter; if enabled, the `command`
  array will have the set of matching file names appended when the trigger
  is invoked.  System limits such as `sysconf(_SC_ARG_MAX)`
  and/or `RLIMIT_STACK` set an upper bound on the size of the parameters
  and environment that are passed to a spawned process.  Watchman will
  try to ensure that the command is runnable by keeping the number of
  file name arguments below the system limits.  If the full set cannot be
  passed to the process, Watchman will pass as many as it thinks will fit and
  omit the rest.  When this argument list truncation occurs, Watchman will
  export `WATCHMAN_FILES_OVERFLOW=true` into the environment so that the child
  process can determine that this has happened.  Watchman cannot break
  the arguments apart and run multiple processes for each argument batch;
  for that functionality, use `xargs(1)` for the `command` and set the
  `stdin` property to `NAME_PER_LINE`.

* `expression` accepts a query expression.  The expression is applied
  to the list of changed files to generate the set of files that are
  relevant to this trigger.  If no files match, the command will not be
  invoked.  Omitting the expression will match all changed files.

* `stdin` specifies how stdin should be configured for the command
  invocation.  You may set the value of this property to one of the following:

    * the string value `/dev/null` - sets stdin to read from `/dev/null`.
      This is the default and will be used if you omit the `stdin` property.

    * an array value will be interpreted as a list of field names.  When
      the command is invoked, Watchman will generate an array of JSON objects
      that contain those field names on stdin.  For example, if `stdin` is set
      to `["name", "size"]`, stdin will be a JSON array containing the list
      of changed files, represented as objects with the `name` and `size`
      properties: `[{"name": "filename.txt", "size": 123}]`.
      The list of valid fields is the same as the same as that
      documented in the `query` command.  Just as with the `query` command,
      if the field list is comprised of a single field then the JSON
      will be an array of those field values.  For instance, if you set
      `stdin` to `["name"]` the JSON will be of the form `["filename.txt"]`
      instead of `[{"name": "filename.txt"}]`.

    * the string value `NAME_PER_LINE` will cause Watchman to generate a list
      of file names on stdin, one name per line.  No quoting will be applied to
      the names, and they may contain spaces.

* `stdout` and `stderr` control the output and error streams.  If omitted,
  the corresponding stream will be inherited from the Watchman process, which
  typically means that the command output/error stream will show up in the
  Watchman log file.  If specified, the value must be a string:

    * `>path/to/file` - causes output to redirected to the specified file.
      The path is relative to the watched root, and will be truncated
      prior to being written to, if it exists, or created if it does not
      exist.

    * `>>path/to/file` - causes output to redirected to the specified file.
      The path is relative to the watched root.  If the file already exists
      then it will be appended to.  The file will be created if it does not
      exist.

* `max_files_stdin` specifies a limit on the number of files reported on
  stdin when stdin is set to hold the set of matched files.  If the number of
  files that matched exceeds this limit, the input will be truncated to match
  this limit and `WATCHMAN_FILES_OVERFLOW=true` will also be exported into the
  environment.  The default, if omitted, is no limit.

* `chdir` can be used to specify the working directory that should be set
  prior to spawning the process.  The default is to set the working directory
  to the watched root.  The value of this property is a string that will be
  interpreted relative to the watched root.  Note that changing the working dir
  does not cause the file names from the query result to be re-written: they
  will *always* be relative to the watched root.  The path to the root can
  be found in the `$WATCHMAN_ROOT` environmental variable.

### Simple syntax

The simple syntax is easier to execute from the CLI than the JSON based
extended syntax, but doesn't allow all of the trigger options to be set.
In only supports the [Simple Pattern Syntax](
/watchman/docs/simple-query.html) for queries.

From the command line:

~~~bash
$ watchman -- trigger /path/to/dir triggername [patterns] -- [cmd]
~~~

Note that the first `--` is to distinguish watchman CLI switches from the
second `--`, which delimits patterns from the trigger command.  This is only
needed when using the CLI, not when using the JSON protocol.

JSON:
~~~json
["trigger", "/path/to/dir", "triggername", <patterns>, "--", <cmd>]
~~~

For example:

~~~bash
$ watchman -- trigger ~/www jsfiles '*.js' -- ls -l
~~~

Note the single quotes around the `*.js`; if you omit them, your shell
will expand it to a list of file names and register those in the trigger.
While this would work, any `*.js` files that you add after registering the
trigger will not cause the trigger to run.

or in JSON:

~~~json
["trigger", "/home/wez/www", "jsfiles", "*.js", "--", "ls", "-l"]
~~~

The simple syntax is interpreted as a trigger object with the following
settings:

* `name` is set to the `triggername`
* `command` is set to the `<cmd>` list
* `expression` is generated from the `<patterns>` list using the rules laid
  out in [Simple Pattern Syntax](/watchman/docs/simple-query.html)
* `append_files` is set to `true`
* `stdin` is set to `["name", "exists", "new", "size", "mode"]`
* `stdout` and `stderr` will be set to output to the Watchman log file
* `max_files_stdin` will be left unset

For this simple example, if `~/www/scripts/foo.js` is changed,
watchman will chdir to `~/www` then invoke `ls -l scripts/foo.js`.  Note that
the output will show up in the Watchman log file, not in your terminal.

### Environment for trigger commands

Since Watchman version 2.9.7, the following environment variables are set
for all trigger commands, even those registered using the simple trigger
syntax:

* `WATCHMAN_FILES_OVERFLOW` is set to `true` if the number of files exceeds
  either the `max_files_stdin` limit or the system argument size limit.
* `WATCHMAN_CLOCK` is set to the current clock at the time of the trigger
  invocation
* `WATCHMAN_SINCE` is set to the clock value of the prior trigger invocation,
  or unset if this is the first trigger invocation.
* `WATCHMAN_ROOT` is set to the path to the watched root
* `WATCHMAN_TRIGGER` is set to the name of the trigger
* `WATCHMAN_SOCK` is set to the path to the Watchman socket, so that you
  can figure out how to connect back to Watchman.

### Relative roots

*Since 3.4.*

Watchman supports optionally evaluating triggers with respect to a path within a
watched root. This is used with the `relative_root` parameter:

~~~json
["trigger", "/path/to/watched/root", {
  "name": "relative-assets",
  "expression": ["pcre", "\.(js|css|c|cpp)$"],
  "command": ["make"],
  "relative_root": "project1"
}]
~~~

Setting a relative root results in the following modifications to triggers:

* Queries are evaluated with respect to the relative root. See
  [File Queries](/watchman/docs/file-query.html) for more.
* The current directory for triggered processes is set to the relative root,
  unless it is changed with `chdir`. If `chdir` is a relative path then it will
  be evaluated with respect to the relative root. So, for the example trigger
  above, if `chdir` is `"subdir2"`, the current directory for triggered `make`
  invocations is `/path/to/watched/root/project1/subdir2`.
* In the environment, `WATCHMAN_ROOT` is still set to the actual root.
* `WATCHMAN_RELATIVE_ROOT` is set to the full path of the relative root.

Relative roots behave similarly to a separate Watchman watch on the
subdirectory, without any of the system overhead that that imposes. This is
useful for large repositories, where your script or tool is only interested in a
particular directory inside the repository.
