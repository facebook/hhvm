<?hh

class C {

  protected ImmMap<int, mixed> $accessTokens;

  function __construct() {
    $this->accessTokens = ImmMap{};
  }

  final public function genWithAccessTokens(
    ?varray<mixed> $tokens_data = null,
  ): bool {
    $tokens_data = $tokens_data ?: varray[];
    $z = $this->accessTokens->toMap();
    return $z->containsKey(100);
  }
}

function main() {
  $obj = new C;
  var_dump($obj->genWithAccessTokens(varray[]));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(varray[]));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(varray[]));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(varray[]));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(varray[]));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(varray[]));
  var_dump($obj->genWithAccessTokens());
}


<<__EntryPoint>>
function main_test38() {
main();
}
