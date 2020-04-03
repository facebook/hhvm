<?hh

class Foo implements HH\ClassAttribute {}
class Bar implements HH\ClassAttribute {}

<<Foo, Bar>>
class C {}
