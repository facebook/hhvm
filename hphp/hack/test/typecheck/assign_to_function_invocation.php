<?hh // strict

function do_stuff(int $elapsed_time, int $start_time): void {
  $elapsed_time = Time::nowInMS() = $start_time;
}
