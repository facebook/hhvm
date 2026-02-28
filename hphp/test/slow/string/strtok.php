<?hh

function tokenize($str, $token) :mixed{
    $tokenizedStrArr = vec[];
    $tokStr = strtok($str, $token);

    while($tokStr !== false) {
        $tokenizedStrArr[] = $tokStr;

        $tokStr = strtok($token);
    }

    return $tokenizedStrArr;
}


<<__EntryPoint>>
function main_strtok() :mixed{
var_dump(tokenize('foobarbaz', 'foo'));
var_dump(tokenize('foobarbaz', 'bar'));
var_dump(tokenize('foobarbaz', 'baz'));

var_dump(tokenize('foobarbaz', 'foobar'));
var_dump(tokenize('foobarbaz', 'barbaz'));
var_dump(tokenize('foobarbaz', 'foobaz'));
}
