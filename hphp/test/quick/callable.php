<?hh

function check($f) :mixed{
  var_dump($f);
  var_dump(is_callable($f));
}

function typehint(callable $a) :mixed{
  var_dump('worked');
}

<<__DynamicallyCallable>>
function id() :mixed{}

<<__EntryPoint>> function main(): void {
    check('');
    check('id');
    check('blarblah');

    $cl = function() { return 'closure'; };
    typehint($cl);
    typehint('id');
}
