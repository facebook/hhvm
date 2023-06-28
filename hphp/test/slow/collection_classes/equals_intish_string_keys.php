<?hh

function main() :mixed{
  // intish string key should not == integer keys
  var_dump(   Set {42}   ==    Set {'42'});
  var_dump(   Set {42}   == ImmSet {'42'});
  var_dump(ImmSet {42}   ==    Set {'42'});
  var_dump(ImmSet {42}   == ImmSet {'42'});
  var_dump(   Set {'42'} ==    Set {42});
  var_dump(   Set {'42'} == ImmSet {42});
  var_dump(ImmSet {'42'} ==    Set {42});
  var_dump(ImmSet {'42'} == ImmSet {42});

  var_dump(   Map { 42  => 42} ==    Map {'42' => 42});
  var_dump(   Map { 42  => 42} == ImmMap {'42' => 42});
  var_dump(ImmMap { 42  => 42} ==    Map {'42' => 42});
  var_dump(ImmMap { 42  => 42} == ImmMap {'42' => 42});
  var_dump(   Map {'42' => 42} ==    Map { 42  => 42});
  var_dump(   Map {'42' => 42} == ImmMap { 42  => 42});
  var_dump(ImmMap {'42' => 42} ==    Map { 42  => 42});
  var_dump(ImmMap {'42' => 42} == ImmMap { 42  => 42});

  // intish string value can be == integer value
  var_dump(   HH\Lib\Legacy_FIXME\eq(Map {42 =>  42 }, Map {42 => '42'}));
  var_dump(   HH\Lib\Legacy_FIXME\eq(Map {42 =>  42 }, ImmMap {42 => '42'}));
  var_dump(HH\Lib\Legacy_FIXME\eq(ImmMap {42 =>  42 }, Map {42 => '42'}));
  var_dump(HH\Lib\Legacy_FIXME\eq(ImmMap {42 =>  42 }, ImmMap {42 => '42'}));
  var_dump(   HH\Lib\Legacy_FIXME\eq(Map {42 => '42'}, Map {42 =>  42 }));
  var_dump(   HH\Lib\Legacy_FIXME\eq(Map {42 => '42'}, ImmMap {42 =>  42 }));
  var_dump(HH\Lib\Legacy_FIXME\eq(ImmMap {42 => '42'}, Map {42 =>  42 }));
  var_dump(HH\Lib\Legacy_FIXME\eq(ImmMap {42 => '42'}, ImmMap {42 =>  42 }));
}

<<__EntryPoint>>
function main_equals_intish_string_keys() :mixed{
main();
}
