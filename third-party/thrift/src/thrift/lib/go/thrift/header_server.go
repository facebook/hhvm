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
	"errors"
	"fmt"
	"net"
	"runtime"
	"runtime/debug"
	"strconv"
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

type server struct {
	processor   Processor
	listener    net.Listener
	log         func(format string, args ...any)
	connContext ConnContextFunc

	wg sync.WaitGroup
	// pipelining mode will implement pipelining + out of order responses when possible.
	pipeliningEnabled bool
	// numWorkers is the number of goroutines to use in the worker pool in the case of pipelining.
	// this should be tuned to the nature of the work (CPU bound: numCPU, IO bound, many more)
	// this can be set to the special value GoroutinePerRequest to enable goroutine per request
	// instead of worker pool model.
	numWorkers int
	// channel to distribute work to the workers.
	workCh chan work

	stats  *thriftstats.ServerStats
	pstats map[string]*thriftstats.TimingSeries
}

// newHeaderServer creates a new thrift server. It includes:
// * load shedding support
// * load balancing compatible with high QPS services
// * pipelining of incoming requests on same connection
// * out of order responses (for clients that support it!)
// * and statstics that you can export to your favorite monitoring system
func newHeaderServer(processor Processor, listener net.Listener, opts *serverOptions) Server {
	return &server{
		processor:   processor,
		listener:    listener,
		log:         opts.log,
		connContext: opts.connContext,

		pipeliningEnabled: opts.pipeliningEnabled,
		numWorkers:        opts.numWorkers,

		pstats: opts.processorStats,
		stats:  opts.serverStats,
	}
}

// ServeContext enters the accept loop and processes requests.
func (s *server) ServeContext(ctx context.Context) error {
	go func() {
		<-ctx.Done()
		s.listener.Close()
		// currently not waiting for connection goroutines or goroutine per request threads.
		s.wg.Wait()
		return
	}()
	// add self
	s.wg.Add(1)
	defer s.wg.Done()

	if s.pipeliningEnabled && s.numWorkers != GoroutinePerRequest {
		// setup work queue for pipelining
		// XXX should we consider a tuneable, buffered work chan?
		s.workCh = make(chan work, s.numWorkers)
		// launch workers
		s.wg.Add(s.numWorkers)
		for i := 0; i < s.numWorkers; i++ {
			go s.worker(ctx)
		}
	}

	err := s.acceptLoop(ctx)
	if ctx.Err() != nil {
		return ctx.Err()
	}
	return err
}

func (s *server) acceptLoop(ctx context.Context) error {
	for {
		conn, err := s.listener.Accept()
		if err != nil {
			// graceful shutdown (accept conn closed) is not an error
			if errors.Is(err, net.ErrClosed) {
				return nil
			}
			s.log("during accept: %s", err)
			return err
		}
		// add connection info to ctx.
		connCtx := s.connContext(ctx, conn)
		go s.processRequests(connCtx, conn)
	}
}

// used to recover from any application panics, and count accordingly.
func (s *server) recoverProcessorPanic() {
	if err := recover(); err != nil {
		s.log("panic in processor: %v: %s", err, debug.Stack())
		s.stats.PanicCount.RecordEvent()
	}
}

// work represents one unit of application level work.
// it should handle when to decrement working count.
type work func()

// executes one work item.
func (s *server) executeWork(w work) {
	func() {
		defer s.recoverProcessorPanic()
		w()
	}()
}

// worker is a single worker goroutine for processing requests in the thrift server.
// number of workers are configurable, and should be tuned based on type of
// processing. If the nature of the processing is network bound with varying latency,
// one should prefer to have many more workers than cores. if its all cpu bound,
// worker per core should suffice. It's all about optimizing for keeping the
// writer per connection goroutines as busy as possible.
func (s *server) worker(ctx context.Context) {
	defer s.wg.Done()
	for {
		select {
		case <-ctx.Done():
			return
		case w := <-s.workCh:
			s.executeWork(w)
		}
	}
}

func (s *server) recordNewConnAndDefer(conn net.Conn) func() {
	// keep track of the number of connections established and closed
	// over time
	s.stats.ConnsEstablished.RecordEvent()
	// update current connection count
	s.stats.ConnCount.Incr()

	return func() {
		conn.Close() // all reads & writes will fail now
		s.stats.ConnsClosed.RecordEvent()
		s.stats.ConnCount.Decr()
	}
}

// reader is a dedicated goroutine handler per connection.
func (s *server) processRequests(ctx context.Context, conn net.Conn) {
	defer s.recordNewConnAndDefer(conn)()

	// create a cancellable ctx for this conn. any protocol error will cancel
	// and cleanup the reader and writer
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	// if pipelining, launch up a single writer goroutine to sequence responses to the conn
	// buffer this channel to ease the contention on heavy write spikes
	writeBufferSz := s.numWorkers
	if writeBufferSz == GoroutinePerRequest {
		writeBufferSz = runtime.NumCPU()
	}
	writeCh := make(chan func() error, writeBufferSz)
	if s.pipeliningEnabled {
		go s.writer(ctx, writeCh, cancel)
	}

	// enter readloop, deferring any work to worker pool, and any writes to the writer
	s.reader(ctx, conn, writeCh, cancel)
}

func (s *server) writeMessage(
	prot Protocol, /* XXX configurable? */
	msg thriftMsg,
	response WritableStruct,
) error {
	// copy client's headers if applicable
	for k, v := range msg.headers {
		if k == LoadHeaderKey {
			continue // do not set a load header sent from the client.
		}
		prot.setRequestHeader(k, v)
	}
	// *always* write our load header
	prot.setRequestHeader(LoadHeaderKey, fmt.Sprintf("%d", loadFn(s.stats)))

	messageType := REPLY
	if _, isExc := response.(ApplicationExceptionIf); isExc {
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
func (s *server) writer(ctx context.Context, writeCh chan func() error, cancel context.CancelFunc) {
	var err error

	for {
		select {
		case write := <-writeCh:
			err = write()
			if err != nil {
				s.log("write error, closing connection: %s", err.Error())
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
func (s *server) reader(ctx context.Context, socket net.Conn, writeCh chan func() error, cancel context.CancelFunc) {
	var err error
	var proto Protocol
	proto, err = NewHeaderProtocol(socket)
	if err != nil {
		s.log("error creating protocol: %s", err)
		s.stats.ProtocolError.RecordEvent()
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
		msg.headers = prot.getResponseHeaders()
		msg.headerSeqID = prot.GetSeqID()
		if remainingMsStr, ok := msg.headers[ClientTimeoutKey]; ok {
			if remainingMs, err := strconv.Atoi(remainingMsStr); err == nil && remainingMs > 0 {
				requestCtx, _ = context.WithTimeout(ctx, time.Duration(1.1*float32(remainingMs))*time.Millisecond) // see links for the 1.1 magic number
			}
		}
		requestNum++
		s.stats.RequestNum.RecordValue(requestNum)
		s.stats.ReadingCount.Incr()
		startReadMessage := time.Now()
		if err != nil {
			if isEOF(err) {
				s.stats.ClientClosed.RecordEvent()
				s.stats.ReadingCount.Decr()
				return
			}
			s.log("error reading message begin from client %s: %s", msg.headers[ClientID], err)
			s.stats.ProtocolError.RecordEvent()
			s.stats.ReadingCount.Decr()
			return
		}

		requestCtx = WithHeaders(requestCtx, msg.headers)
		s.scheduleResponse(ctx, requestCtx, prot, msg, writeCh, cancel, startReadMessage)

		// check signal from either writer goroutine, or us, that we need to bail.
		select {
		case <-ctx.Done():
			return
		default:
		}
		s.stats.NotListening.Record(time.Since(startReadMessage))
	}
}

type headerProtocolExtras interface {
	Protocol
	HeaderProtocolFlags
	HeaderProtocolSeqID
}

// scheduleResponse will cancel ctx if we are unable to continue communication on this connection.
func (s *server) scheduleResponse(ctx, rctx context.Context, prot headerProtocolExtras,
	msg thriftMsg, writeCh chan func() error, cancel context.CancelFunc, requestStart time.Time) {
	// responsible for writing a response or bailing out on cancelled ctx
	scheduleWrite := func(response WritableStruct) {
		writeScheduleBegin := time.Now()
		s.stats.SchedulingWriteCount.Incr()
		write := func() error {
			s.stats.SchedulingWriteCount.Decr()
			s.stats.WritingCount.Incr()
			writeStart := time.Now()
			s.stats.DurationScheduleWrite.Record(writeStart.Sub(writeScheduleBegin))
			defer func() {
				s.stats.WritingCount.Decr()
				s.stats.DurationWrite.Record(time.Since(writeStart))
			}()
			err := s.writeMessage(prot, msg, response)
			if err != nil {
				s.stats.ProtocolError.RecordEvent()
				// protocol error has happened. this connection is over.
				cancel()
				return err
			}
			s.stats.TotalResponseTime.Record(time.Since(requestStart))
			return nil
		}
		// if we are not pipelining, write now.
		if !s.pipeliningEnabled {
			write()
			return
		}
		// if we are pipelining, schedule the write to the writer goroutine
		select {
		case writeCh <- write:
		case <-ctx.Done():
			s.stats.SchedulingWriteCount.Decr()
			// ctx is cancelled, this conn is done, shouldn't write
		}
	}

	// CASE 1 : check for invalid message type, protocol error
	if msg.typ != CALL && msg.typ != ONEWAY {
		// schedule the exception on the writer
		s.stats.ReadingCount.Decr()
		scheduleWrite(NewApplicationException(INVALID_MESSAGE_TYPE_EXCEPTION,
			fmt.Sprintf("unexpected message type: %d", msg.typ)))
		return // successfully rejected it
	}

	// CASE 2 : check for invocation of unknown function
	processorFunc, ok := s.processor.ProcessorFunctionMap()[msg.name]
	if !ok {
		var err error
		// suppress extremely noisy log about non-existent "upgradeToRocket" method
		if msg.name != "upgradeToRocket" {
			s.log("non-existent method invoked: %s by client %s", msg.name, msg.headers[ClientID])
		}
		s.stats.NoSuchFunction.RecordEvent()
		if err = prot.Skip(STRUCT); err != nil {
			s.stats.ProtocolError.RecordEvent()
			s.stats.ReadingCount.Decr()
			cancel()
			return
		}
		if err = prot.ReadMessageEnd(); err != nil {
			s.stats.ProtocolError.RecordEvent()
			s.stats.ReadingCount.Decr()
			cancel()
			return
		}
		// schedule the exception
		s.stats.ReadingCount.Decr()
		scheduleWrite(NewApplicationException(UNKNOWN_METHOD, "Unknown function "+msg.name))
		return // successfully skipped unknown message type
	}

	// gets func with wrapped interceptors
	if processorFunc == nil {
		err := fmt.Errorf("function name %s does not exist", msg.name)
		s.log("while reading %s message from client %s: %s", msg.name, msg.headers[ClientID], err)
		// terminate connection when we fail to read a protocol message.  it is a protocol
		// violation we cannot recover from
		s.stats.ProtocolError.RecordEvent()
		s.stats.ReadingCount.Decr()
		cancel()
		return
	}

	// CASE 3 : invocation of application supplied thrift handler (or loadshed)
	request, err := processorFunc.Read(prot)
	if err != nil {
		s.log("while reading %s message from client %s: %s", msg.name, msg.headers[ClientID], err)
		// terminate connection when we fail to read a protocol message.  it is a protocol
		// violation we cannot recover from
		s.stats.ProtocolError.RecordEvent()
		s.stats.ReadingCount.Decr()
		cancel()
		return
	}

	// schedule the work to happen, then continue reading on this goroutine.
	// increment schedulign work count from now until the work starts.
	s.stats.ReadingCount.Decr()
	s.stats.SchedulingWorkCount.Incr()
	doneReading := time.Now()
	s.stats.DurationRead.Record(doneReading.Sub(requestStart))
	workItem := func() {
		s.stats.SchedulingWorkCount.Decr()
		startFunc := time.Now()
		s.stats.DurationScheduleWork.Record(startFunc.Sub(doneReading))

		// before doing the work, see if scheduling it took too long
		select {
		case <-rctx.Done():
			if rctx.Err() == context.DeadlineExceeded {
				// send back a response that we couldn't even start the request in in time needed
				// if this happens, the server is very likely overloaded
				s.stats.QueueingTimeout.RecordEvent()
				scheduleWrite(taskExpiredError)
				return
			}
			// connection must be closed, nothing to do.
			s.stats.ConnectionPreemptedWork.RecordEvent()
			return
		default:
		}

		var response WritableStruct
		s.stats.WorkingCount.Incr()
		defer func() {
			// done running, decrement working count before responding
			s.stats.WorkingCount.Decr()
			s.stats.DurationWorking.Record(time.Since(startFunc))
			scheduleWrite(response)
		}()
		// finally, lets process the request.
		var runErr ApplicationExceptionIf
		response, runErr = processorFunc.RunContext(rctx, request)
		if pstats := s.pstats[msg.name]; pstats != nil {
			durationFunc := time.Since(startFunc)
			pstats.RecordWithStatus(durationFunc, runErr == nil)
		}
		if runErr != nil {
			s.log("error handling %s from client %s, message: %s", msg.name, msg.headers[ClientID], runErr)
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
		s.stats.PipeliningUnsupportedClient.RecordEvent()
		s.executeWork(workItem)
		return
	}

	// if we are not pipelning, perform the work right here.
	if !s.pipeliningEnabled {
		s.executeWork(workItem)
		return
	}

	// check for "goroutine per request" semantics. If it is set,
	// execute the request execution in another goroutine
	if s.numWorkers == GoroutinePerRequest {
		go s.executeWork(workItem)
		return
	}

	// finally, we have some work to do, schedule the work item to the workers
	select {
	case s.workCh <- workItem:
		// continue to read, we've scheduled the work to be done
	case <-ctx.Done():
		s.stats.SchedulingWorkCount.Decr()
		return
	default:
		// else, we can't loadshed, so blocking wait on workers. we might become overloaded.
		s.stats.WorkersBusy.RecordEvent()
		select {
		case s.workCh <- workItem:
			// continue to read, we've scheduled the work to be done
		case <-ctx.Done():
			s.stats.SchedulingWorkCount.Decr()
			return
		}
	}
}
