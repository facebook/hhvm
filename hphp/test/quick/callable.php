<?hh

function check($f) {
  var_dump($f);
  var_dump(is_callable($f));
}

function typehint(callable $a) {
  var_dump('worked');
}

function id() {}

<<__EntryPoint>> function main(): void {
    check('');
    check('id');
    check('blarblah');

    $cl = function() { return 'closure'; };
    typehint($cl);
    typehint('id');
}
