<?hh

abstract class D {
  abstract const type P;
}
<<__EntryPoint>> function main(): void {
$rtc = new ReflectionTypeConstant('D', 'P');
var_dump($rtc->getTypeStructure());
}
