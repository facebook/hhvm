<?hh
class c {}


<<__EntryPoint>>
function main_164() :mixed{
$a = dict['x'=>'foo'];
$b = 'qqq';
$c = new c;
$c->p = 'zzz';
var_dump("AAA {$a['x']} $a[x] $b $c->p");
}
