<?php

interface Foo {
}

class Bar {
  function baz(Foo $x) {
  }
}

(new Bar())->baz('herp');
