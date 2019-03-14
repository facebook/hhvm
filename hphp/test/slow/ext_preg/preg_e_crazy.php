<?php

class A {
  function good($arg) {

    ExtPregPregECrazy::$method = 'bad';
    var_dump('good');
  }
  function bad($arg) {
    var_dump('bad');
  }
}

ExtPregPregECrazy::$method = 'good';
preg_replace_callback('/foo/', array('A', ExtPregPregECrazy::$method), 'foo foo');
ExtPregPregECrazy::$method = 'good';
preg_replace_callback('/foo/', array('A', &ExtPregPregECrazy::$method), 'foo foo');

abstract final class ExtPregPregECrazy {
  public static $method;
}
