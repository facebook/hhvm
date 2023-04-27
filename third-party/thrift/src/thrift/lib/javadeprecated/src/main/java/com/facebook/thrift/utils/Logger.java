/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.facebook.thrift.utils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public final class Logger {
  private interface LoggerFunction {
    public void apply(String msg);
  }

  private static Method getLogger;
  private final LoggerFunction errorFn;
  private final LoggerFunction warningFn;
  private final String name;

  static {
    try {
      getLogger =
          Class.forName("org.slf4j.LoggerFactory").getDeclaredMethod("getLogger", String.class);
    } catch (ClassNotFoundException | NoSuchMethodException e) {
      // do nothing
    }
  }

  private Logger(String name, LoggerFunction errorFn, LoggerFunction warningFn) {
    this.name = name;
    this.errorFn = errorFn;
    this.warningFn = warningFn;
  }

  public static Logger getLogger(final String name) {
    LoggerFunction errorFn;
    LoggerFunction warningFn;
    try {
      final Object logger = getLogger.invoke(null, name);
      final Method err = logger.getClass().getDeclaredMethod("error", String.class);
      final Method warning = logger.getClass().getDeclaredMethod("warn", String.class);
      errorFn =
          new LoggerFunction() {
            @Override
            public void apply(final String msg) {
              try {
                err.invoke(logger, msg);
              } catch (IllegalAccessException
                  | IllegalArgumentException
                  | InvocationTargetException
                  | ExceptionInInitializerError e) {
                // Fall back to System.err.println
                System.err.println(name + " ERROR: " + msg);
              }
            }
          };
      warningFn =
          new LoggerFunction() {
            @Override
            public void apply(final String msg) {
              try {
                warning.invoke(logger, msg);
              } catch (IllegalAccessException
                  | IllegalArgumentException
                  | InvocationTargetException
                  | ExceptionInInitializerError e) {
                // Fall back to System.err.println
                System.err.println(name + " WRNING: " + msg);
              }
            }
          };
    } catch (IllegalAccessException
        | IllegalArgumentException
        | InvocationTargetException
        | NoSuchMethodException
        | NullPointerException
        | ExceptionInInitializerError e) {
      errorFn =
          new LoggerFunction() {
            @Override
            public void apply(String msg) {
              System.err.println(name + " ERROR: " + msg);
            }
          };
      warningFn =
          new LoggerFunction() {
            @Override
            public void apply(String msg) {
              System.err.println(name + " WARNING: " + msg);
            }
          };
    }
    return new Logger(name, errorFn, warningFn);
  }

  public void error(String msg) {
    errorFn.apply(msg);
  }

  public void error(String msg, Object other) {
    errorFn.apply(msg + other.toString());
  }

  public void warn(String msg) {
    warningFn.apply(msg);
  }

  public void warn(String msg, Object other) {
    errorFn.apply(msg + other.toString());
  }
}
