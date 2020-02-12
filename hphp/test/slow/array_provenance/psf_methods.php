<?hh

class A {
  <<__ProvenanceSkipFrame>>
  public function foo() {
    return varray[1, 2, 3];
  }

  <<__ProvenanceSkipFrame>>
  public static function baz() {
    return darray["bond" => "james bond"];
  }
}

<<__ProvenanceSkipFrame>>
function foo() {
  return varray[1, 2, 3];
}

<<__ProvenanceSkipFrame>>
function bar() {
  return varray[1, dict[2 => varray[17]], 3];
}

<<__EntryPoint>>
function main() {
  $x = new A();
  var_dump(HH\get_provenance(foo()));
  var_dump(HH\get_provenance($x->foo()));
  var_dump(HH\get_provenance(A::baz()));
  var_dump(HH\get_provenance(bar()));
  var_dump(HH\get_provenance(bar()[1]));
  var_dump(HH\get_provenance(bar()[1][2]));
}
