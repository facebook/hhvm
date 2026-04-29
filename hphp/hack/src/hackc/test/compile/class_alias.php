<?hh
// RUN: %hackc compile -v Hack.Lang.AllowUnstableFeatures=true %s | FileCheck %s

<<file:__EnableUnstableFeatures('class_aliases_everywhere')>>

// CHECK: .function {} ({{[0-9]+}},{{[0-9]+}}) <"HH\\void" N > test() {
// CHECK:   NewObjD "Alias"
// CHECK: }

// CHECK: .class {} Original ({{[0-9]+}},{{[0-9]+}}) {
// CHECK:   .method {} [public] ({{[0-9]+}},{{[0-9]+}}) <"HH\\string" "HH\\string" > greet() {
// CHECK:     String "hello"
// CHECK:     RetC All
// CHECK:   }
// CHECK: }
class Original {
  public function greet(): string {
    return "hello";
  }
}

class Alias = Original;

// CHECK: .class {} [interface] IOriginal ({{[0-9]+}},{{[0-9]+}}) {
// CHECK: }
interface IOriginal {}

interface IAlias = IOriginal;

function test(): void {
  new Alias();
}

// Class aliases should NOT produce separate class definitions.
// CHECK-NOT: .class {{.*}} Alias
// CHECK-NOT: .class {{.*}} IAlias

// Verify class alias entries are generated in the bytecode.
// CHECK: .class_alias Alias = Original;
// CHECK: .class_alias IAlias = IOriginal;

// Verify original classes appear in class refs for both class and interface aliases.
// CHECK: .class_refs {
// CHECK:   IOriginal
// CHECK:   Original
// CHECK: }
