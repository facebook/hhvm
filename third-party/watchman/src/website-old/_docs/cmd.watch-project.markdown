---
pageid: cmd.watch-project
title: watch-project
layout: docs
section: Commands
permalink: docs/cmd/watch-project.html
redirect_from: docs/cmd/watch-project/
---

*Since 3.1.*

Requests that the *project* containing the requested dir is watched for
changes.  Watchman will track all files and dirs rooted at the *project* path,
and respond with the relative path difference between the *project* path and
the requested dir.

### Rationale

With a proliferation of tools that wish to take advantage of
filesystem watching at different locations in a filesystem tree, it is
possible and likely for those tools to establish multiple overlapping
watches.

Most systems have a finite limit on the number of directories that can be
watched effectively; when that limit is exceeded the performance and
reliability of filesystem watching is degraded, sometimes to the point that it
ceases to function.

It is therefore desirable to avoid this situation and consolidate the
filesystem watches.  Watchman offers the `watch-project` command to allow
clients to opt-in to the watch consolidation behavior described below.

### What's a project path?

A project is the logical root of a set of related files in a filesystem tree
and is a good point at which to consolidate watches.  Tools such as
[hgwatchman](https://bitbucket.org/facebook/hgwatchman) will most likely have
already established a watch at the root of a project, so any other tools that
wish to watch a sub-directory can do so for no additional cost if they re-use
that existing watch at a higher level in the filesystem tree.

The `watch-project` command uses a simple procedure to locate the
*project* path that corresponds to a given path.  While simple it is
rather verbose to describe it precisely:

1. The search is begun with a list of file names; we'll refer to it as
   `root_files`.  Any file in this list, if present in a directory,
   identifies that directory as being a valid project directory.
2. The search is begun with the candidate directory set to the argument
   passed to `watch-project`.  The candidate directory is passed to
   the `realpath(3)` function and the result is set as the new value
   of the candidate directory.
3. The candidate directory is concatenated with each of the `root_files`,
   one by one, and the resultant path is tested for existence.  If the
   path exists then the candidate directory is the path that will be used
   for watch and the search is halted successfully.
4. If none of the `root_files` can be found in the candidate directory
   then the parent of the candidate directory is used as a new candidate
   and the process is repeated at step 3 above.
5. If no viable candidates are found and the root of the filesystem is reached,
   then the search terminates unsuccessfully.

Watchman may perform the above search procedure twice.  The logic is:

1. `root_files` will be set to list only `.watchmanconfig`
2. Perform the search procedure above
3. If the search terminates successfully, then the watch is established
   for the current value of the candidate directory.
4. If the search terminates unsuccessfully, `root_files` is set to
   the global configuration option [root_files](../config.html#root_files)
   and the search procedure is re-run.
5. If the search terminates successfully, then the watch is established
   for the current value of the candidate directory.
6. If the global configuration option
   [enforce_root_files](../config.html#enforce_root_files) is set to true
   then the watch attempt fails.
7. Otherwise, the watch is established for the original argument to the
   `watch-project` command

What this means in laymans terms is that the definitive location  of the
project root is where the `.watchmanconfig` file is found.  If it is
not found then the set of files defined by the `root_files` configuration
is used to locate a candidate.

If no viable candidate is found then watchman will watch the requested
directory, unless the `enforce_root_files` setting is set to true.

The default value for `root_files` will match most common version control
root directories.  The default value for `enforce_root_files` is `false`.

### Using watch-project

Assuming that `~/www/.hg` and `~/www/some/child/dir` both exist, then
the command:

~~~bash
$ watchman watch-project ~/www/some/child/dir
{
  "version": "3.0.1",
  "watch": "/Users/wez/www",
  "relative_path": "some/child/dir"
}
~~~

establishes a watch on the `~/www` directory because that is the directory
that contains `.hg`, which is one of the items listed in the default value
for `root_files`.

As a client using `watch-project` it is important to observe the
`relative_path` and/or `watch` elements of the response; they identify which
directory is actually being watched.  **Any triggers, subscriptions or queries
that the client issues must be relative to the watched root to operate as
expected.**  A client can use `relative_path` to more easily construct
queries or adjust the results of queries by either concatenating the string
when composing paths in a query expression or removing the string from the
prefix when processing the results.

If `relative_path` is missing from the response it means that the requested
dir is the same as the watched dir and that the `watch-project` invocation
turned out to be exactly equivalent to a `watch` invocation for the requested
directory.

Note that, when you're using the CLI, you can specify the root as
`~/www/some/child/dir` because the shell will resolve `~/www/some/child/dir` to
`/Users/wez/www/some/child/dir`, but when you use the JSON protocol, you are
responsible for supplying an absolute path.

JSON:

~~~json
["watch", "/Users/wez/www/some/child/dir"]
~~~

### Initiating a watch

Once a viable candidate is found, if watchman is not already watching the
directory, then watchman will:

 * Establish change notification for the directory with the kernel
 * Queues up a request to crawl the directory
 * As the directory contents are resolved, those are watched in a similar
   fashion
 * All newly observed files are considered changed

### Persistence

Unless the `--no-save-state` server option was used to start the watchman
service, watches and their associated triggers are saved and re-established
across a process restart.

*Since 3.7.*

The watchman service may decide to reap watches that have been idle for an
extended period of time.  A watch is considered to be idle if no watchman
queries have been issued against the watch.  If a watch is idle, and has no
triggers registered or active subscriptions then it is a candidate for reaping.

The [idle_reap_age_seconds](../config.html#idle-reap-age-seconds) configuration
parameter controls the idle timeout for a watch.  The default is 5 days.
A reaped watch is cancelled and removed from the state file.
