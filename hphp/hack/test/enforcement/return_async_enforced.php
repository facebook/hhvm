<?hh

// akenn: async functions should unwrap Awaitable and report
// enforcement of the inner type, not Awaitable<int>.
async function foo(): Awaitable<int> {
  return 42;
//       ^ enforcement-at-caret
}
