#!/bin/bash
TRACE=sib:9 hphp/tools/hhvm_wrapper.php -c --hdf Eval.EnableIntrinsicsExtension=true hphp/test/slow/super-inlining-bros.php.disabled
