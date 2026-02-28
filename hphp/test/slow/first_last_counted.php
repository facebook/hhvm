<?hh

class C { public function __construct(public $x)[] {} }

<<__EntryPoint>>
function main() :mixed{
  $x = dict[
    json_decode('"foo"') => new C(17),
    json_decode('"bar"') => new C(34),
  ];
  var_dump(HH\Lib\_Private\Native\first_key($x));
  var_dump(HH\Lib\_Private\Native\first($x));
  var_dump(HH\Lib\_Private\Native\last_key($x));
  var_dump(HH\Lib\_Private\Native\last($x));

  $x = vec[new C(51), new C(68)];
  var_dump(HH\Lib\_Private\Native\first_key($x));
  var_dump(HH\Lib\_Private\Native\first($x));
  var_dump(HH\Lib\_Private\Native\last_key($x));
  var_dump(HH\Lib\_Private\Native\last($x));
}
