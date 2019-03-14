<?php

class A {
  function good($arg) {

    ExtPregHackarrdvarrsPregECrazy::$method = 'bad';
    var_dump('good');
  }
  function bad($arg) {
    var_dump('bad');
  }
}

ExtPregHackarrdvarrsPregECrazy::$method = 'good';
preg_replace_callback('/foo/', array('A', ExtPregHackarrdvarrsPregECrazy::$method), 'foo foo');
ExtPregHackarrdvarrsPregECrazy::$method = 'good';
preg_replace_callback('/foo/', array('A', &ExtPregHackarrdvarrsPregECrazy::$method), 'foo foo');

abstract final class ExtPregHackarrdvarrsPregECrazy {
  public static $method;
}
