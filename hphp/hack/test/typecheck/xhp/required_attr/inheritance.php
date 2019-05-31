<?hh // strict

class :base { attribute int a @required; }
class :derived extends :base {}
function bar3(): :derived {
  return <derived />;
}
