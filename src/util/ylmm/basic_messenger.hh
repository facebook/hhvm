//
// $Id: basic_messenger.hh,v 1.4 2003/11/18 11:27:31 cholm Exp $ 
//  
//  ylmm::basic_messenger
//  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk> 
//
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public License 
//  as published by the Free Software Foundation; either version 2.1 
//  of the License, or (at your option) any later version. 
//
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free 
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
//  02111-1307 USA 
//
#ifndef YLMM_basic_messenger
#define YLMM_basic_messenger
#ifndef __IOSTREAM__
#include <iostream>
#endif
#ifndef __CSTDARG__
#include <cstdarg>
#endif
#ifndef __CSTDIO__
#include <cstdio>
#endif

/** @file   ylmm/basic_messenger.hh
    @author Christian Holm
    @date   Sat Jan 18 03:37:32 2003
    @brief  Base class for I/O handling */

/** @defgroup common Common classes */

namespace ylmm
{
  //================================================================
  /** @class basic_lock basic_messenger.hh <ylmm/basic_messenger.hh>
      @brief Single threaded locking object.  

      Used by ylmm::basic_messenger to insure thread safe static
      creation of the default output handler. This implementation does
      nothing.  In a real multi-threaded application, the user should
      pass a proper mutex locking class, defining at a minimum the
      lock and unlock member functions.  
      @see @ref thread_issues 
      @ingroup common
  */ 
  class basic_lock 
  {
  public:
    /** Aquire this lock.  This implemtation does nothing. */
    void lock() {}
    /** Release this lock.  This implemtation does nothing. */
    void unlock() {}
  };
  
  //================================================================
  /** @class basic_messenger basic_messenger.hh <ylmm/basic_messenger.hh>
      @brief Base class for output handling.
      @see @ref messenger_doc
      @param lock The type of thread syncronisation locks used in the 
      application.  Per default a single threaded locking mechanism
      (that is - no locks) is used. The application should pass the
      appropiate type here to make the use of this class thread safe.
      @ingroup common
   */
  template <typename Lock=basic_lock>
  class basic_messenger
  {
  private:
    static basic_messenger* _default_handler; /** The default handler
					       */
    static Lock _lock;		/** Lock used to insure safe thread
				    creation of the default handler */
  protected:
    std::ostream* _err_stream;	/** The stream to write errors to */
    std::ostream* _msg_stream;	/** The stream to write messages to */
    /// 
    /** The real print method
	@param o      Output stream
	@param format @c printf like format string
	@param ap     variadic arugment list */
    void print(std::ostream* o, const char* format, va_list ap);
  public:
    /** Constructor. */
    basic_messenger(std::ostream& msg=std::cout, 
			 std::ostream& err=std::cerr);
    /** Destructor. */
    virtual ~basic_messenger() {}

    //@{
    /// @name Handling streams
    /** Set stream to write errors to. 
	@param err_str The stream to write to
	@return @a err_str */
    std::ostream& error_stream(std::ostream& err_str);  
    /** Get the error stream 
	@return the error stream */
    std::ostream& error_stream() { return *_err_stream; }
    /** Set stream to write messages to. 
	@param msg_str The stream to write to 
	@return @a msg_str */
    std::ostream& message_stream(std::ostream& msg_str); 
    /** Get the messages stream 
	@return the messages stream */
    std::ostream& message_stream() { return *_msg_stream; }    
    //@}

    //@{
    /// @name Normal messages
    /** Print a message on the message stream. 
	@param format The @c printf like format string */
    virtual void message(const char* format, ...);
    /** Print a message on the message stream. 
	@param format The @c printf like format string
	@param ap The variadic arguments. */
    virtual void message(const char* format, va_list ap);
    //@}
    
    //@{
    /// @name Error messages 
    /** Print a error on the error stream. 
	@param format The @c printf like format string */
    virtual void error(const char* format, ...);
    /** Print a error on the error stream. 
	@param format The @c printf like format string 
	@param ap The variadic arguments. */
    virtual void error(const char* format, va_list ap);
    //@}
    
    /** Get the default handler.
	@return the default handler. */
    static basic_messenger* default_handler();
  };
  //__________________________________________________________________
  template<typename L>
  inline 
  basic_messenger<L>::basic_messenger(std::ostream& msg_str,
					     std::ostream& err_str)
  {
    _msg_stream = 0;
    _err_stream = 0;
    message_stream(msg_str);
    error_stream(err_str);
  }
  //__________________________________________________________________
  template<typename L>
  inline std::ostream& 
  basic_messenger<L>::error_stream(std::ostream& s) 
  {
    _err_stream = &s;
    return *_err_stream;
  }
  //__________________________________________________________________
  template<typename L>
  inline std::ostream& 
  basic_messenger<L>::message_stream(std::ostream& s) 
  {
    _msg_stream = &s;
    return *_msg_stream;
  }
  //__________________________________________________________________
  template<typename L>
  inline void 
  basic_messenger<L>::print(std::ostream* o, const char* f, va_list ap)
  {
    static char buf[1024];
    if (!o || o->bad()) return;
#ifdef HAVE_VSNPRINTF
    vsnprintf(buf, 1024, f, ap);
#else 
    vsprintf(buf, f, ap);
#endif
    *o << buf << std::flush;
  }    
  //__________________________________________________________________
  template<typename L>
  inline void 
  basic_messenger<L>::message(const char* msg, ...) 
  {
    va_list ap;
    va_start(ap, msg);
    message(msg, ap);
    va_end(ap);
  }  
  //__________________________________________________________________
  template<typename L>
  inline void 
  basic_messenger<L>::message(const char* msg, va_list ap) 
  {
    print(_msg_stream, msg, ap);
  }
  //__________________________________________________________________
  template<typename L>
  inline void 
  basic_messenger<L>::error(const char* err, ...) 
  {
    va_list ap;
    va_start(ap, err);
    error(err, ap);
    va_end(ap);
  }  
  //__________________________________________________________________
  template<typename L>
  inline void 
  basic_messenger<L>::error(const char* err, va_list ap) 
  {
    print(_err_stream, err, ap);
  }
  //__________________________________________________________________
  template<typename L>
  inline basic_messenger<L>*
  basic_messenger<L>::default_handler() 
  {
    // Double checking locking idom to avoid race conditions. 
    if (!_default_handler) {
      _lock.lock();
      if (!_default_handler) 
	_default_handler = new basic_messenger(std::cout,std::cerr);
      _lock.unlock();
    }
    return _default_handler;
  }
  //__________________________________________________________________
  template<typename L> basic_messenger<L>* 
  basic_messenger<L>::_default_handler = 0;
  //__________________________________________________________________
  template<typename L> L basic_messenger<L>::_lock;
}

#endif
//____________________________________________________________________
//
// EOF
//
