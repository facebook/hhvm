<?hh
<<__EntryPoint>> function main(): void {
print "Test begin\n";

$obj = new stdClass;
unset($obj->doh->re->mi->fa->sol->la->ti);
var_dump($obj);

print "Test end\n";
}
