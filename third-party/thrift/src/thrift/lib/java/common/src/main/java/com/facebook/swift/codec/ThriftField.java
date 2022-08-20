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

package com.facebook.swift.codec;

import static java.lang.annotation.ElementType.FIELD;
import static java.lang.annotation.ElementType.METHOD;
import static java.lang.annotation.ElementType.PARAMETER;
import static java.lang.annotation.RetentionPolicy.RUNTIME;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

/** Marks a field, method or parameter as a Thrift field. */
@Documented
@Retention(RUNTIME)
@Target({METHOD, FIELD, PARAMETER})
public @interface ThriftField {
  short value() default Short.MIN_VALUE;

  /** Indicates this ThriftField has a negative ID, which is deprecated. */
  boolean isLegacyId() default false;

  String name() default "";

  Requiredness requiredness() default Requiredness.UNSPECIFIED;

  Recursiveness isRecursive() default Recursiveness.UNSPECIFIED;

  @Deprecated
  ThriftIdlAnnotation[] idlAnnotations() default {};

  enum Recursiveness {
    UNSPECIFIED,
    FALSE,
    TRUE,
  }

  /**
   * Indicates the behavior for a field when a value is not received, or when the value of the field
   * is not set when sending.
   */
  enum Requiredness {
    /**
     * This is the default (unset) value for {@link ThriftField#requiredness()}. It will not
     * conflict with other explicit settings of {@link #NONE}, {@link #REQUIRED}, or {@link
     * #OPTIONAL}. If all of the {@link ThriftField} annotations for a field are left {@link
     * #UNSPECIFIED}, it will default to {@link #NONE}.
     */
    UNSPECIFIED,
    /**
     * This behavior is equivalent to leaving out 'optional' and 'required' in thrift IDL syntax.
     * However, despite the name, this actually does correspond to defined behavior so if this value
     * is explicitly specified in any annotations, it will conflict with other annotations that
     * specify either {@link #OPTIONAL} or {@link #REQUIRED} for the same field.
     *
     * <p>The serialization behavior is that {@code null} values will not be serialized, but if the
     * field is non-nullable (i.e. it's type is primitive) it will be serialized, even if the field
     * was not explicitly set.
     *
     * <p>The deserialization behavior is similar: When no value is read, the field will be set to
     * {@code null} if the type of the field is nullable, but for primitive types the field will be
     * left untouched (so it will hold the default value for the type).
     */
    NONE,
    /**
     * This behavior indicates that the field will always be serialized (and it is an error if the
     * value is {@code null}), and must always be deserialized (and it is an error if a value is not
     * read).
     */
    REQUIRED,
    /**
     * This behavior indicates that it is always ok if the field is {@code null} when serializing,
     * and that it is always ok to not read a value (and the field will be set to {@code null} when
     * this happens). As such, primitive types should be replaced with boxed types, so that null is
     * always a possibility.
     */
    OPTIONAL
  }
}
