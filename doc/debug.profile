
<h2>Using Google CPU Profiler</h2>

Building with GOOGLE_CPU_PROFILER set lets you collect profiles from
the server or from the command line. However, our testing has found
that the server can get stalled while profiling for reasons
unknown. Still, it's possible to get enough requests in that the
profile can be significant.

There are two stages in profiling, collecting the profile and
processing it into a readable format. Collection is different on the
command line and server.

<h3>Profiling from the command line</h3>

For building stand alone programs you need to link with libprofiler and libunwind: 

 export LIBRARY_PATH=[path]/hphp/external/google-perftools/lib:[path]/hphp/external/libunwind/lib
 g++ <my program>.cpp -lprofiler 

With a compiled program, p, execute:

  CPUPROFILE=p.prof CPUPROFILE_FREQUENCY=1000 p args

This will create a file p.prof when p finishes while taking samples 1000
times per second. The frequency can be changed: higher frequencies
will impact performance but lower frequencies will require a longer
run to collect a significant number of samples.

<h3>Profiling from the server</h3>

Run

  GET http://[server]:9999/prof-cpu-on

Then hit the server some number of times. When satisfied,

  GET http://[server]:9999/prof-cpu-off

A file /hphp/pprof/[host]/hphp.prof should be created. The exact path is
configurable with the runtime option Debug.ProfilerOutputDir (defaults to /tmp on production).

<h3>Processing the profile</h3>

Use the tool pprof to process the profile. For example:

  pprof --gif p p.prof > p.gif

This generates a gif with the callgraph of p.

Note that if you needed to strip the program, it's still possible
to use pprof if you call it on the unstripped version.
