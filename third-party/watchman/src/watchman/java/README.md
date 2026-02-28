Watchman Java Library
====

This provides Java bindings to the Watchman service.

Building
===

Make sure that you have [buck](https://buckbuild.com/) installed. In this
folder, run:

```
buck fetch :watchman
buck build :watchman
```

The resulting JAR file is found in:
`buck-out/gen/watchman.jar'

To run the tests:

```
buck fetch :watchman-tests
buck test :watchman-lib
```

