<?php

class C {
  function foo() { include 'colon_colon_class_include_parent.inc'; }
}

<<__EntryPoint>>
function main_colon_colon_class_include_parent() {
C::foo();
}
