<?hh
class foo {
    public function __get($member) {
        $f = darray["foo"=>"bar","bar"=>"foo"];
        return $f;
    }
}
<<__EntryPoint>> function main(): void {
$f = new foo();
foreach($f->bar as $key => $value) {
    print "$key => $value\n";
}
}
