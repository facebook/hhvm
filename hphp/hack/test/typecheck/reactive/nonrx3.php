<?hh // partial

// OK
<<__NonRx("never rx")>>
function f(): void {
}

// OK
<<__NonRx('never rx')>>
function g(): void {
}
