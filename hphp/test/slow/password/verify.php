<?hh

function test_verify($pass, $hash) :mixed{
  echo password_verify($pass, $hash) ? "true\n" : "false\n";
}



<<__EntryPoint>>
function main_verify() :mixed{
test_verify('foo','$2a$07$usesomesillystringforsalt$');
test_verify('rasmusler',
            '$2a$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi');
test_verify('rasmuslerdorf',
            '$2a$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi');
test_verify('rasmuslerdorf',
            '$2a$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hj');
$password = 'password';
$crypt = crypt($password, 'salt');
echo password_verify($password, $crypt) ? "true\n" : "false\n";
}
