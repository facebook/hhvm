<?hh

class Cc implements HH\ClassConstantAttribute {}

enum class SomeEnumClass {
  <<Cc>>int NOT_ALLOWED = 42;
}
