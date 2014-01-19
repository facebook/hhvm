<?php

function test_rehash($hash, $algo, $options) {
  echo password_needs_rehash($hash, $algo, $options) ? "true\n" : "false\n";
}

test_rehash('foo', 0, []);
test_rehash('foo', 1, []);
test_rehash('$2y$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi',
            PASSWORD_BCRYPT, array());
test_rehash('$2y$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi',
            PASSWORD_BCRYPT, array('cost' => 7));
test_rehash('$2y$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi',
            PASSWORD_BCRYPT, array('cost' => 5));
