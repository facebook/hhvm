---
pageid: cmd.trigger-del
title: trigger-del
layout: docs
section: Commands
permalink: docs/cmd/trigger-del.html
redirect_from: docs/cmd/trigger-del/
---

Deletes a named trigger from the list of registered triggers.  This disables
and removes the trigger from both the in-memory and the saved state lists.

~~~bash
$ watchman trigger-del /root triggername
~~~
