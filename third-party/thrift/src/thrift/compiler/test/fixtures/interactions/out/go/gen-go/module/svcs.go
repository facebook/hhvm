// Autogenerated by Thrift for thrift/compiler/test/fixtures/interactions/src/module.thrift
//
// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
//  @generated

package module


import (
    "context"
    "errors"
    "fmt"
    "io"
    "reflect"

    shared "shared"
    thrift "github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
    metadata "github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

var _ = shared.GoUnusedProtection__
// (needed to ensure safety because of naive import list construction)
var _ = context.Background
var _ = errors.New
var _ = fmt.Printf
var _ = io.EOF
var _ = reflect.Ptr
var _ = thrift.VOID
var _ = metadata.GoUnusedProtection__

type MyService interface {
    Foo(ctx context.Context) (error)
}

type MyServiceClientInterface interface {
    io.Closer
    Foo(ctx context.Context) (error)
}

type MyServiceClient struct {
    ch thrift.RequestChannel
}
// Compile time interface enforcer
var _ MyServiceClientInterface = (*MyServiceClient)(nil)

func NewMyServiceChannelClient(channel thrift.RequestChannel) *MyServiceClient {
    return &MyServiceClient{
        ch: channel,
    }
}

func (c *MyServiceClient) Close() error {
    return c.ch.Close()
}

func (c *MyServiceClient) Foo(ctx context.Context) (error) {
    fbthriftReq := &reqMyServiceFoo{
    }
    fbthriftResp := newRespMyServiceFoo()
    fbthriftErr := c.ch.SendRequestResponse(ctx, "foo", fbthriftReq, fbthriftResp)
    if fbthriftErr != nil {
        return fbthriftErr
    } else if fbthriftEx := fbthriftResp.Exception(); fbthriftEx != nil {
        return fbthriftEx
    }
    return nil
}


type MyServiceProcessor struct {
    processorFunctionMap map[string]thrift.ProcessorFunction
    functionServiceMap   map[string]string
    handler              MyService
}

func NewMyServiceProcessor(handler MyService) *MyServiceProcessor {
    p := &MyServiceProcessor{
        handler:              handler,
        processorFunctionMap: make(map[string]thrift.ProcessorFunction),
        functionServiceMap:   make(map[string]string),
    }
    p.AddToProcessorFunctionMap("foo", &procFuncMyServiceFoo{handler: handler})
    p.AddToFunctionServiceMap("foo", "MyService")

    return p
}

func (p *MyServiceProcessor) AddToProcessorFunctionMap(key string, processorFunction thrift.ProcessorFunction) {
    p.processorFunctionMap[key] = processorFunction
}

func (p *MyServiceProcessor) AddToFunctionServiceMap(key, service string) {
    p.functionServiceMap[key] = service
}

func (p *MyServiceProcessor) GetProcessorFunction(key string) (processor thrift.ProcessorFunction) {
    return p.processorFunctionMap[key]
}

func (p *MyServiceProcessor) ProcessorFunctionMap() map[string]thrift.ProcessorFunction {
    return p.processorFunctionMap
}

func (p *MyServiceProcessor) FunctionServiceMap() map[string]string {
    return p.functionServiceMap
}

func (p *MyServiceProcessor) PackageName() string {
    return "module"
}

func (p *MyServiceProcessor) GetThriftMetadata() *metadata.ThriftMetadata {
    return GetThriftMetadataForService("module.MyService")
}


type procFuncMyServiceFoo struct {
    handler MyService
}
// Compile time interface enforcer
var _ thrift.ProcessorFunction = (*procFuncMyServiceFoo)(nil)

func (p *procFuncMyServiceFoo) Read(decoder thrift.Decoder) (thrift.Struct, error) {
    args := newReqMyServiceFoo()
    if err := args.Read(decoder); err != nil {
        return nil, err
    }
    decoder.ReadMessageEnd()
    return args, nil
}

func (p *procFuncMyServiceFoo) Write(seqId int32, result thrift.WritableStruct, encoder thrift.Encoder) (err error) {
    var err2 error
    messageType := thrift.REPLY
    switch result.(type) {
    case thrift.ApplicationExceptionIf:
        messageType = thrift.EXCEPTION
    }

    if err2 = encoder.WriteMessageBegin("foo", messageType, seqId); err2 != nil {
        err = err2
    }
    if err2 = result.Write(encoder); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.WriteMessageEnd(); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.Flush(); err == nil && err2 != nil {
        err = err2
    }
    return err
}

func (p *procFuncMyServiceFoo) RunContext(ctx context.Context, reqStruct thrift.Struct) (thrift.WritableStruct, thrift.ApplicationExceptionIf) {
    result := newRespMyServiceFoo()
    err := p.handler.Foo(ctx)
    if err != nil {
        x := thrift.NewApplicationExceptionCause(thrift.INTERNAL_ERROR, "Internal error processing Foo: " + err.Error(), err)
        return x, x
    }

    return result, nil
}


type Factories interface {
    Foo(ctx context.Context) (error)
}

type FactoriesClientInterface interface {
    io.Closer
    Foo(ctx context.Context) (error)
}

type FactoriesClient struct {
    ch thrift.RequestChannel
}
// Compile time interface enforcer
var _ FactoriesClientInterface = (*FactoriesClient)(nil)

func NewFactoriesChannelClient(channel thrift.RequestChannel) *FactoriesClient {
    return &FactoriesClient{
        ch: channel,
    }
}

func (c *FactoriesClient) Close() error {
    return c.ch.Close()
}

func (c *FactoriesClient) Foo(ctx context.Context) (error) {
    fbthriftReq := &reqFactoriesFoo{
    }
    fbthriftResp := newRespFactoriesFoo()
    fbthriftErr := c.ch.SendRequestResponse(ctx, "foo", fbthriftReq, fbthriftResp)
    if fbthriftErr != nil {
        return fbthriftErr
    } else if fbthriftEx := fbthriftResp.Exception(); fbthriftEx != nil {
        return fbthriftEx
    }
    return nil
}


type FactoriesProcessor struct {
    processorFunctionMap map[string]thrift.ProcessorFunction
    functionServiceMap   map[string]string
    handler              Factories
}

func NewFactoriesProcessor(handler Factories) *FactoriesProcessor {
    p := &FactoriesProcessor{
        handler:              handler,
        processorFunctionMap: make(map[string]thrift.ProcessorFunction),
        functionServiceMap:   make(map[string]string),
    }
    p.AddToProcessorFunctionMap("foo", &procFuncFactoriesFoo{handler: handler})
    p.AddToFunctionServiceMap("foo", "Factories")

    return p
}

func (p *FactoriesProcessor) AddToProcessorFunctionMap(key string, processorFunction thrift.ProcessorFunction) {
    p.processorFunctionMap[key] = processorFunction
}

func (p *FactoriesProcessor) AddToFunctionServiceMap(key, service string) {
    p.functionServiceMap[key] = service
}

func (p *FactoriesProcessor) GetProcessorFunction(key string) (processor thrift.ProcessorFunction) {
    return p.processorFunctionMap[key]
}

func (p *FactoriesProcessor) ProcessorFunctionMap() map[string]thrift.ProcessorFunction {
    return p.processorFunctionMap
}

func (p *FactoriesProcessor) FunctionServiceMap() map[string]string {
    return p.functionServiceMap
}

func (p *FactoriesProcessor) PackageName() string {
    return "module"
}

func (p *FactoriesProcessor) GetThriftMetadata() *metadata.ThriftMetadata {
    return GetThriftMetadataForService("module.Factories")
}


type procFuncFactoriesFoo struct {
    handler Factories
}
// Compile time interface enforcer
var _ thrift.ProcessorFunction = (*procFuncFactoriesFoo)(nil)

func (p *procFuncFactoriesFoo) Read(decoder thrift.Decoder) (thrift.Struct, error) {
    args := newReqFactoriesFoo()
    if err := args.Read(decoder); err != nil {
        return nil, err
    }
    decoder.ReadMessageEnd()
    return args, nil
}

func (p *procFuncFactoriesFoo) Write(seqId int32, result thrift.WritableStruct, encoder thrift.Encoder) (err error) {
    var err2 error
    messageType := thrift.REPLY
    switch result.(type) {
    case thrift.ApplicationExceptionIf:
        messageType = thrift.EXCEPTION
    }

    if err2 = encoder.WriteMessageBegin("foo", messageType, seqId); err2 != nil {
        err = err2
    }
    if err2 = result.Write(encoder); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.WriteMessageEnd(); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.Flush(); err == nil && err2 != nil {
        err = err2
    }
    return err
}

func (p *procFuncFactoriesFoo) RunContext(ctx context.Context, reqStruct thrift.Struct) (thrift.WritableStruct, thrift.ApplicationExceptionIf) {
    result := newRespFactoriesFoo()
    err := p.handler.Foo(ctx)
    if err != nil {
        x := thrift.NewApplicationExceptionCause(thrift.INTERNAL_ERROR, "Internal error processing Foo: " + err.Error(), err)
        return x, x
    }

    return result, nil
}


type Perform interface {
    Foo(ctx context.Context) (error)
}

type PerformClientInterface interface {
    io.Closer
    Foo(ctx context.Context) (error)
}

type PerformClient struct {
    ch thrift.RequestChannel
}
// Compile time interface enforcer
var _ PerformClientInterface = (*PerformClient)(nil)

func NewPerformChannelClient(channel thrift.RequestChannel) *PerformClient {
    return &PerformClient{
        ch: channel,
    }
}

func (c *PerformClient) Close() error {
    return c.ch.Close()
}

func (c *PerformClient) Foo(ctx context.Context) (error) {
    fbthriftReq := &reqPerformFoo{
    }
    fbthriftResp := newRespPerformFoo()
    fbthriftErr := c.ch.SendRequestResponse(ctx, "foo", fbthriftReq, fbthriftResp)
    if fbthriftErr != nil {
        return fbthriftErr
    } else if fbthriftEx := fbthriftResp.Exception(); fbthriftEx != nil {
        return fbthriftEx
    }
    return nil
}


type PerformProcessor struct {
    processorFunctionMap map[string]thrift.ProcessorFunction
    functionServiceMap   map[string]string
    handler              Perform
}

func NewPerformProcessor(handler Perform) *PerformProcessor {
    p := &PerformProcessor{
        handler:              handler,
        processorFunctionMap: make(map[string]thrift.ProcessorFunction),
        functionServiceMap:   make(map[string]string),
    }
    p.AddToProcessorFunctionMap("foo", &procFuncPerformFoo{handler: handler})
    p.AddToFunctionServiceMap("foo", "Perform")

    return p
}

func (p *PerformProcessor) AddToProcessorFunctionMap(key string, processorFunction thrift.ProcessorFunction) {
    p.processorFunctionMap[key] = processorFunction
}

func (p *PerformProcessor) AddToFunctionServiceMap(key, service string) {
    p.functionServiceMap[key] = service
}

func (p *PerformProcessor) GetProcessorFunction(key string) (processor thrift.ProcessorFunction) {
    return p.processorFunctionMap[key]
}

func (p *PerformProcessor) ProcessorFunctionMap() map[string]thrift.ProcessorFunction {
    return p.processorFunctionMap
}

func (p *PerformProcessor) FunctionServiceMap() map[string]string {
    return p.functionServiceMap
}

func (p *PerformProcessor) PackageName() string {
    return "module"
}

func (p *PerformProcessor) GetThriftMetadata() *metadata.ThriftMetadata {
    return GetThriftMetadataForService("module.Perform")
}


type procFuncPerformFoo struct {
    handler Perform
}
// Compile time interface enforcer
var _ thrift.ProcessorFunction = (*procFuncPerformFoo)(nil)

func (p *procFuncPerformFoo) Read(decoder thrift.Decoder) (thrift.Struct, error) {
    args := newReqPerformFoo()
    if err := args.Read(decoder); err != nil {
        return nil, err
    }
    decoder.ReadMessageEnd()
    return args, nil
}

func (p *procFuncPerformFoo) Write(seqId int32, result thrift.WritableStruct, encoder thrift.Encoder) (err error) {
    var err2 error
    messageType := thrift.REPLY
    switch result.(type) {
    case thrift.ApplicationExceptionIf:
        messageType = thrift.EXCEPTION
    }

    if err2 = encoder.WriteMessageBegin("foo", messageType, seqId); err2 != nil {
        err = err2
    }
    if err2 = result.Write(encoder); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.WriteMessageEnd(); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.Flush(); err == nil && err2 != nil {
        err = err2
    }
    return err
}

func (p *procFuncPerformFoo) RunContext(ctx context.Context, reqStruct thrift.Struct) (thrift.WritableStruct, thrift.ApplicationExceptionIf) {
    result := newRespPerformFoo()
    err := p.handler.Foo(ctx)
    if err != nil {
        x := thrift.NewApplicationExceptionCause(thrift.INTERNAL_ERROR, "Internal error processing Foo: " + err.Error(), err)
        return x, x
    }

    return result, nil
}


type InteractWithShared interface {
    DoSomeSimilarThings(ctx context.Context) (*shared.DoSomethingResult, error)
}

type InteractWithSharedClientInterface interface {
    io.Closer
    DoSomeSimilarThings(ctx context.Context) (*shared.DoSomethingResult, error)
}

type InteractWithSharedClient struct {
    ch thrift.RequestChannel
}
// Compile time interface enforcer
var _ InteractWithSharedClientInterface = (*InteractWithSharedClient)(nil)

func NewInteractWithSharedChannelClient(channel thrift.RequestChannel) *InteractWithSharedClient {
    return &InteractWithSharedClient{
        ch: channel,
    }
}

func (c *InteractWithSharedClient) Close() error {
    return c.ch.Close()
}

func (c *InteractWithSharedClient) DoSomeSimilarThings(ctx context.Context) (*shared.DoSomethingResult, error) {
    fbthriftReq := &reqInteractWithSharedDoSomeSimilarThings{
    }
    fbthriftResp := newRespInteractWithSharedDoSomeSimilarThings()
    fbthriftErr := c.ch.SendRequestResponse(ctx, "do_some_similar_things", fbthriftReq, fbthriftResp)
    if fbthriftErr != nil {
        return nil, fbthriftErr
    } else if fbthriftEx := fbthriftResp.Exception(); fbthriftEx != nil {
        return nil, fbthriftEx
    }
    return fbthriftResp.GetSuccess(), nil
}


type InteractWithSharedProcessor struct {
    processorFunctionMap map[string]thrift.ProcessorFunction
    functionServiceMap   map[string]string
    handler              InteractWithShared
}

func NewInteractWithSharedProcessor(handler InteractWithShared) *InteractWithSharedProcessor {
    p := &InteractWithSharedProcessor{
        handler:              handler,
        processorFunctionMap: make(map[string]thrift.ProcessorFunction),
        functionServiceMap:   make(map[string]string),
    }
    p.AddToProcessorFunctionMap("do_some_similar_things", &procFuncInteractWithSharedDoSomeSimilarThings{handler: handler})
    p.AddToFunctionServiceMap("do_some_similar_things", "InteractWithShared")

    return p
}

func (p *InteractWithSharedProcessor) AddToProcessorFunctionMap(key string, processorFunction thrift.ProcessorFunction) {
    p.processorFunctionMap[key] = processorFunction
}

func (p *InteractWithSharedProcessor) AddToFunctionServiceMap(key, service string) {
    p.functionServiceMap[key] = service
}

func (p *InteractWithSharedProcessor) GetProcessorFunction(key string) (processor thrift.ProcessorFunction) {
    return p.processorFunctionMap[key]
}

func (p *InteractWithSharedProcessor) ProcessorFunctionMap() map[string]thrift.ProcessorFunction {
    return p.processorFunctionMap
}

func (p *InteractWithSharedProcessor) FunctionServiceMap() map[string]string {
    return p.functionServiceMap
}

func (p *InteractWithSharedProcessor) PackageName() string {
    return "module"
}

func (p *InteractWithSharedProcessor) GetThriftMetadata() *metadata.ThriftMetadata {
    return GetThriftMetadataForService("module.InteractWithShared")
}


type procFuncInteractWithSharedDoSomeSimilarThings struct {
    handler InteractWithShared
}
// Compile time interface enforcer
var _ thrift.ProcessorFunction = (*procFuncInteractWithSharedDoSomeSimilarThings)(nil)

func (p *procFuncInteractWithSharedDoSomeSimilarThings) Read(decoder thrift.Decoder) (thrift.Struct, error) {
    args := newReqInteractWithSharedDoSomeSimilarThings()
    if err := args.Read(decoder); err != nil {
        return nil, err
    }
    decoder.ReadMessageEnd()
    return args, nil
}

func (p *procFuncInteractWithSharedDoSomeSimilarThings) Write(seqId int32, result thrift.WritableStruct, encoder thrift.Encoder) (err error) {
    var err2 error
    messageType := thrift.REPLY
    switch result.(type) {
    case thrift.ApplicationExceptionIf:
        messageType = thrift.EXCEPTION
    }

    if err2 = encoder.WriteMessageBegin("do_some_similar_things", messageType, seqId); err2 != nil {
        err = err2
    }
    if err2 = result.Write(encoder); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.WriteMessageEnd(); err == nil && err2 != nil {
        err = err2
    }
    if err2 = encoder.Flush(); err == nil && err2 != nil {
        err = err2
    }
    return err
}

func (p *procFuncInteractWithSharedDoSomeSimilarThings) RunContext(ctx context.Context, reqStruct thrift.Struct) (thrift.WritableStruct, thrift.ApplicationExceptionIf) {
    result := newRespInteractWithSharedDoSomeSimilarThings()
    retval, err := p.handler.DoSomeSimilarThings(ctx)
    if err != nil {
        x := thrift.NewApplicationExceptionCause(thrift.INTERNAL_ERROR, "Internal error processing DoSomeSimilarThings: " + err.Error(), err)
        return x, x
    }

    result.Success = retval
    return result, nil
}


type BoxService interface {
}

type BoxServiceClientInterface interface {
    io.Closer
}

type BoxServiceClient struct {
    ch thrift.RequestChannel
}
// Compile time interface enforcer
var _ BoxServiceClientInterface = (*BoxServiceClient)(nil)

func NewBoxServiceChannelClient(channel thrift.RequestChannel) *BoxServiceClient {
    return &BoxServiceClient{
        ch: channel,
    }
}

func (c *BoxServiceClient) Close() error {
    return c.ch.Close()
}


type BoxServiceProcessor struct {
    processorFunctionMap map[string]thrift.ProcessorFunction
    functionServiceMap   map[string]string
    handler              BoxService
}

func NewBoxServiceProcessor(handler BoxService) *BoxServiceProcessor {
    p := &BoxServiceProcessor{
        handler:              handler,
        processorFunctionMap: make(map[string]thrift.ProcessorFunction),
        functionServiceMap:   make(map[string]string),
    }

    return p
}

func (p *BoxServiceProcessor) AddToProcessorFunctionMap(key string, processorFunction thrift.ProcessorFunction) {
    p.processorFunctionMap[key] = processorFunction
}

func (p *BoxServiceProcessor) AddToFunctionServiceMap(key, service string) {
    p.functionServiceMap[key] = service
}

func (p *BoxServiceProcessor) GetProcessorFunction(key string) (processor thrift.ProcessorFunction) {
    return p.processorFunctionMap[key]
}

func (p *BoxServiceProcessor) ProcessorFunctionMap() map[string]thrift.ProcessorFunction {
    return p.processorFunctionMap
}

func (p *BoxServiceProcessor) FunctionServiceMap() map[string]string {
    return p.functionServiceMap
}

func (p *BoxServiceProcessor) PackageName() string {
    return "module"
}

func (p *BoxServiceProcessor) GetThriftMetadata() *metadata.ThriftMetadata {
    return GetThriftMetadataForService("module.BoxService")
}


