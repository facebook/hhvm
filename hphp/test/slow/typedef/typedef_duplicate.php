<?hh

class MyClass {}
class OtherClass {}

type MyType = MyClass;
type MyType = MyClass; //ok

// raise a fatal:
type MyType = OtherClass;
