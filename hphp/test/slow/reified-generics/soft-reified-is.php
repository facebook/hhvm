<?hh

abstract class SoftReified<<<__Soft>> reify T> {}
final class SoftReifiedInt extends SoftReified<int> {}

<<__EntryPoint>>
function main(): void {
  // these aren't allowed by the typechecker, but just tracking what the runtime
  // does do
  var_dump((new SoftReifiedInt()) is SoftReified<int>);
  var_dump((new SoftReifiedInt()) is SoftReified<string>);
}
