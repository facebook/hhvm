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

package types

import (
	"context"
	"iter"
	"testing"

	"github.com/stretchr/testify/require"
)

// Mock client interface for testing
type mockClient interface {
	DoSomething() string
	Close() error
}

// Mock client implementation
type mockClientImpl struct {
	channel RequestChannel
}

func (c *mockClientImpl) DoSomething() string {
	return "done"
}

func (c *mockClientImpl) Close() error {
	return c.channel.Close()
}

// Mock constructor function
func newMockChannelClient(channel RequestChannel) mockClient {
	return &mockClientImpl{channel: channel}
}

// Another mock client for testing different types
type anotherMockClient interface {
	DoOtherThing() int
}

type anotherMockClientImpl struct {
	channel RequestChannel
}

func (c *anotherMockClientImpl) DoOtherThing() int {
	return 42
}

func newAnotherMockChannelClient(channel RequestChannel) anotherMockClient {
	return &anotherMockClientImpl{channel: channel}
}

// Mock RequestChannel for testing
type mockRequestChannel struct{}

func (c *mockRequestChannel) Close() error {
	return nil
}

func (c *mockRequestChannel) SendRequestResponse(ctx context.Context, method string, request WritableStruct, response ReadableStruct) error {
	return nil
}

func (c *mockRequestChannel) SendRequestNoResponse(ctx context.Context, method string, request WritableStruct) error {
	return nil
}

func (c *mockRequestChannel) SendRequestStream(ctx context.Context, method string, request WritableStruct, response ReadableStruct, newStreamElemFn func() ReadableResult, onStreamNextFn func(ReadableStruct), onStreamErrorFn func(error), onStreamCompleteFn func()) (iter.Seq2[ReadableStruct, error], error) {
	return nil, nil
}

func TestRegisterAndConstructClient(t *testing.T) {
	InternalRegisterClientConstructor[mockClient](newMockChannelClient)

	channel := &mockRequestChannel{}
	client, err := InternalConstructClientFromRegistry[mockClient](channel)

	require.NoError(t, err)
	require.NotNil(t, client)
	require.Equal(t, "done", client.DoSomething())
}

func TestConstructClientNotRegistered(t *testing.T) {
	// unregisteredClient is not registered
	type unregisteredClient interface {
		Unregistered() bool
	}

	channel := &mockRequestChannel{}
	client, err := InternalConstructClientFromRegistry[unregisteredClient](channel)

	require.Error(t, err)
	require.Nil(t, client)
	require.Contains(t, err.Error(), "no registered client constructor")
}

func TestRegisterMultipleClients(t *testing.T) {
	InternalRegisterClientConstructor[mockClient](newMockChannelClient)
	InternalRegisterClientConstructor[anotherMockClient](newAnotherMockChannelClient)

	channel := &mockRequestChannel{}

	client1, err1 := InternalConstructClientFromRegistry[mockClient](channel)
	require.NoError(t, err1)
	require.NotNil(t, client1)
	require.Equal(t, "done", client1.DoSomething())

	client2, err2 := InternalConstructClientFromRegistry[anotherMockClient](channel)
	require.NoError(t, err2)
	require.NotNil(t, client2)
	require.Equal(t, 42, client2.DoOtherThing())
}

func TestRegisterOverwritesExisting(t *testing.T) {
	// Register with a constructor that returns a specific value
	InternalRegisterClientConstructor[mockClient](func(channel RequestChannel) mockClient {
		return &mockClientImpl{channel: channel}
	})

	// Overwrite with a different constructor
	customImpl := &mockClientImpl{}
	InternalRegisterClientConstructor[mockClient](func(channel RequestChannel) mockClient {
		return customImpl
	})

	channel := &mockRequestChannel{}
	client, err := InternalConstructClientFromRegistry[mockClient](channel)

	require.NoError(t, err)
	require.Same(t, customImpl, client)
}
