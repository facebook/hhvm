<?hh

class X
{
    private $prop10 = varray[];
    private $prop11 = darray[0 => 10, "a" => "red", 1 => TRUE];
    private $prop12 = darray[0 => 10, 1 => "red", "xx" => darray[0 => 2.3, 1 => NULL, "zz" => varray[12, FALSE, "zzz"]]];

    private $prop20 = darray[];
    private $prop21 = darray[0 => 10, "a" => "red", 1 => TRUE];
    private $prop22 = darray[0 => 10, 1 => "red", "xx" => darray[0 => 2.3, 1 => NULL, "zz" => varray[12, FALSE, "zzz"]]];
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $x = new X;
  var_dump($x);
}
