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

// ResponseHeaderGetter is a temporary measure to allow protocols to expose headers received with the response.
type ResponseHeaderGetter interface {
	GetResponseHeaders() map[string]string
}

// Compile time interface enforcer
var _ ResponseHeaderGetter = (*headerProtocol)(nil)
var _ ResponseHeaderGetter = (*rocketClient)(nil)
var _ ResponseHeaderGetter = (*upgradeToRocketClient)(nil)
var _ ResponseHeaderGetter = (*httpProtocol)(nil)
