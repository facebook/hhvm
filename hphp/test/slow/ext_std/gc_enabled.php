<?hh

<<__EntryPoint>>
function main_gc_enabled() :mixed{
var_dump(gc_enabled());
gc_disable();
var_dump(gc_enabled());
gc_enable();
var_dump(gc_enabled());
ini_set("zend.enable_gc", false);
var_dump(gc_enabled());
ini_set("zend.enable_gc", true);
var_dump(gc_enabled());
gc_disable();
var_dump(gc_enabled());
}
