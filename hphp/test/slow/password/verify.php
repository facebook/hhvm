<?php

function test_verify($pass, $hash) {
  echo password_verify($pass, $hash) ? "true\n" : "false\n";
}

test_verify('foo','$2a$07$usesomesillystringforsalt$');
test_verify('rasmusler',
            '$2a$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi');
test_verify('rasmuslerdorf',
            '$2a$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi');
test_verify('rasmuslerdorf',
            '$2a$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hj');
