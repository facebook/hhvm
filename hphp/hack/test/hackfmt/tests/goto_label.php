<?hh

function singleGotoLabel(): void {
  L0:
}

function multipleGotoLabels(): void {
  L0: L1:
}

function gotoLabelWithExpression(): void {
  L0: $x = 1 + 1;
}
