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
	"time"

	"github.com/stretchr/testify/mock"
)

// MockServerObserver is a mock implementation of ServerObserver interface for testing.
type MockServerObserver struct {
	mock.Mock
}

func (m *MockServerObserver) ConnDropped() {
	m.Called()
}

func (m *MockServerObserver) ConnAccepted() {
	m.Called()
}

func (m *MockServerObserver) ConnTLSAccepted() {
	m.Called()
}

func (m *MockServerObserver) ReceivedHeaderRequest() {
	m.Called()
}

func (m *MockServerObserver) TaskKilled() {
	m.Called()
}

func (m *MockServerObserver) TaskTimeout() {
	m.Called()
}

func (m *MockServerObserver) DeclaredException() {
	m.Called()
}

func (m *MockServerObserver) UndeclaredException() {
	m.Called()
}

func (m *MockServerObserver) ServerOverloaded() {
	m.Called()
}

func (m *MockServerObserver) ReceivedRequest() {
	m.Called()
}

func (m *MockServerObserver) SentReply() {
	m.Called()
}

func (m *MockServerObserver) ActiveRequests(numRequests int) {
	m.Called(numRequests)
}

func (m *MockServerObserver) ProcessorPanic() {
	m.Called()
}

func (m *MockServerObserver) ProcessDelay(delay time.Duration) {
	m.Called(delay)
}

func (m *MockServerObserver) ProcessTime(duration time.Duration) {
	m.Called(duration)
}

func (m *MockServerObserver) ReceivedRequestForFunction(function string) {
	m.Called(function)
}

func (m *MockServerObserver) AnyExceptionForFunction(function string) {
	m.Called(function)
}

func (m *MockServerObserver) TimeReadUsForFunction(function string, duration time.Duration) {
	m.Called(function, duration)
}

func (m *MockServerObserver) TimeProcessUsForFunction(function string, duration time.Duration) {
	m.Called(function, duration)
}

func (m *MockServerObserver) TimeWriteUsForFunction(function string, duration time.Duration) {
	m.Called(function, duration)
}
