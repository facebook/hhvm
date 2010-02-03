
<h2>How to Use HPHP Compiled Libraries in C++</h2>

There are several functions that have to be called correctly to make sure
HPHP compiled binaries work in a C++ environment:

1. Without Memory Manager

(1) hphp_process_init() and hphp_process_exit()

The 1st function initializes all static variables a PHP library contains. This
function is only needed to be called just ONCE before calling any other HPHP
functions. Please call this function from process level (the "main thread")
before starting any threads.

Call hphp_process_exit() as the last HPHP function call when program exits.

(2) hphp_session_init() and hphp_session_exit()

If a PHP library does not have cycles in object references, one can call
hphp_session_init() at the beginning of a "session" and then call
hphp_session_exit() at the end of the session to free up all memory.

Both functions have to be called on per-thread basis.

2. With Memory Manager

If memory leak is detected, it may mean the PHP library has objects or arrays
forming cycles when referring to each other. This is problematic to reference
counting based memory deallocation, but it can be solved by using
MemoryManager that can sweep dangling memory periodically.

To enable MemorManager, this has to be set at program startup time, and it
cannot be turned off afterwards:

  HPHP::RuntimeOption::EnableMemoryManager = true;
  MemoryManager::TheMemoryManager()->enable();

(1) hphp_process_init()

(same as above)

(2) Taking a checkpoint

The memory manager is powerful enough to take a snapshot of the memory at any
time by doing

  hphp_session_init(); // required, same as above
  // optionally update more global states
  MemoryManager::TheMemoryManager()->checkpoint();

We only recommend to take checkpoint just once on per-thread basis, because
we have not tested what will happen when multiple checkpoint() is called.

(3) Rollback periodically

Call this once per end of "session":

  hphp_session_exit();

Please make sure there is no stack variables that are still alive when calling
rollback(). Otherwise, rollback() will release the memory to the pool, causing
that stack variable's destructor to work on collected memory.


3. Thread Local Memory Management

Almost all variables are thread-local. If you want to share those variables
between threads, you will run into trouble. Instead, copy them into your own
object classes to do so. Alternatively, use <b>apc_fetch()</b> and
<b>apc_store()</b>.


4. Execution Context

Execution context controls where PHP's stdout goes to, whether to execute
shutdown or postsend functions, etc.. It is optional to use it, but it's highly
recommended to avoid calling functions that are not "pure".

(1) At beginning of a session, set up execution context like this,

  ExecutionContext *context = hphp_context_init();

(2) At end of a session, tear down execution context like this,

  hphp_context_exit(context, true);

For the 2nd parameter, if true, post-send processing (PSP) will be performed,
if false, all PSP functions will be ignored.
