<?hh
<<__EntryPoint>> function main(): void {
var_dump(date('d.m.Y',strtotime('last day of april')));
var_dump(date('d.m.Y',strtotime('last tuesday of march 2015')));
var_dump(date('d.m.Y',strtotime('last wednesday of march 2015')));
var_dump(date('d.m.Y',strtotime('last wednesday of april 2015')));
var_dump(date('d.m.Y',strtotime('last wednesday of march 2014')));
var_dump(date('d.m.Y',strtotime('last wednesday of april 2014')));
}
