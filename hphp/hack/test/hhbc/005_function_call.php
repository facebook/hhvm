<?hh // strinc

function callee(): void {

}

function caller(): void {
  callee();
}
