<?hh

function print_json(json $json) {
  match ($json) {
    _: null => { print('null'); }
    _: bool => { print($json ? 'true' : 'false'); }
    _: string => { print($json); }
    _: num => { print((string)$json); }
    _: vec<_> => { print('array'); }
    _: dict<_, _> => { print('object'); }
  }
}

function patterns(mixed $x) {
  match ($x) {
    $variable_pattern => {}
    $variable_pattern_______________________________________________________ => {}

    _: A => {}
    _: A\B => {}
    _: \A\B => {}
    _: \A\B<C________________________________________________________> => {}

    $_: A => {}
    $_: A\B => {}
    $_: \A\B => {}
    $_: \A\B<C________________________________________________________> => {}

    $a: A => {}
    $b: A\B => {}
    $b: \A\B => {}
    $abc____________________________________________________: \A\B<C________________________________________________________> => {}

    None => {}
    Some($x) => {}

    _ => {}
    _named_wildcard => {}
  }
}
