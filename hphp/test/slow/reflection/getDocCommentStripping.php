<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/**
 * Info about class A
 */
class A {

  /**
   * Info about property $a
   */
  public ?int $a;

  /**
   * Info about function a
   */
  function a(): void {}

}


<<__EntryPoint>>
function main(): void {
  $r = new ReflectionClass(A::class);
  var_dump($r->getDocComment());
  var_dump($r->getProperty('a')->getDocComment());
  var_dump($r->getMethod('a')->getDocComment());
}
