<?hh // strict

function empty(): vec<int> {
  return vec[];
}

function collection(): vec<string> {
  return vec["hi", "there"];
}

function add(vec<int> $vec): void {
  $vec[] = 2;
}

function retElem(vec<int> $vec): int {
  return $vec[1];
}
