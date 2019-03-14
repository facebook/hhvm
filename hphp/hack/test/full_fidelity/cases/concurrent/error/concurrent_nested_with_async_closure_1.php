<?hh

async function f() {
  concurrent {
    async {
      concurrent {
        await g();
        await h();
      }
    };
    await f();
  }
}
