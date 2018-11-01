<?hh

async function f() {
  concurrent {
    async () ==> {
      await g();
    };
    await h();
  }
}
