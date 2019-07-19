<?hh

class :test {
  attribute int id @required;
  attribute string name @lateinit;
}

@@Attr
function f(): void {}

@Attr @
function g(): void {}
