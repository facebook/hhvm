---
pageid: capabilities
title: Capabilities
layout: docs
category: Compatibility
permalink: docs/capabilities.html
redirect_from: docs/capabilities/
---

*Since 3.8*

Capability names are used to identify modules that are either conditionally
configured or that are introduced over time.

You can use the [expanded version command](/watchman/docs/cmd/version.html)
to query capabilities and avoid building knowledge of version numbers in
your client application(s).

You can use [list-capabilities](/watchman/docs/cmd/list-capabilities.html)
command to obtain a list of capabilities supported by your watchman server.

### Commands

Every command is identified by the command name prefixed by the string `cmd-`.
For example, the `watch-project` command is indicated by the capability name
`cmd-watch-project`.

### Expression Terms

Every expression term is identified by the term name prefixed by the string
`term-`.  For example, the `match` term is indicated by the capability name
`term-match`.

### File Result Fields

Every field is identified by the field name prefixed by the string `field-`.
For example, the `size` field is indicated by the capability name `field-size`.

### Feature Enhancements

Sometimes we will enhance existing functionality by adding new options to
existing commands.  Since these changes won't result in adding a new command
they won't implicitly gain a capability name.  In these cases we'll assign
an appropriate capability name by hand.

The following feature capabilities are possible / released:

Capability Name | Since version | Description
----------------|---------------|------------
`relative_root` | 3.3           | `relative_root` query option
`wildmatch`     | 3.7           | [Expanded `match` term with recursive globs](/watchman/docs/expr/match.html#wildmatch)
`suffix-set`    | 5.0           | [Expanded `suffix` to support set of suffixes](/watchman/docs/expr/suffix.html#suffixset)

