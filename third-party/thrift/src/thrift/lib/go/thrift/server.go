/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package thrift

import (
	"context"
	"fmt"
	"net"
	"runtime"
	"runtime/debug"
	"strconv"
	"strings"
	"sync"
	"time"

	thriftstats "github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
)

const (
	// LoadHeaderKey is the header key for thrift headers where ServiceRouter
	// expects load to be reported for cached load.  You must configure this
	// in your SMC tier under the "load_counter" key.
	LoadHeaderKey = "load"

	// ClientTimeoutKey is the header key for thrift headers with ServiceRouter
	ClientTimeoutKey = "client_timeout"
	// ClientID is the header key for thrift headers with ServiceRouter
	ClientID = "client_id"
)

var taskExpiredException = NewApplicationException(UNKNOWN_APPLICATION_EXCEPTION, "Task Expired")

type server struct {
	processor   Processor
	pstats      map[string]*thriftstats.TimingSeries
	interceptor Interceptor
	ln          net.Listener
	wg          sync.WaitGroup

	// configuration
	reportError func(format string, args ...interface{})

	// pipelining mode will implement pipelining + out of order responses when possible.
	pipeliningEnabled bool

	// numWorkers is the number of goroutines to use in the worker pool in the case of pipelining.
	// this should be tuned to the nature of the work (CPU bound: numCPU, IO bound, many more)
	// this can be set to the special value GoroutinePerRequest to enable goroutine per request
	// instead of worker pool model.
	numWorkers int
	// channel to distribute work to the workers.
	workCh chan work

	stats *thriftstats.ServerStats

	connContext ConnContextFunc
}

// NewServer creates a new thrift server. It includes:
// * load shedding support
// * load balancing compatible with high QPS services
// * pipelining of incoming requests on same connection
// * out of order responses (for clients that support it!)
// * and statstics that you can export to your favorite monitoring system
func NewServer(processor Processor, listener net.Listener, transportType TransportID, options ...ServerOption) Server {
	if transportType != TransportIDHeader {
		panic(fmt.Sprintf("Server does not support: %v", transportType))
	}
	opts := newServerOptions(options...)
	// allocate a server with defaults
	server := &server{
		ln:                listener,
		processor:         processor,
		interceptor:       opts.interceptor,
		pstats:            opts.processorStats,
		reportError:       opts.log,
		pipeliningEnabled: opts.pipeliningEnabled,
		numWorkers:        opts.numWorkers,
		stats:             opts.serverStats,
		connContext:       opts.connContext,
	}

	return server
}

// This counter is what powers client side load balancing.
// loadFn is a function that reports system load.  It must report the
// server load as an unsigned integer.  Higher numbers mean the server
// is more loaded.  Clients choose the servers that report the lowest
// load.
// NOTE: if you run multiple servers with different capacities, you
// should ensure your load numbers are comparable and account for this
// (i.e. divide by NumCPU)
// NOTE: loadFn is called on every single response.  it should be fast.
func (tf *server) loadFn() uint {
	working := tf.stats.WorkingCount.Get() + tf.stats.SchedulingWorkCount.Get()
	denominator := float64(runtime.NumCPU())
	return uint(1000. * float64(working) / denominator)
}

var tooBusyResponse ApplicationException = NewApplicationException(
	UNKNOWN_APPLICATION_EXCEPTION,
	"server is too busy",
)

func (tf *server) serve(ctx context.Context) error {
	// add self
	tf.wg.Add(1)
	defer tf.wg.Done()

	if tf.ln == nil {
		return fmt.Errorf("ServeContext() called without Listen()")
	}

	if tf.pipeliningEnabled && tf.numWorkers != GoroutinePerRequest {
		// setup work queue for pipelining
		// XXX should we consider a tuneable, buffered work chan?
		tf.workCh = make(chan work, tf.numWorkers)
		// launch workers
		tf.wg.Add(tf.numWorkers)
		for i := 0; i < tf.numWorkers; i++ {
			go tf.worker()
		}
	}

	return tf.acceptLoop(ctx)
}

func (tf *server) acceptLoop(ctx context.Context) error {
	for {
		conn, err := tf.ln.Accept()
		if err != nil {
			// graceful shutdown (accept conn closed) is not an error
			if !strings.Contains(err.Error(), "use of closed network connection") {
				tf.reportError("during accept: %s", err)
				return err
			}
			return nil
		}
		// add connection info to ctx.
		ctx = tf.connContext(ctx, conn)
		go tf.processRequests(ctx, conn)
	}
}

// ServeContext enters the accept loop and processes requests.
func (tf *server) ServeContext(ctx context.Context) error {
	go func() {
		<-ctx.Done()
		tf.stop()
	}()
	return tf.serve(ctx)
}

func (tf *server) stop() error {
	if tf.ln == nil {
		return fmt.Errorf("not listening")
	}
	if err := tf.ln.Close(); err != nil {
		return err
	}

	// at least wait for our workers to clock out?
	if tf.workCh != nil {
		close(tf.workCh)
	}
	// currently not waiting for connection goroutines or goroutine per request threads.
	tf.wg.Wait()
	return nil
}

// used to recover from any application panics, and count accordingly.
func (tf *server) recoverProcessorPanic() {
	if err := recover(); err != nil {
		tf.reportError("panic in processor: %v: %s", err, debug.Stack())
		tf.stats.PanicCount.RecordEvent()
	}
}

// work represents one unit of application level work.
// it should handle when to decrement working count.
type work func()

// executes one work item.
func (tf *server) executeWork(w work) {
	func() {
		defer tf.recoverProcessorPanic()
		w()
	}()
}

// worker is a single worker goroutine for processing requests in the thrift server.
// number of workers are configurable, and should be tuned based on type of
// processing. If the nature of the processing is network bound with varying latency,
// one should prefer to have many more workers than cores. if its all cpu bound,
// worker per core should suffice. It's all about optimizing for keeping the
// writer per connection goroutines as busy as possible.
func (tf *server) worker() {
	defer tf.wg.Done()
	for w := range tf.workCh {
		tf.executeWork(w)
	}
}

func (tf *server) recordNewConnAndDefer(conn net.Conn) func() {
	// keep track of the number of connections established and closed
	// over time
	tf.stats.ConnsEstablished.RecordEvent()
	// update current connection count
	tf.stats.ConnCount.Incr()

	return func() {
		conn.Close() // all reads & writes will fail now
		tf.stats.ConnsClosed.RecordEvent()
		tf.stats.ConnCount.Decr()
	}
}

// reader is a dedicated goroutine handler per connection.
func (tf *server) processRequests(ctx context.Context, conn net.Conn) {
	defer tf.recordNewConnAndDefer(conn)()

	// create a cancellable ctx for this conn. any protocol error will cancel
	// and cleanup the reader and writer
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	// if pipelining, launch up a single writer goroutine to sequence responses to the conn
	// buffer this channel to ease the contention on heavy write spikes
	writeBufferSz := tf.numWorkers
	if writeBufferSz == GoroutinePerRequest {
		writeBufferSz = runtime.NumCPU()
	}
	writeCh := make(chan func() error, writeBufferSz)
	if tf.pipeliningEnabled {
		go tf.writer(ctx, writeCh, cancel)
	}

	// enter readloop, deferring any work to worker pool, and any writes to the writer
	tf.reader(ctx, conn, writeCh, cancel)
}

func (tf *server) writeMessage(
	prot Protocol, /* XXX configurable? */
	msg thriftMsg,
	response WritableStruct,
) error {
	// copy client's headers if applicable
	for k, v := range msg.headers {
		if k == LoadHeaderKey {
			continue // do not set a load header sent from the client.
		}
		prot.SetRequestHeader(k, v)
	}
	// *always* write our load header
	prot.SetRequestHeader(LoadHeaderKey, fmt.Sprintf("%d", tf.loadFn()))

	messageType := REPLY
	if _, isExc := response.(ApplicationException); isExc {
		messageType = EXCEPTION
	}
	// explicitly set seqid for the header to be what was given to us at read message begin.
	if protSeqID, ok := prot.(HeaderProtocolSeqID); ok {
		// only supported by HeaderProtocol
		protSeqID.SetSeqID(msg.headerSeqID)
	}
	err := prot.WriteMessageBegin(msg.name, messageType, msg.seqID)
	if err == nil && response != nil {
		err = response.Write(prot)
	}
	if err == nil {
		err = prot.WriteMessageEnd()
	}
	if err == nil {
		err = prot.Flush()
	}
	return err
}

// writer is a goroutine responsible for continuously sequencing the writes
// for a given conn.
func (tf *server) writer(ctx context.Context, writeCh chan func() error, cancel context.CancelFunc) {
	var err error

	for {
		select {
		case write := <-writeCh:
			err = write()
			if err != nil {
				tf.reportError("write error, closing connection: %s", err.Error())
				cancel() // this connection is toast, cancel ctx for the reader in case they are blocking
				return
			}
		case <-ctx.Done():
			return
		}
	}
}

type thriftMsg struct {
	name        string
	typ         MessageType
	seqID       int32
	headerSeqID uint32
	headers     map[string]string
}

// reader is a dedicated goroutine for reading from thrift transport.
// it has the ability to send request processing work, or serialize a write
// back to the conn immediately in the case that it didn't have anything to
// process. It will continue running as long as it can operate the thrift
// protocol on the transport, so if it returns, connection is unusable.
func (tf *server) reader(ctx context.Context, socket net.Conn, writeCh chan func() error, cancel context.CancelFunc) {
	var err error
	var proto Protocol
	proto, err = NewHeaderProtocol(socket)
	if err != nil {
		tf.reportError("error creating protocol: %s", err)
		tf.stats.ProtocolError.RecordEvent()
		return
	}

	prot := proto.(headerProtocolExtras)

	// readloop
	var requestNum int64
	for {
		var msg thriftMsg
		requestCtx := ctx // request specific context options will derive from the parent (reader on a connection) context
		// maintain the header seqid we received, as well as the msg seq id.
		msg.name, msg.typ, msg.seqID, err = prot.ReadMessageBegin()
		// add headers to the ctx. This allows thrift functions to inspect headers, at the cost of a gomap per request
		msg.headers = prot.GetResponseHeaders()
		msg.headerSeqID = prot.GetSeqID()
		if remainingMsStr, ok := msg.headers[ClientTimeoutKey]; ok {
			if remainingMs, err := strconv.Atoi(remainingMsStr); err == nil && remainingMs > 0 {
				requestCtx, _ = context.WithTimeout(ctx, time.Duration(1.1*float32(remainingMs))*time.Millisecond) // see links for the 1.1 magic number
			}
		}
		requestNum++
		tf.stats.RequestNum.RecordValue(requestNum)
		tf.stats.ReadingCount.Incr()
		startReadMessage := time.Now()
		if err != nil {
			if err, ok := err.(TransportException); ok && err.TypeID() == END_OF_FILE {
				tf.stats.ClientClosed.RecordEvent()
				tf.stats.ReadingCount.Decr()
				return
			}
			tf.reportError("error reading message begin from client %s: %s", msg.headers[ClientID], err)
			tf.stats.ProtocolError.RecordEvent()
			tf.stats.ReadingCount.Decr()
			return
		}

		requestCtx = WithHeaders(requestCtx, msg.headers)
		tf.scheduleResponse(ctx, requestCtx, prot, msg, writeCh, cancel, startReadMessage)

		// check signal from either writer goroutine, or us, that we need to bail.
		select {
		case <-ctx.Done():
			return
		default:
		}
		tf.stats.NotListening.Record(time.Since(startReadMessage))
	}
}

type headerProtocolExtras interface {
	Protocol
	HeaderProtocolFlags
	HeaderProtocolSeqID
}

// scheduleResponse will cancel ctx if we are unable to continue communication on this connection.
func (tf *server) scheduleResponse(ctx, rctx context.Context, prot headerProtocolExtras,
	msg thriftMsg, writeCh chan func() error, cancel context.CancelFunc, requestStart time.Time) {
	// responsible for writing a response or bailing out on cancelled ctx
	scheduleWrite := func(response WritableStruct) {
		writeScheduleBegin := time.Now()
		tf.stats.SchedulingWriteCount.Incr()
		write := func() error {
			tf.stats.SchedulingWriteCount.Decr()
			tf.stats.WritingCount.Incr()
			writeStart := time.Now()
			tf.stats.DurationScheduleWrite.Record(writeStart.Sub(writeScheduleBegin))
			defer func() {
				tf.stats.WritingCount.Decr()
				tf.stats.DurationWrite.Record(time.Since(writeStart))
			}()
			err := tf.writeMessage(prot, msg, response)
			if err != nil {
				tf.stats.ProtocolError.RecordEvent()
				// protocol error has happened. this connection is over.
				cancel()
				return err
			}
			tf.stats.TotalResponseTime.Record(time.Since(requestStart))
			return nil
		}
		// if we are not pipelining, write now.
		if !tf.pipeliningEnabled {
			write()
			return
		}
		// if we are pipelining, schedule the write to the writer goroutine
		select {
		case writeCh <- write:
		case <-ctx.Done():
			tf.stats.SchedulingWriteCount.Decr()
			// ctx is cancelled, this conn is done, shouldn't write
		}
	}

	// CASE 1 : check for invalid message type, protocol error
	if msg.typ != CALL && msg.typ != ONEWAY {
		// schedule the exception on the writer
		tf.stats.ReadingCount.Decr()
		scheduleWrite(NewApplicationException(INVALID_MESSAGE_TYPE_EXCEPTION,
			fmt.Sprintf("unexpected message type: %d", msg.typ)))
		return // successfully rejected it
	}

	// CASE 2 : check for invocation of unknown function
	processorFunc, ok := tf.processor.ProcessorFunctionMap()[msg.name]
	if !ok {
		var err error
		// suppress extremely noisy log about non-existent "upgradeToRocket" method
		if msg.name != "upgradeToRocket" {
			tf.reportError("non-existent method invoked: %s by client %s", msg.name, msg.headers[ClientID])
		}
		tf.stats.NoSuchFunction.RecordEvent()
		if err = prot.Skip(STRUCT); err != nil {
			tf.stats.ProtocolError.RecordEvent()
			tf.stats.ReadingCount.Decr()
			cancel()
			return
		}
		if err = prot.ReadMessageEnd(); err != nil {
			tf.stats.ProtocolError.RecordEvent()
			tf.stats.ReadingCount.Decr()
			cancel()
			return
		}
		// schedule the exception
		tf.stats.ReadingCount.Decr()
		scheduleWrite(NewApplicationException(UNKNOWN_METHOD, "Unknown function "+msg.name))
		return // successfully skipped unknown message type
	}

	// gets func with wrapped interceptors
	if processorFunc == nil {
		err := fmt.Errorf("function name %s does not exist", msg.name)
		tf.reportError("while reading %s message from client %s: %s", msg.name, msg.headers[ClientID], err)
		// terminate connection when we fail to read a protocol message.  it is a protocol
		// violation we cannot recover from
		tf.stats.ProtocolError.RecordEvent()
		tf.stats.ReadingCount.Decr()
		cancel()
		return
	}

	// CASE 3 : invocation of application supplied thrift handler (or loadshed)
	request, err := processorFunc.Read(prot)
	if err != nil {
		tf.reportError("while reading %s message from client %s: %s", msg.name, msg.headers[ClientID], err)
		// terminate connection when we fail to read a protocol message.  it is a protocol
		// violation we cannot recover from
		tf.stats.ProtocolError.RecordEvent()
		tf.stats.ReadingCount.Decr()
		cancel()
		return
	}

	// schedule the work to happen, then continue reading on this goroutine.
	// increment schedulign work count from now until the work starts.
	tf.stats.ReadingCount.Decr()
	tf.stats.SchedulingWorkCount.Incr()
	doneReading := time.Now()
	tf.stats.DurationRead.Record(doneReading.Sub(requestStart))
	workItem := func() {
		tf.stats.SchedulingWorkCount.Decr()
		startFunc := time.Now()
		tf.stats.DurationScheduleWork.Record(startFunc.Sub(doneReading))

		// before doing the work, see if scheduling it took too long
		select {
		case <-rctx.Done():
			if rctx.Err() == context.DeadlineExceeded {
				// send back a response that we couldn't even start the request in in time needed
				// if this happens, the server is very likely overloaded
				tf.stats.QueueingTimeout.RecordEvent()
				scheduleWrite(taskExpiredException)
				return
			}
			// connection must be closed, nothing to do.
			tf.stats.ConnectionPreemptedWork.RecordEvent()
			return
		default:
		}

		var response WritableStruct
		tf.stats.WorkingCount.Incr()
		defer func() {
			// done running, decrement working count before responding
			tf.stats.WorkingCount.Decr()
			tf.stats.DurationWorking.Record(time.Since(startFunc))
			scheduleWrite(response)
		}()
		// finally, lets process the request.
		var runErr ApplicationException
		response, runErr = processorFunc.RunContext(rctx, request)
		if pstats := tf.pstats[msg.name]; pstats != nil {
			durationFunc := time.Since(startFunc)
			pstats.RecordWithStatus(durationFunc, runErr == nil)
		}
		if runErr != nil {
			tf.reportError("error handling %s from client %s, message: %s", msg.name, msg.headers[ClientID], runErr)
			msg.headers["uex"] = errorType(runErr)
			msg.headers["uexw"] = runErr.Error()
			// if response is nil, send err as the application level error
			if response == nil {
				response = runErr
			}
			return
		}

		// If we got a structured exception back, write metadata about it into headers
		if rr, ok := response.(WritableResult); ok && rr.Exception() != nil {
			ex := rr.Exception()
			msg.headers["uex"] = errorType(ex)
			msg.headers["uexw"] = ex.Error()
		}
	}
	// Check if the client supports out of order responses.
	// If not, there is nothing to gain (and everything to lose) by pipelining.
	if (prot.GetFlags() & HeaderFlagSupportOutOfOrder) == 0 {
		tf.stats.PipeliningUnsupportedClient.RecordEvent()
		tf.executeWork(workItem)
		return
	}

	// if we are not pipelning, perform the work right here.
	if !tf.pipeliningEnabled {
		tf.executeWork(workItem)
		return
	}

	// check for "goroutine per request" semantics. If it is set,
	// execute the request execution in another goroutine
	if tf.numWorkers == GoroutinePerRequest {
		go tf.executeWork(workItem)
		return
	}

	// finally, we have some work to do, schedule the work item to the workers
	select {
	case tf.workCh <- workItem:
		// continue to read, we've scheduled the work to be done
	case <-ctx.Done():
		tf.stats.SchedulingWorkCount.Decr()
		return
	default:
		// else, we can't loadshed, so blocking wait on workers. we might become overloaded.
		tf.stats.WorkersBusy.RecordEvent()
		select {
		case tf.workCh <- workItem:
			// continue to read, we've scheduled the work to be done
		case <-ctx.Done():
			tf.stats.SchedulingWorkCount.Decr()
			return
		}
	}
}
