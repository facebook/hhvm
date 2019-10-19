<?hh

interface IFoo {}

interface IBar {}

interface IBaz extends IFoo, IBar {}

class Foo implements IFoo {}

class Bar extends Foo implements IBaz {}

class Baz extends Bar {}
