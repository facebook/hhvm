<?hh // partial

class A_f11 {
  <<__Rx, __Mutable>>
    public async function f(): Awaitable<void> {
  }
}

<<__Rx, __MutableReturn>>
function get(): A_f11 {
  return new A_f11();
}


<<__Rx>>
async function f_f11(): Awaitable<void> {
  await get()->f();
}
