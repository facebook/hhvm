<?hh // strict

abstract class D {
  abstract const type P;
}

$rtc = new ReflectionTypeConstant('D', 'P');
var_dump($rtc->getTypeStructure());
