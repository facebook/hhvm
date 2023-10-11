<?hh

class Inv<T> {
  public function __construct(public T $item) { }
}

class Cov<+T> {
  public function __construct(private T $item) { }
}

function make<T>(T $x):Cov<T> {
  return new Cov($x);
}

function test(): Cov<mixed> {
  $i = (new Inv("a"))->item;
  // At this point we have
  //   $i : #1
  //   string <: #1
  // Now we generate another fresh variable, say #2
  //   #1 <: #2
  // And to maintain transitive closure property
  //   string <: #2
  // So we have (marking type variable key by [.])
  //  #1, string <: [#2]
  //  string <: [#1] <: #2
  // $c : Cov<#2>
  // But we immediately solve because #2 appears only covariantly
  // So #2 := #1|string
  // (Note that necessarily we have [#1] <: #2 because every tyvar lower bound
  // has itself an upper bound)
  // So we now have
  //   string <: #1 <: #1|string
  // We SHOULD be able to simplify the upper bound away (because #1 <: #1|string is an identity)
  // to get
  //    $i:#1|string
  //    $c:Cov<#1|string>
  //    string <: #1
  $c = make($i);
  // hh_show_env();
  return $c;
}
