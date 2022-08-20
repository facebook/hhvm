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

package com.facebook.thrift.util;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.util.concurrent.ThreadFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Exceptions;

/**
 * Wraps JDK9 cleaner with method handles so you can use it when code is compiled with JDK8, but run
 * with version greater than JDK8
 */
public class CleanerWrapper {
  private static final Logger LOGGER = LoggerFactory.getLogger(CleanerWrapper.class);
  private static final CleanerWrapper DUMMY_CLEANER = new CleanerWrapper();
  private static final MethodHandle CREATE_WITH_TF;
  private static final MethodHandle CREATE;
  private static final MethodHandle REGISTER;

  static {
    MethodHandle createWithTf = null;
    MethodHandle create = null;
    MethodHandle register = null;
    try {
      MethodHandles.Lookup lookup = MethodHandles.lookup();

      Class<?> cleanerClass = Class.forName("java.lang.ref.Cleaner");
      Class<?> cleanableClass = Class.forName("java.lang.ref.Cleaner.Cleanable");

      create = lookup.findStatic(cleanerClass, "create", MethodType.methodType(cleanerClass));
      createWithTf =
          lookup.findStatic(
              cleanerClass, "create", MethodType.methodType(cleanerClass, ThreadFactory.class));
      register =
          lookup.findVirtual(
              cleanerClass,
              "register",
              MethodType.methodType(cleanableClass, Object.class, Runnable.class));
    } catch (Throwable t) {
      LOGGER.warn("cleaner will not work with JDKs older than JDK9", t);
    }

    CREATE_WITH_TF = createWithTf;
    CREATE = create;
    REGISTER = register;
  }

  private Object cleaner;

  private CleanerWrapper() {}

  public static CleanerWrapper create(ThreadFactory threadFactory) {
    if (CREATE_WITH_TF == null) {
      return DUMMY_CLEANER;
    }

    try {
      CleanerWrapper wrapper = new CleanerWrapper();
      wrapper.cleaner = CREATE_WITH_TF.invokeExact(threadFactory);
      return wrapper;
    } catch (Throwable t) {
      throw Exceptions.propagate(t);
    }
  }

  public static CleanerWrapper create() {
    if (CREATE == null) {
      return DUMMY_CLEANER;
    }

    try {
      CleanerWrapper wrapper = new CleanerWrapper();
      wrapper.cleaner = CREATE.invokeExact();
      return wrapper;
    } catch (Throwable t) {
      throw Exceptions.propagate(t);
    }
  }

  public void register(Object o, Runnable action) {
    if (REGISTER == null) {
      return;
    }

    try {
      REGISTER.invokeWithArguments(cleaner, o, action);
    } catch (Throwable t) {
      throw Exceptions.propagate(t);
    }
  }
}
