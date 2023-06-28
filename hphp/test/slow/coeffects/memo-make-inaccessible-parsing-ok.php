<?hh

<<__Memoize(#MakeICInaccessible)>>
function implicitDefaults(): void {
}

<<__Memoize(#MakeICInaccessible)>>
function explicitDefaults()[defaults]: void {
}

<<__Memoize(#SoftMakeICInaccessible)>>
function softImplicitDefaults(): void {
}

<<__Memoize(#SoftMakeICInaccessible)>>
function softExplicitDefaults()[defaults]: void {
}

<<__Memoize(#MakeICInaccessible)>>
function leakSafeLocal()[leak_safe_local]: void {
}

<<__Memoize(#SoftMakeICInaccessible)>>
function softLeakSafeLocal()[leak_safe_local]: void {
}

<<__Memoize(#MakeICInaccessible)>>
function leakSafeShallow()[leak_safe_shallow]: void {
}

<<__Memoize(#SoftMakeICInaccessible)>>
function softLeakSafeShallow()[leak_safe_shallow]: void {
}

<<__EntryPoint>>
function main() :mixed{
  echo "Done\n";
}
