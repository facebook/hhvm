<?hh

function test(): void {
  ExampleDsl`
    foo(
      bar(
        baz(),
        qux(),
      ),
      qaal(),
  )`;
}
