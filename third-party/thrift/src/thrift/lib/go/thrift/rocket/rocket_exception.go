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

package rocket

import (
	"encoding/json"

	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
)

// RocketExceptionType is the type of a Thrift Rocket exception.
type RocketExceptionType int16

const (
	rocketExceptionUnknown         RocketExceptionType = 0
	rocketExceptionDeclared        RocketExceptionType = 1
	rocketExceptionAppUnknown      RocketExceptionType = 2
	rocketExceptionAny             RocketExceptionType = 3
	rocketExceptionDeprecatedProxy RocketExceptionType = 4
)

func (e RocketExceptionType) String() string {
	switch e {
	case rocketExceptionUnknown:
		return "UnknownException"
	case rocketExceptionDeclared:
		return "DeclaredException"
	case rocketExceptionAppUnknown:
		return "AppUnknownException"
	case rocketExceptionAny:
		return "AnyException"
	case rocketExceptionDeprecatedProxy:
		return "DEPRECATEDProxyException"
	}
	panic("unreachable")
}

func (e RocketExceptionType) MarshalJSON() ([]byte, error) {
	return json.Marshal(e.String())
}

type rocketException struct {
	Name          string
	What          string
	ExceptionType RocketExceptionType
	Kind          rpcmetadata.ErrorKind
	Blame         rpcmetadata.ErrorBlame
	Safety        rpcmetadata.ErrorSafety
}

var _ error = (*rocketException)(nil)

func newRocketException(exception *rpcmetadata.PayloadExceptionMetadataBase) *rocketException {
	result := &rocketException{
		Name:          "unknown",
		What:          "unknown",
		ExceptionType: rocketExceptionUnknown,
		Kind:          rpcmetadata.ErrorKind_UNSPECIFIED,
		Blame:         rpcmetadata.ErrorBlame_UNSPECIFIED,
		Safety:        rpcmetadata.ErrorSafety_UNSPECIFIED,
	}
	if exception.NameUTF8 != nil {
		result.Name = *exception.NameUTF8
	}
	if exception.WhatUTF8 != nil {
		result.What = *exception.WhatUTF8
	}

	metadata := exception.Metadata
	if metadata == nil {
		// No metadata - we are done here.
		return result
	}

	var classification *rpcmetadata.ErrorClassification
	if metadata.DeclaredException != nil {
		result.ExceptionType = rocketExceptionDeclared
		classification = metadata.DeclaredException.ErrorClassification
	} else if metadata.AppUnknownException != nil {
		result.ExceptionType = rocketExceptionAppUnknown
		classification = metadata.AppUnknownException.ErrorClassification
	} else if metadata.AnyException != nil {
		result.ExceptionType = rocketExceptionAny
	} else if metadata.DEPRECATEDProxyException != nil {
		result.ExceptionType = rocketExceptionDeprecatedProxy
	}
	if classification != nil {
		if classification.Kind != nil {
			result.Kind = classification.GetKind()
		}
		if classification.Blame != nil {
			result.Blame = classification.GetBlame()
		}
		if classification.Safety != nil {
			result.Safety = classification.GetSafety()
		}
	}
	return result
}

func newUnknownPayloadExceptionMetadataBase(name string, what string) *rpcmetadata.PayloadExceptionMetadataBase {
	return newPayloadExceptionMetadataBase(&rocketException{
		Name:          name,
		What:          what,
		ExceptionType: rocketExceptionUnknown,
		Safety:        rpcmetadata.ErrorSafety_SAFE,
		Kind:          rpcmetadata.ErrorKind_TRANSIENT,
		Blame:         rpcmetadata.ErrorBlame_SERVER,
	})
}

func newPayloadExceptionMetadataBase(err *rocketException) *rpcmetadata.PayloadExceptionMetadataBase {
	classification := rpcmetadata.NewErrorClassification().
		SetKind(&err.Kind).
		SetBlame(&err.Blame).
		SetSafety(&err.Safety)

	metadata := rpcmetadata.NewPayloadExceptionMetadata()
	switch err.ExceptionType {
	case rocketExceptionDeclared:
		declared := rpcmetadata.NewPayloadDeclaredExceptionMetadata().
			SetErrorClassification(classification)
		metadata.SetDeclaredException(declared)
	case rocketExceptionAppUnknown:
		appUnknown := rpcmetadata.NewPayloadAppUnknownExceptionMetdata().
			SetErrorClassification(classification)
		metadata.SetAppUnknownException(appUnknown)
	case rocketExceptionAny:
		metadata.SetAnyException(rpcmetadata.NewPayloadAnyExceptionMetadata())
	case rocketExceptionDeprecatedProxy:
		metadata.SetDEPRECATEDProxyException(rpcmetadata.NewPayloadProxyExceptionMetadata())
	case rocketExceptionUnknown:
	default:
		panic("unreachable")
	}

	base := rpcmetadata.NewPayloadExceptionMetadataBase().
		SetNameUTF8(&err.Name).
		SetWhatUTF8(&err.What).
		SetMetadata(metadata)
	return base
}

func (e *rocketException) Error() string {
	data, err := json.Marshal(e)
	if err != nil {
		panic(err)
	}
	return string(data)
}

func (e *rocketException) IsDeclared() bool {
	return e.ExceptionType == rocketExceptionDeclared
}
