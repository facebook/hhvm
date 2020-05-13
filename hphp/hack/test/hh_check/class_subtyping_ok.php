<?hh

class C0<-T> {}
class C1<T> extends C0<C0<T>> {}
class C2<T> extends C1<C1<T>> {}

function subtype(): C2<C0<int>> {
  //throw new Exception();
}

function is_subtype(C0<C2<int>> $_): void {}

function main(): void {
  is_subtype(subtype());
}
