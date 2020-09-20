<?hh

async function f() {
  concurrent {
    await async {
      concurrent {
        await g();
        await h();
      }
    };
    await f();
  }
}
