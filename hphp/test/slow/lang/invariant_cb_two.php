<?hh
function a() {}
<<__EntryPoint>> function main(): void {
invariant_callback_register('a');
invariant_callback_register('a');
}
