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
	"encoding/json"
)

type rocketExceptionType int16

const (
	rocketExceptionUnknown         rocketExceptionType = 0
	rocketExceptionDeclared        rocketExceptionType = 1
	rocketExceptionAppUnknown      rocketExceptionType = 2
	rocketExceptionAny             rocketExceptionType = 3
	rocketExceptionDeprecatedProxy rocketExceptionType = 4 // This is necessary for SR Proxy timeouts to work
)

func (e rocketExceptionType) String() string {
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

func (e rocketExceptionType) MarshalJSON() ([]byte, error) {
	return json.Marshal(e.String())
}

type rocketException struct {
	Name          string
	What          string
	ExceptionType rocketExceptionType
	Kind          ErrorKind
	Blame         ErrorBlame
	Safety        ErrorSafety
}

var _ error = (*rocketException)(nil)

func newRocketException(exception *PayloadExceptionMetadataBase) *rocketException {
	err := &rocketException{
		Name:          "unknown",
		What:          "unknown",
		ExceptionType: rocketExceptionUnknown,
		Kind:          ErrorKind_UNSPECIFIED,
		Blame:         ErrorBlame_UNSPECIFIED,
		Safety:        ErrorSafety_UNSPECIFIED,
	}
	if exception.NameUTF8 != nil {
		err.Name = *exception.NameUTF8
	}
	if exception.WhatUTF8 != nil {
		err.What = *exception.WhatUTF8
	}
	var class *ErrorClassification
	if exception.Metadata != nil {
		if exception.Metadata.DeclaredException != nil {
			err.ExceptionType = rocketExceptionDeclared
			if exception.Metadata.DeclaredException.ErrorClassification != nil {
				class = exception.Metadata.DeclaredException.ErrorClassification
			}
		} else if exception.Metadata.AppUnknownException != nil {
			err.ExceptionType = rocketExceptionAppUnknown
			if exception.Metadata.AppUnknownException.ErrorClassification != nil {
				class = exception.Metadata.AppUnknownException.ErrorClassification
			}
		} else if exception.Metadata.AnyException != nil {
			err.ExceptionType = rocketExceptionAny
		} else if exception.Metadata.DEPRECATEDProxyException != nil {
			err.ExceptionType = rocketExceptionDeprecatedProxy
		}
		if class != nil {
			if class.Kind != nil {
				err.Kind = class.GetKind()
			}
			if class.Blame != nil {
				err.Blame = class.GetBlame()
			}
			if class.Safety != nil {
				err.Safety = class.GetSafety()
			}
		}
	}
	return err
}

func newUnknownPayloadExceptionMetadataBase(name string, what string) *PayloadExceptionMetadataBase {
	return newPayloadExceptionMetadataBase(&rocketException{
		Name:          name,
		What:          what,
		ExceptionType: rocketExceptionUnknown,
		Safety:        ErrorSafety_SAFE,
		Kind:          ErrorKind_TRANSIENT,
		Blame:         ErrorBlame_SERVER,
	})
}

func newPayloadExceptionMetadataBase(err *rocketException) *PayloadExceptionMetadataBase {
	base := NewPayloadExceptionMetadataBase()
	base.SetNameUTF8(&err.Name)
	base.SetWhatUTF8(&err.What)
	class := NewErrorClassification()
	class.SetKind(&err.Kind)
	class.SetBlame(&err.Blame)
	class.SetSafety(&err.Safety)
	metadata := NewPayloadExceptionMetadata()
	switch err.ExceptionType {
	case rocketExceptionDeclared:
		declared := NewPayloadDeclaredExceptionMetadata()
		declared.SetErrorClassification(class)
		metadata.SetDeclaredException(declared)
	case rocketExceptionAppUnknown:
		appUnknown := NewPayloadAppUnknownExceptionMetdata()
		appUnknown.SetErrorClassification(class)
		metadata.SetAppUnknownException(appUnknown)
	case rocketExceptionAny:
		metadata.SetAnyException(NewPayloadAnyExceptionMetadata())
	case rocketExceptionDeprecatedProxy:
		metadata.SetDEPRECATEDProxyException(NewPayloadProxyExceptionMetadata())
	case rocketExceptionUnknown:
	default:
		panic("unreachable")
	}
	base.SetMetadata(metadata)
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
