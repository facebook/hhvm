<?hh

class A implements Serializable {
  public $_foo = 42;
  public function serialize() :mixed{
    return serialize(dict['a' => 'apple', 'b' => 'banana']);
  }
  public function unserialize($_) :mixed{}
}

class B {
  public $_bar = 'hurr durr';
}

class C implements JsonSerializable {
  public $_foo = 42;
  public function jsonSerialize() :mixed{
    return serialize(dict['a' => 'apple', 'b' => 'banana']);
  }
}


function attempt(mixed $in): void {
  try {
    var_dump(\HH\serialize_with_options($in, dict['disallowObjects' => true]));
  } catch (Exception $e) {
    echo "threw ". get_class($e).": ".$e->getMessage()."\n";
  }
}

 <<__EntryPoint>>
function main() :mixed{
  attempt(new A());
  attempt(new B());
  attempt(new C());
  $x = 1;
  attempt(() ==> {});
  attempt(() ==> $x);
  attempt(vec[]);
  attempt(Vector{});
  attempt(ImmVector{});
  attempt(Map{});
  attempt(ImmMap{});
  attempt(Set{});
  attempt(ImmSet{});
  attempt(Pair{1,2});
  attempt(42);
  attempt(false);
  attempt(null);
  attempt(meth_caller(A::class, 'serialize'));
  attempt(vec[new A()]);
  attempt(Vector{new B()});
}
