#!/bin/bash

PREFIX="$(realpath ~/fbsource/fbcode)"
buck run  @mode/dev //hphp/hack/test/rust:rust_ocaml -- --lower > /tmp/lowerer.out
grep ":EQUAL:" /tmp/lowerer.out | sed "s|$PREFIX||" |  sort > ~/fbsource/fbcode/hphp/hack/test/rust/lowerer_pass
hg diff ~/fbsource/fbcode/hphp/hack/test/rust/lowerer_pass --rev .~1
