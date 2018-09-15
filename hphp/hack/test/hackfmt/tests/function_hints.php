<?hh

class Foooooooooooo {}
class Baaaaaaaaaaar {}

function f((function (Foooooooooooo, Baaaaaaaaaaar, Foooooooooooo, Baaaaaaaaaaar): Foooooooooooo) $x): void {}

function g((function (    ...   ) :Foooooooooooo) $x): void{}

function h((function (Foooooooooooo   ...) :Foooooooooooo) $x): void{}

function i((function (Baaaaaaaaaaar, Baaaaaaaaaaar, Foooooooooooo...
  ) :Foooooooooooo) $x): void{}

function j((function (Baaaaaaaaaaar, Baaaaaaaaaaar, ...) :Foooooooooooo) $x): void{}
