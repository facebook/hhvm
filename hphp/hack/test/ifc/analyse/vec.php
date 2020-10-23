<?hh // strict

<<__InferFlows>>
function empty(): vec<int> {
  return vec[];
}

<<__InferFlows>>
function collection(): vec<string> {
  return vec["hi", "there"];
}

<<__InferFlows>>
function add(vec<int> $vec): void {
  $vec[] = 2;
}

<<__InferFlows>>
function retElem(vec<int> $vec): int {
  return $vec[1];
}
