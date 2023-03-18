<?hh

<<__SupportDynamicType>>
function map<T>(T $_, (function(T): void) $_): void {}

<<__SupportDynamicType>>
function main1(): void {
  map(42.0, $f ==> { (float) $f; });
}

<<__SupportDynamicType>>
function main2(): void {
  map(42.0, (float $f) ==> { (float) $f; });
}
