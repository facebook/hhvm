//
// $Id: basic_scanner.hh,v 1.14 2004/10/30 10:52:50 cholm Exp $
//
//  basic_scanner.hh
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
#ifndef YLMM_basic_scanner
#define YLMM_basic_scanner
#ifndef __VECTOR__
# include <vector>
#endif
#ifndef YLMM_basic_messenger
# include <util/ylmm/basic_messenger.hh>
#endif
#ifndef YLMM_basic_buffer
# include <util/ylmm/basic_buffer.hh>
#endif
#ifndef YLMM_basic_location
# include <util/ylmm/basic_location.hh>
#endif
#ifdef output
# undef output
#endif
#ifdef input
# undef input
#endif
#ifdef unput
# undef unput
#endif

/**  @file   basic_scanner.hh
     @author Christian Holm Christensen
     @date   Mon Sep 30 00:08:22 2002
     @brief  Declaration of scanner ABC. */

namespace ylmm
{
  //=====================================================================
  /** @class scanner_base basic_scanner.hh <ylmm/basic_scanner.hh>
      @brief ABC for scanner classes.

      This defines the common interface for scanner classes, that
      doesn't depend on the semantic and location type.  It's
      interface can safely be used in the application or via some
      library.
      @param lock The type of thread syncronisation locks used in the
      application.  Per default a single threaded locking mechanism
      (that is - no locks) is used. The application should pass the
      appropiate type here to make the use of this class thread safe.
      @ingroup lex
  */
  template <typename Lock=basic_lock>
  class scanner_base
  {
  public:
    /// Locking type
    typedef Lock lock_type;
    /// Stack type of buffers.
    typedef std::vector<basic_buffer*> buffer_stack;
  protected:
    basic_messenger<lock_type>* _messenger; /** The output handler */
    buffer_stack _buffers;	/** Stack (LIFO) of scanner buffers */
    basic_buffer* _current;	/** Current buffer we're reading
				    from. */
    /** Constructor.
	@param stream Stream to parse from. A buffer is created to
	hold this stream. */
    scanner_base(std::istream& stream);
    /** Constructor.
	@param buf Buffer to parse from.  If null, then a buffer
	reading from std::cin is set up and used. */
    scanner_base(basic_buffer* buf);
    /** Destructor.
	This free's the stack of buffer states.  If you need the
	buffers, you better copy them out before deleting the
	scanner. */
    virtual ~scanner_base();
  public:
    //@{
    /// @name Abstract interface
    /** Get the start condition
	@return the start condition */
    virtual int start_condition() const = 0;
    /** Get type of parsed token
	@return type of parsed token  */
    virtual int type() const = 0;
    /** Get the text
	@return the text read */
    virtual const char* text() const = 0;
    /** Get the length of the text.
	@return the length of the text read */
    virtual int length() const = 0;
    /** Start scanning
	@return read token ID */
    virtual int scan() = 0;
    //@}

    //@{
    /// @name Input handling
    /** Read a single character from the input.
	@return character read. */
    int read();
    /** Get input from current buffer.
	@param buf Buffer to read into.
	@param result number of characters read.
	@param max maximum number of characters to read.
	@return number of charaters read */
    int read(char* buf, int& result, int max);
    /** Put one character back on input stream.
	@param c character to put back */
    void putback(int c);
    //@}

    //@{
    /// @name Buffer handling
    /** When reacing end of file, this member function is called.
	@return 0 if there are no more buffers to read from, 1
	otherwise. */
    virtual int wrap();
    /** Get the current buffer.
	@return current buffer. */
    basic_buffer* current_buffer() const { return _current; }
    /** Switch to a new scanner buffer.
	@param buf The new buffer.  If 0, old is restored. */
    void switch_buffer(basic_buffer* buf);
    /** Get the list of buffers.
	Be careful manipulating this stack.  If you add a buffer to
	the end of it, you may get yourself into a lot of trouble.
	This class mainly exposes this member function so that one may
	copy the buffer stack to some other location, if needed.
	@return the buffer stack */
    const buffer_stack& buffers() const { return _buffers; }
    //@}

    //@{
    /// @name output handler
    /** Get the output handler
	@return The output handler used by this parser. */
    virtual const basic_messenger<lock_type>& messenger() const;
    /** Set and get the output handler
	@param o The output handler to use.
	@return The output handler used by this parser. */
    virtual basic_messenger<lock_type>& messenger(basic_messenger<lock_type>& o);
    //@}

    //@{
    /// @name Output handling
    /** Signal a fatal error.
	@param msg Error message. */
    virtual void fatal(const char* msg);
    /** Echo text to output
	@param text The text to print.
	@param length The number of chars (including terminator) in
	text. */
    virtual void echo(const char* text, int length);
    /** Echo character to output
	@param c The character to print */
    virtual void echo(const char c);
    /** Service function for printing messages on message stream.
	@param msg the message to print, possibly with @c printf
	like field descriptors. */
    void message(const char* msg, ...);
    /** Service function for printing messages on error stream.
	@param msg the message to print, possibly with @c printf
	like field descriptors. */
    void error(const char* msg, ...);
    //@}
  };
  //__________________________________________________________________
  template <typename L>
  inline scanner_base<L>::scanner_base(basic_buffer* buf)
  {
    _messenger = basic_messenger<L>::default_handler();
    _current = 0;
    if (!buf) buf = new basic_buffer(std::cin, true);
    switch_buffer(buf);
  }
  //__________________________________________________________________
  template <typename L>
  inline scanner_base<L>::scanner_base(std::istream& stream)
  {
    _messenger = basic_messenger<L>::default_handler();
    _current = 0;
    basic_buffer* buf = new basic_buffer(stream, true);
    switch_buffer(buf);
  }
  //__________________________________________________________________
  template <typename L>
  inline scanner_base<L>::~scanner_base()
  {
    for (buffer_stack::iterator i = _buffers.begin();
	 i != _buffers.end(); i++)
      delete (*i);
  }

  //__________________________________________________________________
  template <typename L>
  inline int scanner_base<L>::wrap()
  {
    // Try to restore an old buffer, if it exists.
    switch_buffer(0);
    return _current ? 0 : 1;
  }
  //__________________________________________________________________
  template <typename L>
  inline int scanner_base<L>::read(char* text, int& result, int max)
  {
    result = 0;
    if (!_current)
      fatal("No buffer to read from");
    else
      result = _current->read(text, max);
    return result;
  }
  //__________________________________________________________________
  template <typename L>
  inline int scanner_base<L>::read()
  {
    if (!_current) return 0;
    char c = _current->read();
    return c;
  }
  //__________________________________________________________________
  template <typename L>
  inline void scanner_base<L>::putback(int c)
  {
    if (!_current)
      fatal("No buffer to put back to");
    else
      _current->putback(char(c));
  }
  //__________________________________________________________________
  template <typename L>
  inline void scanner_base<L>::switch_buffer(basic_buffer* buf)
  {
    // If 0 was passed, try to restore old buffer.
    if (!buf) {
      if (!_current) {
	fatal("no current buffer");
	return;
      }
      // Check the stack for more buffers
      if (_buffers.size() == 0) {
	delete _current;
	_current = 0;
	// If there are no more, then return and EOF
	return;
      }

      // Pop the top element of the stack
      buf = _buffers.back();
      _buffers.pop_back();

      // and then switch to it
      if (!buf->activate()) {
	fatal("Failed to switch back to old buffer");
	return;
      }
      // Free the memory associated with the last buffer
      delete _current;
      _current = buf;
    }
    // Otherwise, we've got a new buffer to read from
    else {
      // First we ativate the new buffer ...
      if (!buf->activate()) {
	/// If that fails, we continue with the old one.
	fatal("Failed to switch to new buffer");
	return;
      }
      // Otherwise, if we have a buffer. we push it on the stack ...
      if (_current) _buffers.push_back(_current);
      // and then we make the new buffer the current one.
      _current = buf;
    }
  }
  //__________________________________________________________________
  template <typename L>
  inline const basic_messenger<L>&
  scanner_base<L>::messenger() const
  {
    return *_messenger;
  }

  //__________________________________________________________________
  template <typename L>
  inline basic_messenger<L>&
  scanner_base<L>::messenger(basic_messenger<lock_type>& output)
  {
    _messenger = &output;
    return *_messenger;
  }
  //__________________________________________________________________
  template <typename L>
  inline void scanner_base<L>::echo(const char* text, int)
  {
    if (!_messenger) return;
    _messenger->message(text);
  }
  //__________________________________________________________________
  template <typename L>
  inline void scanner_base<L>::echo(const char c)
  {
    if (!_messenger) return;
    _messenger->message("%c",c);
  }
  //__________________________________________________________________
  template <typename L>
  inline void scanner_base<L>::fatal(const char* text)
  {
    if (_messenger) {
      if (_current && _current->auto_increment())
	_messenger->error("@ %d, %d: ",
			  _current->line(), _current->column());
      _messenger->error(text);
    }
  }
  //__________________________________________________________________
  template <typename L>
  inline void scanner_base<L>::error(const char* msg,...)
  {
    if (!_messenger) return;
    va_list ap;
    va_start(ap, msg);
    _messenger->error(msg, ap);
    va_end(ap);
  }

  //__________________________________________________________________
  template <typename L>
  inline void scanner_base<L>::message(const char* msg, ...)
  {
    if (!_messenger) return;
    va_list ap;
    va_start(ap, msg);
    _messenger->message(msg, ap);
    va_end(ap);
  }

  //=====================================================================
  /** @class basic_scanner basic_scanner.hh <ylmm/basic_scanner.hh>
      @brief Basic scanner implementation.

      This templated class implements a scanner, or rather, a specific
      instantation of this tempalte implmentes it.  To use it, define
      the the processor macro @c YLMM_SCANNER_CLASS to a specific
      instance of this template, or a sub-class ofa specific instance
      in your @b Lex input file to.  Note that the third argument,
      @p id can be used to disambiqutate two scanners with the same
      @p Token,  @p Location pair.
      See also @ref scanner_doc
      @param Token the type of the tokens used in the grammar.
      @param Location the location type used, if applicable for the
      backend parser generator (Normal @b Yacc does not support that,
      but @b Bison does).
      @param id is an integer that uniquely identifies the class.
      @param lock The type of thread syncronisation locks used in the
      application.  Per default a single threaded locking mechanism
      (that is - no locks) is used. The application should pass the
      appropiate type here to make the use of this class thread safe.
      @ingroup lex
  */
  template<typename Token, typename Location=basic_location,
	   int id=0, typename Lock=basic_lock>
  class basic_scanner : public scanner_base<Lock>
  {
  public:
    /// Type of tokens
    typedef Token token_type;
    /// Type of locationss
    typedef Location location_type;
    /// Scanner id
    enum { scanner_id = id };
  protected:
    typedef scanner_base<Lock> base_type;
    int _type;			/** The last token type read via yylex
				 */
    token_type _token;		/** The last token read. */
  public:
    /** Constructor.
	@param buf Buffer to parse from.  If null, then a buffer
	reading from std::cin is set up and used. */
    basic_scanner(basic_buffer* buf=0) : scanner_base<Lock>(buf) {}
    /** Constructor.
	@param stream Stream to parse from. */
    basic_scanner(std::istream& stream) : scanner_base<Lock>(stream) {}
    //@{
    /// @name @c yylex Interface
    /** Get the current start condition.
	This member function is implemented in lexmm.hh
	@return the current start condition.  */
    virtual int start_condition() const;
    /** Get the current input text.
	This member function is implemented in lexmm.hh
	@return current input text. */
    virtual const char* text() const;
    /** Get the length of the current input text.
	This member function is implemented in lexmm.hh
	@return length of the current input text. */
    virtual int length() const;
    /** The main method of this class.
	The member _type is set to the token type, and it's value is
	returned to the caller.  Also, the internal members, _token
	and _location are should be set by the handlers defined in the
	input file to @b Lex.  The parser may use this member function
	directly from ylmm::basic_parser::scan, but then it's up to
	the handlers to set the appropiate value of the token (and
	possibly the location) in the parser.  See also general
	comments in the documentation.
	@return token ID number. */
    virtual int scan();
    //@}

    //@{
    /// @name Parser interface.
    /** Get the last read token type.
	@return last read token type. */
    int type() const { return _type; }
    /** Get the last read token.
	Updates the token information in place.  That is, the token
	passed is updated to the last read token value.  The
	@c token_type @e must be assignable.
	@param t The token value. */
    void token(token_type& t) { t = _token; }
    /** Get the last read position.
	Updates the location information in-place in the passed
	argument @p l.  If the @p f argument is @c true modify the
	current mark (start of region) too.
	@param l The location to update in.
	@param f If true, also update the mark to point to last	point. */
    void where(location_type& l, bool f=true);
    /** A more generic interface to the scanner.
	The parser may call this member function from it scan method
	to pop the next token.  The point is, that often one need to
	do a look-up on identifiers and similar before returning the
	current token type to the LALR(1) parser (think typedefs in C,
	classes and templates in C++), so this function can be
	overloaded by a derived class to do that translation.  The
	default implementation just reads a token from the input and
	returns the value to the caller.
	@param t The token value to update.
	@param l The location value to update.
	@param f If true, update the mark of the region too.
	@return The token type number. */
    virtual int next(token_type& t, location_type& l, bool f=true);
    /** Like above, but without the location argument.
	@param t The token value to update.
	@return The token type number. */
    virtual int next(token_type& t);
    //@}
  };

  //__________________________________________________________________
  template<typename T, typename L, int id, typename M>
  inline void
  basic_scanner<T,L,id,M>::where(location_type& l, bool first)
  {
    if (!base_type::_current) return;
    if (first) {
      l.first_line(l.last_line());
      l.first_column(l.last_column());
    }
    l.last_line(base_type::_current->line());
    l.last_column(base_type::_current->column());
  }
  //__________________________________________________________________
  template<typename T, typename L, int id, typename M>
  inline int
  basic_scanner<T,L,id,M>::next(token_type& t,
				location_type& l, bool f)
  {
    scan();
    token(t);
    where(l,f);
    return type();
  }
  //__________________________________________________________________
  template<typename T, typename L, int id, typename M>
  inline int
  basic_scanner<T,L,id,M>::next(token_type& t)
  {
    scan();
    token(t);
    return type();
  }
}
#endif
//____________________________________________________________________
//
// EOF
//

