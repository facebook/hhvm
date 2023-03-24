---
title: version
section: Commands
---

The version command will tell you the version and build information for the
currently running watchman service:

```bash
$ watchman version
{
    "version": "2.9.6",
    "buildinfo": "git:2727d9a1e47a4a2229c65cbb2f0c7656cbd96270"
}
```

To get the version of the client:

```bash
$ watchman -v
2.9.8
```

If the server and client versions don't match up, you should probably restart
your server: `watchman shutdown-server ; watchman`.

### Capabilities

_Since 3.8._

The version command can be used to check for named capabilities. Capabilities
make it easier to check whether the server implements functionality based on the
name of that function rather than by having the client build up knowledge about
when those functions were introduced.

You can read more about the
[available capability names](/watchman/docs/capabilities.html).

To check whether the `relative_root` capability is supported:

```bash
$ watchman -j <<< '["version", {"optional":["relative_root"]}]'
{
    "version": "3.8.0",
    "capabilities": {
        "relative_root": true
    }
}
```

If the capability is not supported:

```bash
$ watchman -j <<< '["version", {"optional":["will-never-exist"]}]'
{
    "version": "3.8.0",
    "capabilities": {
        "will-never-exist": false
    }
}
```

To have the server generate an error response if a capability is not supported:

```bash
$ watchman -j <<< '["version", {"required":["will-never-exist"]}]'
{
    "version": "3.8.0",
    "capabilities": {
        "will-never-exist": false
    },
    "error": "client required capability `will-never-exist` is not supported by this server"
}
```

To require one feature and test whether some optional features are supported:

```bash
$ watchman -j <<< '["version", {"required":["term-match"],"optional":["a","b"]}]'
{
    "version": "3.8.0",
    "capabilities": {
        "a": false,
        "b": false,
        "term-match": true
    }
}
```

### capabilityCheck

The **node** and **python** clients provide a `capabilityCheck` method that will
perform the version check above, and that also provide limited support for
testing capability support against older versions of the watchman server. This
facilitates a smoother transition from version number based checks to capability
named based checks.

In _python_:

```python
import pywatchman
client = pywatchman.client()
# will throw an error if any of the required names are not supported
res = client.capabilityCheck(optional=['a'], required=['term-match'])
print res
# {'version': '3.8.0', 'capabilities': {'term-match': True, 'a': False}}
```

In _node_:

```js
var watchman = require('fb-watchman');
var client = new watchman.Client();
client.capabilityCheck({optional:['a'], required:['term-match']},
    function (error, resp) {
        if (error) {
          // error will be an Error object if any of the required named
          // are not supported
        }
        console.log(resp);
        // {'version': '3.8.0', 'capabilities': {'term-match': false, 'a': false}}
        client.end();
    });
```
