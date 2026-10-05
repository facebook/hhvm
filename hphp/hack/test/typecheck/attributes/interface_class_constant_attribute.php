<?hh

final class ConstantAttribute implements HH\ClassConstantAttribute {
  public function __construct(int $_value)[] {}
}

interface AnnotatedConstants {
  <<ConstantAttribute(1)>>
  const string VALID = 'valid';

  <<ConstantAttribute('invalid')>>
  const string INVALID = 'invalid';
}
