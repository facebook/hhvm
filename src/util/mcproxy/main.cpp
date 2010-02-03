
#include "proxy.h"
#include "stats.h"
#include "cluster.h"
#include "loadconfig.h"
#include "prefix.h"
#include "qio.h"
#include "gfuncs.h"

extern int portnum(int fd);

/* How often to refresh our configuration, in seconds. */
#define CONFIG_RELOAD_INTERVAL 57

extern QIOReadCBF handle_connection;
extern int debug_enabled;
extern int last_action;
extern int parent_pipe_fd;
extern int async_deletes;
extern char *async_dir;
extern uint32_t max_asyncstore_len;
extern int server_timeout_msec;

time_t last_config_update_time = 0;


void daemonize()
{
  if (getppid() == 1) return;
  switch (fork()) {
    case 0:
      setsid();
      int i;
      for (i=getdtablesize();i>=0;--i) close(i); // close all descriptors
      i = open("/dev/null", O_RDWR);
      dup2(i, 1);
      dup2(i, 2);
      break;
    case -1:
      HPHP::Logger::Error("Can't fork background process");
      throw HPHP::Exception("exit with %d", 1);
    default:  /* Parent process */
      HPHP::Logger::Verbose("Running in the background");
      throw HPHP::Exception("exit with %d", 0);
  }
}

void usage(char *progname)
{
  fprintf(stderr,
    "Usage: %s [-AbdmR] [-a dir] [-c cluster] [-f configfile]\n"
      "\t\t[-F host[:port]] [-L logfile_path] [-p port] [-s size] [-n connection limit]\n",
    progname);
  fprintf(stderr, "\t-a\tdirectory for async storage (default /tmp)\n");
  fprintf(stderr, "\t-A\tenable asynchronous forwarding of deletes\n");
  fprintf(stderr, "\t-b\trun in background\n");
  fprintf(stderr, "\t-c\tspecify local cluster ID\n");
  fprintf(stderr, "\t-d\tactivate debug mode\n");
  fprintf(stderr, "\t-f\tload configuration from file\n");
  fprintf(stderr, "\t-F\tload configuration from memcached server\n");
  fprintf(stderr, "\t-L\tlogfile path name\n");
  fprintf(stderr, "\t-m\tmanaged mode (auto restart on crash)\n");
  fprintf(stderr, "\t-n\tset maximum number of connections (default 65536)\n");
  fprintf(stderr, "\t-s\tset maximum async store size in MB (default 10)\n");
  throw HPHP::Exception("exit with %d", 1);
}

/*
 * The parent process has died in managed mode.
 */
static void
parent_died(int fd, void *arg)
{
  HPHP::Logger::Info("Parent died; exiting");
  throw HPHP::Exception("exit with %d", 0);
}


/*
 * Forks off child processes and watches for their deaths if we're running in
 * managed mode.
 */
static void
watch_children(int *pipefds)
{
  char  c;
  int  pid;
  int  res;

#ifdef SIGCHLD
  signal(SIGCHLD, SIG_IGN);
#endif
  while (1)
  {
    if (pipe(pipefds))
    {
      HPHP::Logger::Error("Can't open parent-child pipe");
      throw HPHP::Exception("exit with %d", 1);
    }
    switch (pid = fork()) {
    case 0:
      /* Child process. Continue with the startup logic. */
      close(pipefds[0]);
      stats_set_starttime((int) time(NULL));
      return;

    case -1:
      close(pipefds[0]);
      close(pipefds[1]);
      HPHP::Logger::Error("Can't spawn child process, sleeping");
      sleep(10);
      break;

    default:
      close(pipefds[1]);
      HPHP::Logger::Info("Spawned child process %d", pid);
      while ((res = read(pipefds[0], &c, 1)) == -1)
        ;

      close(pipefds[0]);
      HPHP::Logger::Info("Child process %d exited", pid);
      if (res == 1 && c == 'q')
      {
        HPHP::Logger::Info("It was terminated; terminating parent");
        throw HPHP::Exception("exit with %d", 0);
      }
    }
  }
}


/*
 * Configuration has been loaded; start using it.
 */
static void
config_done(REQUEST *request, void *args)
{
  int ignore_cluster = args != NULL;

  if (server_count() == 0)
  {
    HPHP::Logger::Error("No servers specified in config file");
    return;
  }

  server_connect_all();

  if (ignore_cluster)
    return;

  if (cluster_determine_id())
  {
    HPHP::Logger::Error("Can't determine local cluster");
    return;
  }

  last_config_update_time = time(NULL);

  HPHP::Logger::Verbose("Local cluster ID is %d", cluster_id());
}


/*
 * Reloads the configuration periodically.
 */
static void
reload_config(void *arg)
{
  char *config_file = (char*)arg;

  if (NULL != config_file)
  {
    read_config_file(config_file, config_done);
  }
  else
  {
    read_config_server(config_done);
  }
}

/*
 * handle SIGHUP
 */
static void
sighup_handler(int signum)
{
}

int start_mcproxy(int argc, char **argv)
{
  struct rlimit rlim;
  int c;
  int port = 11100;
  int accept_fd;
  int managed = 0;
  int want_background = 0;
  int num_sockets = 65536;
  int have_config_file = 0;
  char *pidfile = NULL;
  char *config_file = NULL;
  char *config_host_port = NULL;
  char *config_key = NULL;
  int fds[2];

  // if a child is spawned, will be overwritten by child
  stats_set_starttime((int) time(NULL));
#ifdef FIND_LEAKS
  /* Activate the garbage collector's leak detection mode. */
  GC_find_leak = 1;
  GC_start_debugging();
#endif

  ginit();
  request_list_init();
  request_init();
  prefix_init();
  continuum_init(continuum_default_error_fn, NULL);

#ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif

  while ((c = getopt(argc, argv, "a:Abc:df:F:L:mn:p:P:Rs:k:t:")) != EOF)
  {
    switch (c) {
    case 'a':
      async_dir = optarg;
      break;

    case 'A':
      async_deletes = 1;
      break;

    case 'b':
      want_background = 1;
      //setup_sigsegv();
      break;

    case 'c':
      cluster_set_id(atoi(optarg));
      break;

    case 'd':
      //debug_enabled++;
      break;

    case 'f':
      if (access(optarg, R_OK))
      {
        HPHP::Logger::Error("%s", optarg);
        throw HPHP::Exception("exit with %d", 1);
      }
      config_file = optarg;
      have_config_file = 1;
      break;

    case 'F':
      config_host_port = optarg;
      break;
    case 'L':
      //syslog_name = optarg;
      break;
    case 'm':
      managed = 1;
      break;

    case 'n':
      num_sockets = atoi(optarg);
      break;

    case 'k':
      config_key = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      if (port == 0)
        usage(argv[0]);
      break;

    case 'P':
      pidfile = optarg;
      break;

    case 's':
      max_asyncstore_len = atoi(optarg) * 1024 * 1024;
      break;

    case 't':
      server_timeout_msec = atoi(optarg);
      break;

    default:
      usage(argv[0]);
    }
  }

  if (NULL == config_file && NULL == config_host_port)
  {
    HPHP::Logger::Error("No config source specified");
    throw HPHP::Exception("exit with %d", 1);
  }

  /*
   * If config is from a file, we can read it now; otherwise we have to
   * wait until the async I/O system is up and running. (Even in the
   * case of a file, we'll reload the config once the I/O system is
   * up, but this lets us flag syntax errors before forking.)
   */
  if (NULL != config_file && read_config_file(config_file, NULL))
  {
    HPHP::Logger::Error("Can't read config file");
    throw HPHP::Exception("exit with %d", 1);
  }

  if (NULL != config_host_port && config_set_server(config_host_port,
              config_key))
  {
    HPHP::Logger::Error("Invalid configuration server");
    throw HPHP::Exception("exit with %d", 1);
  }

  /*
   * If both a server and a file are specified, use the file for the
   * initial configuration (which we've just read above) and then switch
   * to the server. This allows a bootstrap configuration in case the
   * server is offline.
   */
  if (NULL != config_host_port && NULL != config_file)
    config_file = NULL;

  if (want_background)
  {
    /* open and close the log file to verify we have permission to
     * open it. We do this so that we can produce a useful error
     * message before daemonize closes all open file descriptors,
     * including stdout/stderr.
     */
    daemonize();
    if( signal(SIGHUP, sighup_handler) == SIG_ERR ) {
      HPHP::Logger::Error("Can't set SIGHUP handler");
      throw HPHP::Exception("exit with %d", 1);
    }
  }

  accept_fd = serversock(port);
  port = portnum(accept_fd);
  if (accept_fd < 0)
  {
    HPHP::Logger::Error("Can't open listen socket");
    throw HPHP::Exception("exit with %d", 1);
  }
  HPHP::Logger::Info("mcproxy is listening on port %d", port);

  if (pidfile != NULL)
  {
    int fd;
    char pid[20];
    sprintf(pid, "%d", getpid());

    fd = open(pidfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0)
    {
      if (write(fd, pid, strlen(pid)) < (int)strlen(pid))
        HPHP::Logger::Error("%s", pidfile);
      close(fd);
    }
    else
      HPHP::Logger::Error("%s", pidfile);
  }

  if (managed)
  {
    watch_children(fds);
  }

  /*
   * Set the fdlimit before qio_init() because libevent uses the fd
   * limit as a initial allocation buffer and if the limit is too
   * low, then libevent realloc's the buffer which may cause epoll
   * to copy out to the wrong address returning bogus data to user
   * space, or it's just a bug in libevent.  It's simpler to increase
   * the limit than to fix libevent.
   * libev does not suffer from this problem.
   */
  if (getrlimit(RLIMIT_NOFILE, &rlim) != 0) {
    HPHP::Logger::Error("Failed to getrlimit RLIMIT_NOFILE");
    throw HPHP::Exception("exit with %d", 1);
  } else {
    if ((int)rlim.rlim_cur < num_sockets)
      rlim.rlim_cur = num_sockets;
    if (rlim.rlim_max < rlim.rlim_cur)
      rlim.rlim_max = rlim.rlim_cur;
    if (getuid() == 0 &&
        setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
      HPHP::Logger::Error("failed to setrlimit RLIMIT_NOFILE");
      throw HPHP::Exception("exit with %d", 1);
    }
  }

  /*
   * Have to do the qio_init() after forking since libevent doesn't
   * survive being forked on some platforms.
   */
  qio_init();

  /*
   * Determine the local cluster ID if possible, then start connecting
   * to all the servers we know about so far, including (possibly) the
   * configuration server.
   */
  config_done(NULL, have_config_file ? NULL : (void*)1);

  /*
   * Now schedule the server to read the configuration from whatever
   * configuration source we're using. This will happen after we pass
   * control to the I/O system.
   */
  qio_schedule(time(NULL) + 2, reload_config, config_file,
      CONFIG_RELOAD_INTERVAL);

  HPHP::Logger::Verbose("Listening on port %d fd %d", port, accept_fd);

  /*
   * If we're running in managed mode, the child should exit if the
   * parent is killed.
   */
  if (managed)
  {
    parent_pipe_fd = fds[1];
    qio_add(parent_pipe_fd, parent_died, NULL, NULL);
  }

  client_add_handler(accept_fd);
  unlink(pidfile);
  return port;
}

void run_mcproxy() {
  qio_main();
  qio_cleanup();
}
