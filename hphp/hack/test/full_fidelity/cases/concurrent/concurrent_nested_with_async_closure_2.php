<?hh

async function f() {
  concurrent {
    await async {
      await g();
    };
    await h();
  }
}
