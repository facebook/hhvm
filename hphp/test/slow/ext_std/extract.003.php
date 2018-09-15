<?php

class A {
  function b() {
    include __DIR__."/extract.003.inc";
  }
}

<<__EntryPoint>>
function main_extract_003() {
(new A)->b();
}
