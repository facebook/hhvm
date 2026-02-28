<?hh

class AAA {}
class BBB extends AAA {}

// These are deliberately not subtypes of AAA or BBB
class One {}
class Two extends One {}

abstract class Foo extends Bar {
  abstract const type Ta as AAA super Two;
}

abstract class Bar {
  abstract const type Ta as AAA super One;
}
