<?hh
class :test {
  static function go() :mixed{
    echo "Everything's cool\n";
  }
}


<<__EntryPoint>>
function main_1850() :mixed{
:test::go();
}
