<?hh

class Foo<-T> {}

class Bar<-T> {}

class Baz<-T> extends Foo<Bar<T>> {}
