<?hh

function __source(): int { return 1; }
function __sink($input): void {}

function no_flow_to_sink(): void {
  $vec = vec[0, 0];
  __sink($vec[0]);
}

function source_through_vec_to_sink(): void {
  $vec = vec[];
  $vec[] = __source();
  __sink($vec[0]);
}

function source_through_vec_stopped(): void {
  $vec = vec[];
  $vec[] = __source();
  $vec[] = 1;
  __sink($vec[1]);
}

function source_through_vec_at_index_to_sink(): void {
  $vec = vec[0, 0, 0];
  $vec[2] = __source();
  __sink($vec[2]);
}

function source_through_vec_at_index_stopped(): void {
  $vec = vec[0, 0, 0];
  $vec[2] = __source();
  $vec[2] = 0;
  __sink($vec[2]);
}

<<__EntryPoint>> function main(): void {
  no_flow_to_sink();
  source_through_vec_to_sink();
  source_through_vec_stopped();
  source_through_vec_at_index_to_sink();
  source_through_vec_at_index_stopped();
}
