<?hh

class C {

  protected ImmMap<int, mixed> $accessTokens;

  function __construct() {
    $this->accessTokens = ImmMap{};
  }

  final public function genWithAccessTokens(
    ?array<int, mixed> $tokens_data = null,
  ): bool {
    $tokens_data = $tokens_data ?: array();
    $z = $this->accessTokens->toMap();
    return $z->containsKey(100);
  }
}

function main() {
  $obj = new C;
  var_dump($obj->genWithAccessTokens(array()));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(array()));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(array()));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(array()));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(array()));
  var_dump($obj->genWithAccessTokens());
  var_dump($obj->genWithAccessTokens(array()));
  var_dump($obj->genWithAccessTokens());
}

main();
