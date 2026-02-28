<?hh

class C {
  public $p = 0;
}

function error_boundary(inout $x, $fn) :mixed{
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

<<__EntryPoint>> function main(): void {
print "Test begin\n";

$o = new C;
$o->a .= "<a>";
$o->b .= "<b>";
$o->b .= "<b>";
$o->p += 1;
$o->q ??= 0;
$o->q += 1;
$o->r .= "hello";
print_r($o);

$o = null;
error_boundary(inout $o, (inout $o) ==> $o->a .= "<a>");
error_boundary(inout $o, (inout $o) ==> $o->a .= "<b>");
error_boundary(inout $o, (inout $o) ==> $o->a .= "<b>");
error_boundary(inout $o, (inout $o) ==> $o->a += 1);
error_boundary(inout $o, (inout $o) ==> $o->a += 1);
error_boundary(inout $o, (inout $o) ==> $o->a .= "hello");
print_r($o);

$o = 42;
$o->a .= "<a>";
$o->b .= "<b>";
print_r($o);
print "\n";

print "Test end\n";
}
