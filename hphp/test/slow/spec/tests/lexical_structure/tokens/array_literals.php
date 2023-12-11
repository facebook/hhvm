<?hh

class X
{
    private $prop10 = vec[];
    private $prop11 = dict[0 => 10, "a" => "red", 1 => TRUE];
    private $prop12 = dict[0 => 10, 1 => "red", "xx" => dict[0 => 2.3, 1 => NULL, "zz" => vec[12, FALSE, "zzz"]]];

    private $prop20 = dict[];
    private $prop21 = dict[0 => 10, "a" => "red", 1 => TRUE];
    private $prop22 = dict[0 => 10, 1 => "red", "xx" => dict[0 => 2.3, 1 => NULL, "zz" => vec[12, FALSE, "zzz"]]];
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $x = new X;
  var_dump($x);
}
