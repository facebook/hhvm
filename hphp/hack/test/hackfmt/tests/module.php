<?hh

// don't open a collapsed definition
new module hackfmt.test1 {}


// leave empty definition how it is if there are comments
new module hackfmt.test2 {
  // This is trivia that gets attached somewhere inside
}

// un-collapsed definition isn't forcefully collapsed (yet?)
new module hackfmt.test3 {
}

// all on one line to the right shape
new module hackfmt.test4 {imports {a,b,c} exports {d,e,f}}
