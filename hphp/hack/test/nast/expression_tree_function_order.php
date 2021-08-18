<?hh

function test(): void {
  Code`
    foo(
      bar(
        baz(),
        qux(),
      ),
      qaal(),
  )`;
}
