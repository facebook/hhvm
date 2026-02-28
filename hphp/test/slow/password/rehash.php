<?hh

function test_rehash($hash, $algo, $options) :mixed{
  echo password_needs_rehash($hash, $algo, $options) ? "true\n" : "false\n";
}


<<__EntryPoint>>
function main_rehash() :mixed{
test_rehash('foo', 0, dict[]);
test_rehash('foo', 1, dict[]);
test_rehash('$2y$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi',
            PASSWORD_BCRYPT, dict[]);
test_rehash('$2y$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi',
            PASSWORD_BCRYPT, dict['cost' => 7]);
test_rehash('$2y$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi',
            PASSWORD_BCRYPT, dict['cost' => 5]);
}
