<?hh
async function f() {
  await async {
    using new C();
    print 1;
  };
}
