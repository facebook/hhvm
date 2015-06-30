/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_EXT_ASIO_EXTERNAL_THREAD_EVENT_H_
#define incl_EXT_ASIO_EXTERNAL_THREAD_EVENT_H_

#include <atomic>
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AsioSession;
class c_ExternalThreadEventWaitHandle;

/**
 * An asynchronous external thread event.
 *
 * A root class of all classes of objects representing events external to
 * the web request thread that synchronizes on them using ASIO framework.
 *
 * This is the preferred mechanism of I/O integration with ASIO framework.
 *
 * To implement a new class of such events, define a subclass that stores
 * information needed to unserialize result and provides a memory for storing
 * raw result. Then define a method that populates the result and marks
 * the event as finished, override unserialize method that will produce
 * result in the format understood by HHVM and implement destructor that
 * will clean up the stored data.
 *
 * Example:
 *
 * class FooEvent : public AsioExternalThreadEvent {
 *   public:
 *     FooEvent(int max_value) : m_maxValue(max_value), m_failed(false) {}
 *     ~FooEvent() {}
 *     void setResult(int value) {
 *       m_value = value;
 *       markAsFinished();
 *     }
 *     void setException(const FooException& exception) {
 *       m_failed = true;
 *       markAsFinished();
 *     }
 *   protected:
 *     void unserialize(Cell& result) const {
 *       if (UNLIKELY(m_failed)) {
 *         Object e(SystemLib::AllocInvalidOperationExceptionObject(
 *           "An error has occurred while scheduling the operation"));
 *         throw e;
 *       }
 *
 *       if (UNLIKELY(m_value > m_maxValue)) {
 *         Object e(SystemLib::AllocInvalidOperationExceptionObject(
 *           "Invalid response returned by Foo backend"));
 *         throw e;
 *       }
 *
 *       cellDup(make_tv<KindOfInt64>(m_value), result);
 *     }
 *   private:
 *     int m_value, m_maxValue;
 *     bool m_failed;
 * };
 *
 * To use this mechanism from your extension, create an instance of the class,
 * schedule an asynchronous operation and return the wait handle to the caller.
 *
 * Example:
 *
 * Object f_gen_foo(int max_value) {
 *   // validate user input early
 *   if (max_value < 0) {
 *     Object e(SystemLib::AllocInvalidArgumentExceptionObject(
 *       "Expected max_value to be non-negative"));
 *     throw e;
 *   }
 *
 *   FooEvent* event = new FooEvent(max_value);
 *   try {
 *     // make foo run asynchronously and eventually call event->setResult()
 *     foo_async_schedule(event);
 *   } catch (const FooException& exception) {
 *     // handle error while trying to schedule requested operation
 *     event->setException(exception);
 *   } catch (...) {
 *     // unknown exception; should be never reached
 *     assert(false);
 *     event->abandon();
 *     Object e(SystemLib::AllocInvalidOperationExceptionObject(
 *       "Encountered unexpected exception"));
 *     throw e;
 *   }
 *   return event->getWaitHandle();
 * }
 *
 * Caveats:
 *  - web request may die before the event is finished; never store pointers
 *    to any data owned by PHP as the PHP thread may die at any time
 */
class AsioExternalThreadEvent {
  public:
    /**
     * Get wait handle representing this external thread event.
     *
     * This function may be called only from the web request thread between
     * construction of this object and return of the control back to the VM.
     * Do not try to call this after any PHP code got executed; the asynchronous
     * operation may have finished and this object could have been already
     * destroyed.
     *
     * The caller is responsible for obtaining a reference count immediately
     * after obtaining the pointer (e.g. by type casting this into Object,
     * populating a TypedValue using tvWriteObject, or setting an array
     * element). If any PHP code is executed, a bad things may happen.
     *
     * It is okay to call this after asynchronous operation was scheduled.
     * Even if the operation has finished, the object is not destroyed until
     * ASIO main loop is executed.
     */
    c_ExternalThreadEventWaitHandle* getWaitHandle() {
      return m_waitHandle;
    }

    /**
     * Abandon this external thread event.
     *
     * Abandons this event and frees all associated resources before any action
     * is taken. Useful if a caller changed its mind after this event was
     * constructed (e.g. due to failed transfer of ownership to the processing
     * thread).
     *
     * This function may be called only from the web request thread between
     * construction of this object and return of the control back to the VM,
     * assuming that no other methods of this object were called prior to this
     * call (except for constructor). Once this function is called, all
     * associated resources are reclaimed and it is illegal to perform any
     * further operation on this object.
     */
    void abandon();

    /**
     * Cancel waiting for the event. INTERNAL USE ONLY. DO NOT CALL.
     *
     * Returns true iff we transitioned from Waiting to Canceled; otherwise,
     * this function waits until processing thread completed its transition
     * to the Finished state.
     *
     * This function is called internally from the web request thread to signal
     * that the result will not be retrieved by the web request thread.
     *
     * This is used in rare case when the web request thread is dying without
     * waiting for the asynchronous operation to finish.
     */
    bool cancel();

    /**
     * Destroy the object. INTERNAL USE ONLY. DO NOT CALL.
     *
     * The purpose of this function is to make destructor protected so that
     * this object is not accidentally used in conjunction with shared pointers.
     */
    void release() { delete this; }

    /**
     * Unserialize result. Implemented by subclasses.
     *
     * Unserializes result and writes it to the provided uninitialized
     * TypedValue variable.
     *
     * This function is optionally called internally from the web request thread
     * to retrieve result of the operation after markAsFinished() was called
     * to signal the result is ready.
     *
     * A failed operation may be signaled by throwing a PHP exception instead
     * of populating result variable. Result variable will be ignored.
     * If a result was already initialized, it must be uninitialized (decref
     * if needed) prior to throwing an exception.
     */
    virtual void unserialize(Cell& result) = 0;

  protected:
    /**
     * Construct AsioExternalThreadEvent
     *
     * Subclass may optionally pass a PHP object (usually defined by extension)
     * that can be accessed during unserialize() via getPrivData(). A reference
     * to the object will be held while associated ExternalThreadEventWaitHandle
     * object is in WAITING state.
     */
    explicit AsioExternalThreadEvent(ObjectData* priv_data = nullptr);

    /**
     * Destruct AsioExternalThreadEvent
     *
     * Object lifetime and ownership is managed internally. Do not try to
     * destruct this object yourself. Instead, make sure markAsFinished()
     * is eventually called.
     */
    virtual ~AsioExternalThreadEvent() {
      assert(
        m_state.load() == Finished ||
        m_state.load() == Canceled ||
        m_state.load() == Abandoned
      );
    };

    /**
     * Mark the event as finished.
     *
     * Marks event as finished and transfers ownership of this object to
     * the web request thread.
     *
     * This function may be called only from the processing thread to signal
     * that the result is ready to be unserialized. Once this function is
     * called, unserialize() function or object destructor may be called
     * at any time and the processing thread should not do any further
     * operations on this object.
     */
    void markAsFinished();

    /**
     * Get private PHP data needed for unserialization.
     *
     * This function obtains private data stored by constructor. It may be
     * called only from the unserialize() implementation.
     */
    ObjectData* getPrivData() const {
      assert(m_state.load() == Finished);
      return m_waitHandle->getPrivData();
    }

  private:
    enum state_t : uint32_t {
      /**
       * Web request thread waiting for processing thread to finish.
       *
       * This object is owned by processing thread, which is responsible for
       * eventually calling markAsFinished().
       */
      Waiting,

      /**
       * Web request thread scheduled to unserialize result.
       *
       * This object is owned by web request thread, which is responsible for
       * destruction of this object after optional unserialization.
       */
      Finished,

      /**
       * Web request thread died before processing thread finished.
       *
       * This object is owned by processing thread, which is responsible for
       * eventually calling markAsFinished() that will destruct this object.
       */
      Canceled,

      /**
       * Web request thread abandoned event before passing to processing thread.
       *
       * This object is owned by web request thread, which is trying to abandon
       * it prior to passing ownership to processing thread.
       */
      Abandoned,
    };

    AsioExternalThreadEventQueue* m_queue;
    c_ExternalThreadEventWaitHandle* m_waitHandle;
    std::atomic<uint32_t/*state_t*/> m_state;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_EXT_ASIO_EXTERNAL_THREAD_EVENT_H_
