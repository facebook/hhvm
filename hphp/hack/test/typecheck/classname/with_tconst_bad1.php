<?hh // strict

interface I {}
class C implements I {}

abstract class ALoader {
  abstract const type Ti as I;
  abstract const classname<this::Ti> I_NAME;
}

class CLoader extends ALoader {
  const type Ti = stdClass;
  const classname<this::Ti> I_NAME = stdClass::class;
}
