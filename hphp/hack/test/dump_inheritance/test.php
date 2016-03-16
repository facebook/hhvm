<?hh

interface IFoo {}

interface IBar {}

interface IQux extends IFoo, IBar {}

class Foo implements IFoo {}

class Bar extends Foo, Bar implements IQux {}

class Baz extends Bar {}
