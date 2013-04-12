<?php

 class X {  function foo() {    return function() use(&$this) {      return $this->bar();    };  }  function bar() {}}