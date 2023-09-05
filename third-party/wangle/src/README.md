[![Support Ukraine](https://img.shields.io/badge/Support-Ukraine-FFD500?style=flat&labelColor=005BBB)](https://opensource.fb.com/support-ukraine) [![Travis Build Status](https://api.travis-ci.com/facebook/wangle.svg?branch=master)](https://travis-ci.com/facebook/wangle)
[![CI Status](https://github.com/facebook/wangle/workflows/CI/badge.svg?branch=master)](https://github.com/facebook/wangle/actions?workflow=CI)
<section class="dex_guide"><h1 class="dex_title">Wangle</h1><section class="dex_document"><h1></h1><p class="dex_introduction">C++ networking library</p><p>Wangle is a library that makes it easy to build protocols, application clients, and application servers.</p>

<p>It&#039;s like Netty + Finagle smooshed together, but in C++</p>

## Building and Installing

The main dependencies are:
* The folly library from https://github.com/facebook/folly
* The fizz library from https://github.com/facebookincubator/fizz
* CMake
* OpenSSL, at least version 1.0.2+, preferably with TLS extension support.

Once folly is installed, run the following inside the wangle directory to build, test, and install wangle:
```
cmake .
make
ctest
sudo make install
```

## Tutorial

There is a tutorial [here](tutorial.md) that explains the basics of Wangle and shows how to build an echo server/client.

## Examples

See the examples/ directory for some example Wangle servers and clients

## License
Wangle is Apache 2.0-licensed.

## Contributing
See the CONTRIBUTING file for how to help out.

## Documentation

<p>Wangle interfaces are asynchronous.  Interfaces are currently based on <a href="https://github.com/facebook/folly/tree/master/folly/futures">Futures</a>, but we&#039;re also exploring how to support fibers</p>

<h2 id="client-server-abstractio">Client / Server abstraction <a href="#client-server-abstractio" class="headerLink">#</a></h2>

<p>You&#039;re probably familiar with Java&#039;s Netty, or Python&#039;s twisted, or similar libraries.</p>

<p>It is built on top of folly/async/io, so it&#039;s one level up the stack from that (or similar abstractions like boost::asio)</p>

<p>ServerBootstrap - easily manage creation of threadpools and pipelines</p>

<p>ClientBootstrap - the same for clients</p>

<p>Pipeline - set up a series of handlers that modify your socket data</p>

<h2 id="request-response-abstrac">Request / Response abstraction <a href="#request-response-abstrac" class="headerLink">#</a></h2>

<p>This is roughly equivalent to the <a href="https://twitter.github.io/finagle/" target="_blank">Finagle</a> library.</p>

<p>Aims to provide easy testing, load balancing, client pooling, retry logic, etc.  for any request/response type service - i.e. thrift, http, etc.</p>

<p>Service - a matched interface between client/server.  A server will implement this interface, and a client will call in to it.  These are protocol-specific</p>

<p>ServiceFilter - a generic filter on a service. Examples: stats, request timeouts, rate limiting</p>

<p>ServiceFactory - A factory that creates client connections.  Any protocol specific setup code goes here</p>

<p>ServiceFactoryFilter - Generic filters that control how connections are created.  Client examples: load balancing, pooling,  idle timeouts, markdowns, etc.</p></section><section class="dex_document"><h1>ServerBootstrap</h1><p class="dex_introduction">Easily create a new server</p><p>ServerBootstrap does the work to set up one or multiple acceptor threads, and one or multiple sets of IO threads.  The thread pools can be the same.  SO_REUSEPORT is automatically supported for multiple accept threads. tcp is most common, although udp is also supported.</p>

<h2 id="methods">Methods <a href="#methods" class="headerLink">#</a></h2>

<p><strong>childPipeline(PipelineFactory&lt;Pipeline&gt;)</strong></p>

<p>Sets the pipeline factory for each new connection.  One pipeline per connection will be created.</p>

<p><strong>group(IOThreadPoolExecutor accept, IOThreadPoolExecutor io)</strong></p>

<p>Sets the thread pools for accept and io thread pools.  If more than one thread is in the accept group, SO_REUSEPORT is used.  Defaults to a single accept thread, and one io thread per core.</p>

<p><strong>bind(SocketAddress),bind(port)</strong></p>

<p>Binds to a port. Automatically starts to accept after bind.</p>

<p><strong>stop()</strong></p>

<p>Stops listening on all sockets.</p>

<p><strong>join()</strong></p>

<p>Joins all threadpools - all current reads and writes will be completed before this method returns.</p>

<div class="remarkup-note"><span class="remarkup-note-word">NOTE:</span> however that both accept and io thread pools will be stopped using this method, so the thread pools can&#039;t be shared, or care must be taken using shared pools during shutdown.</div>

<p><strong>waitForStop()</strong></p>

<p>Waits for stop() to be called from another thread.</p>

<h2 id="other-methods">Other methods <a href="#other-methods" class="headerLink">#</a></h2>

<p><strong>channelFactory(ServerSocketFactory)</strong></p>

<p>Sets up the type of server.  Defaults to TCP AsyncServerSocket, but AsyncUDPServerSocket is also supported to receive udp messages.  In practice, ServerBootstrap is only useful for udp if you need to multiplex the messages across many threads, or have TCP connections going on at the same time, etc.  Simple usages of AsyncUDPSocket probably don&#039;t need the complexity of ServerBootstrap.</p>

<p><strong>pipeline(PipelineFactory&lt;AcceptPipeline&gt;)</strong></p>

<p>This pipeline method is used to get the accepted socket (or udp message) <strong>before</strong> it has been handed off to an IO thread.  This can be used to steer the accept thread to a particular thread, or for logging.</p>

<p>See also AcceptRoutingHandler and RoutingDataHandler for additional help in reading data off of the accepted socket <strong>before</strong> it gets attached to an IO thread.  These can be used to hash incoming sockets to specific threads.</p>

<p><strong>childHandler(AcceptorFactory)</strong></p>

<p>Previously facebook had lots of code that used AcceptorFactories instead of Pipelines, this is a method to support this code and be backwards compatible.  The AcceptorFactory is responsible for creating acceptors, setting up pipelines, setting up AsyncSocket read callbacks, etc.</p>

<h2 id="examples">Examples <a href="#examples" class="headerLink">#</a></h2>

<p>A simple example:</p>

<div class="remarkup-code-block" data-code-lang="php"><pre class="remarkup-code"><span class="no">ServerBootstrap</span><span class="o">&lt;</span><span class="no">TelnetPipeline</span><span class="o">&gt;</span> <span class="no">server</span><span class="o">;</span>                                                                                                      
<span class="no">server</span><span class="o">.</span><span class="nf" data-symbol-name="childPipeline">childPipeline</span><span class="o">(</span><span class="nc" data-symbol-name="std">std</span><span class="o">::</span><span class="na" data-symbol-context="std" data-symbol-name="make_shared">make_shared</span><span class="o">&lt;</span><span class="no">TelnetPipelineFactory</span><span class="o">&gt;());</span>                                                                             
<span class="no">server</span><span class="o">.</span><span class="nf" data-symbol-name="bind">bind</span><span class="o">(</span><span class="no">FLAGS_port</span><span class="o">);</span>                                                                                                                     
<span class="no">server</span><span class="o">.</span><span class="nf" data-symbol-name="waitForStop">waitForStop</span><span class="o">();</span></pre></div></section><section class="dex_document"><h1>ClientBootstrap</h1><p class="dex_introduction">Create clients easily</p><p>ClientBootstrap is a thin wrapper around AsyncSocket that provides a future interface to the connect callback, and a Pipeline interface to the read callback.</p>

<h2 id="methods">Methods <a href="#methods" class="headerLink">#</a></h2>

<p><strong>group(IOThreadPoolExecutor)</strong></p>

<p>Sets the thread or group of threads where the IO will take place.  Callbacks are also made on this thread.</p>

<p><strong>bind(port)</strong></p>

<p>Optionally bind to a specific port</p>

<p><strong>Future&lt;Pipeline*&gt; connect(SocketAddress)</strong></p>

<p>Connect to the selected address.  When the future is complete, the initialized pipeline will be returned.</p>

<div class="remarkup-note"><span class="remarkup-note-word">NOTE:</span> future.cancel() can be called to cancel an outstanding connection attempt.</div>

<p><strong>pipelineFactory(PipelineFactory&lt;Pipeline&gt;)</strong></p>

<p>Set the pipeline factory to use after a connection is successful.</p>

<h2 id="example">Example <a href="#example" class="headerLink">#</a></h2>

<div class="remarkup-code-block" data-code-lang="php"><pre class="remarkup-code"><span class="no">ClientBootstrap</span><span class="o">&lt;</span><span class="no">TelnetPipeline</span><span class="o">&gt;</span> <span class="no">client</span><span class="o">;</span>
<span class="no">client</span><span class="o">.</span><span class="nf" data-symbol-name="group">group</span><span class="o">(</span><span class="nc" data-symbol-name="std">std</span><span class="o">::</span><span class="na" data-symbol-context="std" data-symbol-name="make_shared">make_shared</span><span class="o">&lt;</span><span class="nc" data-symbol-name="folly">folly</span><span class="o">::</span><span class="na" data-symbol-context="folly" data-symbol-name="wangle">wangle</span><span class="o">::</span><span class="na" data-symbol-name="IOThreadPoolExecutor">IOThreadPoolExecutor</span><span class="o">&gt;(</span><span class="mi">1</span><span class="o">));</span>
<span class="no">client</span><span class="o">.</span><span class="nf" data-symbol-name="pipelineFactory">pipelineFactory</span><span class="o">(</span><span class="nc" data-symbol-name="std">std</span><span class="o">::</span><span class="na" data-symbol-context="std" data-symbol-name="make_shared">make_shared</span><span class="o">&lt;</span><span class="no">TelnetPipelineFactory</span><span class="o">&gt;());</span>
<span class="c">// synchronously wait for the connect to finish</span>
<span class="no">auto</span> <span class="no">pipeline</span> <span class="o">=</span> <span class="no">client</span><span class="o">.</span><span class="nf" data-symbol-name="connect">connect</span><span class="o">(</span><span class="nf" data-symbol-name="SocketAddress">SocketAddress</span><span class="o">(</span><span class="no">FLAGS_host</span><span class="o">,</span><span class="no">FLAGS_port</span><span class="o">)).</span><span class="nf" data-symbol-name="get">get</span><span class="o">();</span>

<span class="c">// close the pipeline when finished</span>
<span class="no">pipeline</span><span class="o">-&gt;</span><span class="na" data-symbol-name="close">close</span><span class="o">();</span></pre></div></section><section class="dex_document"><h1>Pipeline</h1><p class="dex_introduction">Send your socket data through a series of tubes</p><p>A Pipeline is a series of Handlers that intercept inbound or outbound events, giving full control over how events are handled.  Handlers can be added dynamically to the pipeline.</p>

<p>When events are called, a Context* object is passed to the Handler - this means state can be stored in the context object, and a single instantiation of any individual Handler can be used for the entire program.</p>

<p>Netty&#039;s documentation: <a href="http://netty.io/4.0/api/io/netty/channel/ChannelPipeline.html" target="_blank">ChannelHandler</a></p>

<p>Usually, the bottom of the Pipeline is a wangle::AsyncSocketHandler to read/write to a socket, but this isn&#039;t a requirement.</p>

<p>A pipeline is templated on the input and output types:</p>

<div class="remarkup-code-block" data-code-lang="php"><pre class="remarkup-code"><span class="no">EventBase</span> <span class="no">base_</span><span class="o">;</span>
<span class="no">Pipeline</span><span class="o">&lt;</span><span class="no">IOBufQueue</span><span class="o">&amp;,</span> <span class="nc" data-symbol-name="std">std</span><span class="o">::</span><span class="na" data-symbol-context="std" data-symbol-name="unique_ptr">unique_ptr</span><span class="o">&lt;</span><span class="no">IOBuf</span><span class="o">&gt;&gt;</span> <span class="no">pipeline</span><span class="o">;</span>
<span class="no">pipeline</span><span class="o">.</span><span class="nf" data-symbol-name="addBack">addBack</span><span class="o">(</span><span class="nf" data-symbol-name="AsyncSocketHandler">AsyncSocketHandler</span><span class="o">(</span><span class="nc" data-symbol-name="AsyncSocket">AsyncSocket</span><span class="o">::</span><span class="nf" data-symbol-context="AsyncSocket" data-symbol-name="newSocket">newSocket</span><span class="o">(</span><span class="no">eventBase</span><span class="o">)));</span></pre></div>

<p>The above creates a pipeline and adds a single AsyncSocket handler, that will push read events through the pipeline when the socket gets bytes.  Let&#039;s try handling some socket events:</p>

<div class="remarkup-code-block" data-code-lang="php"><pre class="remarkup-code"><span class="k">class</span> <span class="no">MyHandler</span> <span class="o">:</span> <span class="k">public</span> <span class="no">InboundHandler</span><span class="o">&lt;</span><span class="nc" data-symbol-name="folly">folly</span><span class="o">::</span><span class="na" data-symbol-context="folly" data-symbol-name="IOBufQueue">IOBufQueue</span><span class="o">&amp;&gt;</span> <span class="o">&#123;</span>
 <span class="k">public</span><span class="o">:</span>

  <span class="no">void</span> <span class="nf" data-symbol-name="read">read</span><span class="o">(</span><span class="no">Context</span><span class="o">*</span> <span class="no">ctx</span><span class="o">,</span> <span class="nc" data-symbol-name="folly">folly</span><span class="o">::</span><span class="na" data-symbol-context="folly" data-symbol-name="IOBufQueue">IOBufQueue</span><span class="o">&amp;</span> <span class="no">q</span><span class="o">)</span> <span class="no">override</span> <span class="o">&#123;</span>
    <span class="no">IOBufQueue</span> <span class="no">data</span><span class="o">;</span>   
    <span class="k">if</span> <span class="o">(</span><span class="no">q</span><span class="o">.</span><span class="nf" data-symbol-name="chainLength">chainLength</span><span class="o">()</span> <span class="o">&gt;=</span> <span class="mi">4</span><span class="o">)</span> <span class="o">&#123;</span>
       <span class="no">data</span><span class="o">.</span><span class="nf" data-symbol-name="append">append</span><span class="o">(</span><span class="no">q</span><span class="o">.</span><span class="nf" data-symbol-name="split">split</span><span class="o">(</span><span class="mi">4</span><span class="o">));</span>
       <span class="no">ctx</span><span class="o">-&gt;</span><span class="na" data-symbol-name="fireRead">fireRead</span><span class="o">(</span><span class="no">data</span><span class="o">);</span>
    <span class="o">&#125;</span> 
  <span class="o">&#125;</span>
<span class="o">&#125;;</span></pre></div>

<p>This handler only handles read (inbound) data, so we can inherit from InboundHandler, and ignore the outbound type (so the ordering of inbound/outbound handlers in your pipeline doesn&#039;t matter).   It checks if there are at least 4 bytes of data available, and if so, passes them on to the next handler.  If there aren&#039;t yet four bytes of data available, it does nothing, and waits for more data.</p>

<p>We can add this handler to our pipeline like so:</p>

<div class="remarkup-code-block" data-code-lang="php"><pre class="remarkup-code"><span class="nx">pipeline</span><span class="k">.</span><span data-symbol-name="addBack" class="nf">addBack</span><span class="k">(</span><span data-symbol-name="MyHandler" class="nf">MyHandler</span><span class="k">(</span><span class="k">)</span><span class="k">)</span><span class="k">;</span></pre></div>

<p>and remove it just as easily:</p>

<div class="remarkup-code-block" data-code-lang="php"><pre class="remarkup-code"><span class="no">pipeline</span><span class="o">.</span><span class="no">remove</span><span class="o">&lt;</span><span class="no">MyHandler</span><span class="o">&gt;();</span></pre></div>

<h3 id="staticpipeline">StaticPipeline <a href="#staticpipeline" class="headerLink">#</a></h3>

<p>Instantiating all these handlers and pipelines can hit the allocator pretty hard.  There are two ways to try to do fewer allocations.  StaticPipeline allows <strong>all</strong> the handlers, and the pipeline, to be instantiated all in the same memory block, so we only hit the allocator once.</p>

<p>The other option is to allocate the handlers once at startup, and reuse them in many pipelines.  This means all state has to be saved in the HandlerContext object instead of the Handler itself, since each handler can be in multiple pipelines.  There is one context per pipeline to get around this limitation.</p></section><section class="dex_document"><h1>Built-in handlers</h1><p class="dex_introduction">The stuff that comes with the box</p><h2 id="byte-to-byte-handlers">Byte to byte handlers <a href="#byte-to-byte-handlers" class="headerLink">#</a></h2>

<h3 id="asyncsockethandler">AsyncSocketHandler <a href="#asyncsockethandler" class="headerLink">#</a></h3>

<p>This is almost always the first handler in the pipeline for clients and servers - it connects an AsyncSocket to the pipeline.  Having it as a handler is nice, because mocking it out for tests becomes trivial.</p>

<h3 id="outputbufferinghandler">OutputBufferingHandler <a href="#outputbufferinghandler" class="headerLink">#</a></h3>

<p>Output is buffered and only sent once per event loop.  This logic is exactly what is in ThriftServer, and very similar to what exists in proxygen - it can improve throughput for small writes by up to 300%.</p>

<h3 id="eventbasehandler">EventBaseHandler <a href="#eventbasehandler" class="headerLink">#</a></h3>

<p>Putting this right after an AsyncSocketHandler means that writes can happen from any thread, and eventBase-&gt;runInEventBaseThread() will automatically be called to put them in the correct thread.  It doesn&#039;t intrinsically make the pipeline thread-safe though, writes from different threads may be interleaved, other handler stages must be only used from one thread or be thread safe, etc.</p>

<p>In addition, reads are still always called on the eventBase thread.</p>

<h2 id="codecs">Codecs <a href="#codecs" class="headerLink">#</a></h2>

<h3 id="fixedlengthframedecoder">FixedLengthFrameDecoder <a href="#fixedlengthframedecoder" class="headerLink">#</a></h3>

<p>A decoder that splits received IOBufs by a fixed number of bytes.  Used for fixed-length protocols</p>

<h3 id="lengthfieldprepender">LengthFieldPrepender <a href="#lengthfieldprepender" class="headerLink">#</a></h3>

<p>Prepends a fixed-length field length.  Field length is configurable.</p>

<h3 id="lengthfieldbasedframedec">LengthFieldBasedFrameDecoder <a href="#lengthfieldbasedframedec" class="headerLink">#</a></h3>

<p>The receiving portion of LengthFieldPrepender - decodes based on a fixed frame length, with optional header/tailer data sections.</p>

<h3 id="linebasedframedecoder">LineBasedFrameDecoder <a href="#linebasedframedecoder" class="headerLink">#</a></h3>

<p>Decodes by line (with optional ending detection types), to be used for text-based protocols</p>

<h3 id="stringcodec">StringCodec <a href="#stringcodec" class="headerLink">#</a></h3>

<p>Converts from IOBufs to std::strings and back for text-based protocols.  Must be used after one of the above frame decoders</p></section><section class="dex_document"><h1>Services</h1><p class="dex_introduction">How to add a new protocol</p><p><a href="https://twitter.github.io/finagle/guide/ServicesAndFilters.html" target="_blank">Finagle&#039;s documentation</a> on Services is highly recommended</p>

<h2 id="services">Services <a href="#services" class="headerLink">#</a></h2>

<p>A Pipeline was read() and write() methods - it streams bytes in one or both directions.  write() returns a future, but the future is set when the bytes are successfully written.   Using pipeline there is no easy way to match up requests and responses for RPC.</p>

<p>A Service is an RPC abstraction - Both clients and servers implement the interface.   Servers implement it by handling the request.  Clients implement it by sending the request to the server to complete.</p>

<p>A Dispatcher is the adapter between the Pipeline and Service that matches up the requests and responses.  There are several built in Dispatchers, however if you are doing anything advanced, you may need to write your own.</p>

<p>Because both clients and servers implement the same interface, mocking either clients or servers is trivially easy.</p>

<h2 id="servicefilters">ServiceFilters <a href="#servicefilters" class="headerLink">#</a></h2>

<p>ServiceFilters provide a way to wrap filters around every request and response.  Things like logging, timeouts, retrying requests, etc. can be implemented as ServiceFilters.</p>

<p>Existing ServiceFilters include:</p>

<ul>
<li>CloseOnReleaseFilter - rejects requests after connection is closed.  Often used in conjunction with</li>
<li>ExpiringFilter - idle timeout and max connection time (usually used for clients)</li>
<li>TimeoutFilter - request timeout time.  Usually used on servers.  Clients can use future.within to specify timeouts individually.</li>
<li>ExecutorFilter - move requests to a different executor.</li>
</ul>

<h2 id="servicefactories">ServiceFactories <a href="#servicefactories" class="headerLink">#</a></h2>

<p>For some services, a Factory can help instantiate clients.   In Finagle, these are frequently provided for easy use with specific protocols, i.e. http, memcache, etc.</p>

<h2 id="servicefactoryfilters">ServiceFactoryFilters <a href="#servicefactoryfilters" class="headerLink">#</a></h2>

<p>ServiceFactoryFilters provide filters for getting clients.  These include most connection-oriented things, like connection pooling, selection, dispatch, load balancing, etc.</p>

<p>Existing ServiceFactoryFilters:</p>

<ul>
<li></li>
<li></li>
</ul></section>
