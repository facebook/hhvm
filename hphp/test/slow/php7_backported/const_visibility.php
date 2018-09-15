<?php

// Just testing that the parser allows this (for PHP7/symfony compat); there
// is no enforcement.

class Foo {
  const NOVIS = 'novis';
  public const PUBVIS = 'public';
  protected const PROTECTEDVIS = 'protected';
}

class Bar extends Foo {
  public function getProtected() {
    return self::PROTECTEDVIS;
  }
}


<<__EntryPoint>>
function main_const_visibility() {
var_dump(Foo::NOVIS);
var_dump(Foo::PUBVIS);
var_dump((new Bar())->getProtected());
}
