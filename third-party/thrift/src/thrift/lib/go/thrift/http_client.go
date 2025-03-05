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
	"bytes"
	"io"
	"net/http"
	"net/url"
	"strconv"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type httpClient struct {
	client             *http.Client
	response           *http.Response
	url                *url.URL
	requestBuffer      *bytes.Buffer
	responseBuffer     bytes.Buffer
	header             http.Header
	nsecConnectTimeout int64
	nsecReadTimeout    int64
}

// newHTTPPostClient creates a new HTTP POST client.
func newHTTPPostClient(urlstr string) (*httpClient, error) {
	parsedURL, err := url.Parse(urlstr)
	if err != nil {
		return nil, err
	}
	buf := make([]byte, 0, 1024)
	client := http.DefaultClient
	return &httpClient{client: client, url: parsedURL, requestBuffer: bytes.NewBuffer(buf), header: http.Header{}}, nil
}

// SetHeader sets the HTTP Header for this specific Thrift Transport
// It is important that you first assert the Transport as a HTTPClient type
// like so:
//
// httpTrans := trans.(*httpClient)
// httpTrans.SetHeader("User-Agent","Thrift Client 1.0")
func (p *httpClient) SetHeader(key string, value string) {
	p.header.Add(key, value)
}

// GetHeader gets the HTTP Header represented by the supplied Header Key for this specific Thrift Transport
// It is important that you first assert the Transport as a HTTPClient type
// like so:
//
// httpTrans := trans.(*httpClient)
// hdrValue := httpTrans.GetHeader("User-Agent")
func (p *httpClient) GetHeader(key string) string {
	return p.header.Get(key)
}

// DelHeader deletes the HTTP Header given a Header Key for this specific Thrift Transport
// It is important that you first assert the Transport as a HTTPClient type
// like so:
//
// httpTrans := trans.(*httpClient)
// httpTrans.DelHeader("User-Agent")
func (p *httpClient) DelHeader(key string) {
	p.header.Del(key)
}

func (p *httpClient) closeResponse() error {
	p.response = nil
	p.responseBuffer.Reset()
	return nil
}

func (p *httpClient) Close() error {
	if p.requestBuffer != nil {
		p.requestBuffer.Reset()
		p.requestBuffer = nil
	}
	return p.closeResponse()
}

func (p *httpClient) Read(buf []byte) (int, error) {
	if p.response == nil {
		return 0, types.NewTransportException(types.NOT_OPEN, "Response buffer is empty, no request.")
	}
	n, err := p.responseBuffer.Read(buf)
	if n > 0 && (err == nil || err == io.EOF) {
		return n, nil
	}
	return n, types.NewTransportExceptionFromError(err)
}

func (p *httpClient) ReadByte() (byte, error) {
	return readByte(&p.responseBuffer)
}

func (p *httpClient) Write(buf []byte) (int, error) {
	n, err := p.requestBuffer.Write(buf)
	return n, err
}

func (p *httpClient) WriteByte(c byte) error {
	return p.requestBuffer.WriteByte(c)
}

func (p *httpClient) WriteString(s string) (int, error) {
	return p.requestBuffer.WriteString(s)
}

func (p *httpClient) Flush() error {
	// Close any previous response body to avoid leaking connections.
	p.closeResponse()

	req, err := http.NewRequest("POST", p.url.String(), p.requestBuffer)
	if err != nil {
		return types.NewTransportExceptionFromError(err)
	}
	p.header.Set("Content-Type", "application/x-thrift")
	req.Header = p.header
	response, err := p.client.Do(req)
	if err != nil {
		return types.NewTransportExceptionFromError(err)
	}

	defer response.Body.Close()

	if response.StatusCode != http.StatusOK {
		// Close the response to avoid leaking file descriptors. closeResponse does
		// more than just call Close(), so temporarily assign it and reuse the logic.
		p.response = response
		p.closeResponse()

		// TODO(pomack) log bad response
		return types.NewTransportException(types.UNKNOWN_TRANSPORT_EXCEPTION, "HTTP Response code: "+strconv.Itoa(response.StatusCode))
	}

	_, err = io.Copy(&p.responseBuffer, response.Body)
	if err != nil {
		return types.NewTransportExceptionFromError(err)
	}

	p.response = response
	return nil
}

func (p *httpClient) RemainingBytes() uint64 {
	return uint64(p.responseBuffer.Len())
}
