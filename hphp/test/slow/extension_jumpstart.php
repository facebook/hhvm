<?hh

<<__EntryPoint>>
function main(): void {
  apc_store('prefix:foo', 1);
  apc_store('prefix:bar', 2);
  apc_store('noprefix:bar', 3);

  var_dump(extension_warmup_data("apc"));
}
