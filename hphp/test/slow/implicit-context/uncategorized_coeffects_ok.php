<?hh

<<__Memoize(#Uncategorized)>>
function ok_implicit_defaults(): void {
}

<<__Memoize(#Uncategorized)>>
function ok_explicit_defaults()[defaults]: void {
}

<<__Memoize(#Uncategorized)>>
function ok_defaults_and_redundant()[defaults, leak_safe, globals]: void {
}

<<__EntryPoint>>
function main(): void {
  echo "parses";
}
