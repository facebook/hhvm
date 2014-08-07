## What
This directory contains sub directories, one each for each zend extension.

## Zend Extensions for Testing
These zend extensions have these requirements:
1. for testing;

2. loaded into HHVM as a DSO;

3. built independently from the main build tree, following the standard recipie for building
  a zend extension for HHVM `hphpize && cmake . && make`

It is tempting to build the DSO as part of the main root-level
all-encompassing cmake/make, but that doesn't provide an environment to
satisfy requirement (c).

As such, we'll end up running cmake recursively in each of the
subdirectories, and so you'll see the pathology of progress reports
jumping from, say, 77% of the way done to 100% done (for the simple
recursion to build the DSO), back to 77% done.  Don't fret.

## Design Alternatives

It is also tempting to build the DSO on demand for the test that
actually needs it.  The existing mechanism for this is the foo.php.skipif
file, which is evaluated by hphp/test/run as a php file.  However, the
foo.php.skipif is evaluated using the same arguments to hhvm that are used
when running the main test foo.php.  Consequently, if an foo.php.ini
file specifies the DSO to load, then evaluation of foo.php.skipif
will see the same request, prior to having a chance to build the DSO.
In other words, a dependency loop.

(As a hack to use the .skipif mechanisms, you may be tempted to make another
file aaa.php with aaa.php.skipif desribing how to build the DSO, but alas,
you can't guarantee that hphp/test/run will run aaa.php before foo.php.)
