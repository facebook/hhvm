<?hh

class Foo<+T> {}

interface Bar<-T> {}

class Baz<+T> extends Foo<T> implements Bar<T> {}
