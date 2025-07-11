<?hh

use namespace HH\Lib\{Async, C, Vec};

/**
 * Wrapper on top of Async\LowPri that can be used to process an ordered
 * collection of work (async lambdas) in a prioritized manner. All tasks will be
 * initialized to run in a low priority context, with only the first task being
 * prioritized to the current (non-lowpri) context. When the first task is
 * finished it will prioritize the next, unfinished task. This continues until
 * all tasks are done processing.
 */
final class PrioritizedTasks<T> {
  // Container to hold the actual LowPri jobs
  private vec<Async\LowPri<T>> $lowpri_jobs;

  // Tracking of which tasks have been completed
  private vec<bool> $completed;

  // Index of the most recently prioritized task, used to track state for when
  // the current task should prioritize the next unfinished task.
  private int $prioritizedIdx = 0;

  /**
   * Construct the PrioritizedTasks object with tracking state. Await run() to
   * begin processing prioritized tasks.
   */
  public function __construct(
    private vec<(function(): Awaitable<T>)> $tasks,
  ) {
    invariant(C\count($tasks) > 0, 'Tasks vector cannot be empty');
    $this->lowpri_jobs = Vec\map($tasks, $_ ==> new Async\LowPri<T>());
    $this->completed = Vec\fill(C\count($tasks), false);
  }

  /**
   * Mark a task as finished and prioritize the next unfinished task if the
   * current task was the most recently prioritized task.
   */
  private function finish(int $idx): void {
    $this->completed[$idx] = true;
    if ($idx === $this->prioritizedIdx) {
      for ($i = $idx + 1; $i < C\count($this->completed); $i++) {
        if (!$this->completed[$i]) {
          $this->lowpri_jobs[$i]->prioritize();
          $this->prioritizedIdx = $i;
          return;
        }
      }
    }
  }

  /**
   * Actually run the async tasks. Starts all tasks concurrently, but continues
   * to call prioritize() on the first-most unfinished task each time the first
   * unfinished task is finished.
   */
  public async function run(): Awaitable<vec<T>> {
    $this->lowpri_jobs[0]->prioritize();

    return await Vec\map_with_key_async(
      $this->tasks,
      async ($idx, $task) ==> {
        $res = await $this->lowpri_jobs[$idx]->run($task);
        $this->finish($idx);
        return $res;
      },
    );
  }
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
    $create_task = async (int $task_num, int $sleep_ms): Awaitable<int> ==> {
      await HH\Asio\usleep($sleep_ms * 1000);
      echo "Finish {$task_num}\n";
      return $task_num;
    };

    $tasks = vec[
      async () ==> await $create_task(0, 3000),
      async () ==> await $create_task(1, 1000),
      async () ==> await $create_task(2, 2000),
    ];

    $runner = new PrioritizedTasks<int>($tasks);
    $results = \HH\Asio\join($runner->run());

    var_dump($results);
}
