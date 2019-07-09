<?hh /* $Id$ */
<<__EntryPoint>> function main(): void {
var_dump((object)1 is stdClass);
var_dump(! (object)1 is Exception);
}
