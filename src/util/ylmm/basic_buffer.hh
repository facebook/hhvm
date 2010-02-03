// $Id: basic_buffer.hh,v 1.13 2004/03/09 02:38:01 cholm Exp $ 
//  
//  basic_buffer.hh
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
#ifndef YLMM_basic_buffer
#define YLMM_basic_buffer
#ifndef __IOSTREAM__
# include <iostream>
#endif
#ifndef __FSTREAM__
# include <fstream>
#endif
#ifndef __STDEXCEPT__
# include <stdexcept>
#endif

/**  @file   ylmm/basic_buffer.hh
     @author Christian Holm Christensen
     @date   Mon Sep 30 00:08:22 2002
     @brief  Declaration of scanner ABC. */

namespace ylmm 
{
  /** @class basic_buffer basic_buffer.hh <ylmm/basic_buffer.hh>
      @brief ABC for scanner buffers. 

      This class is used by the ylmm::basic_scanner class to read
      input.  Internally it uses a std::istream to read from.  User
      code can sub-class this class if need the client code needs to
      override things like line and column bookkeeping.  However, if
      the client code needs to read from something that isn't a file
      or the standard input (for example via readline), then the
      client code is much better off making a customised
      std::streambuf layer - it really isn't as hard as the standard
      leaves you thinking it may be, and it's really powerful. 
      @see @ref buffer_issues.

      To do a proper interface to @b Flex notion of buffers, there's
      an extra data member, and some extra member functions.  These
      are @e not used by other kinds implmentations of @b Lex.  
      @ingroup lex
  */
  class basic_buffer
  {
  private:
    void* _extra;		/** Extra data used by @b Flex */
  protected:
    std::istream* _stream;	/** Pointer to external input stream */
    bool _is_owner;		/** Do we own this stream? */
    bool _interactive;		/** Are we reading interactively or
				    not */
    int _line;			/** The current line number */
    int _column;		/** The current column number */
    bool _auto_increment;	/** Should we automatically set
				    locations */
    char _last_read;		/** Last read character*/

    //@{
    /// @name Automatic updates
    /** Do an automatic incrementation. 
	@param c the character read. */
    void increment_it(char c);
    //@}

    //@{
    /// @name Actual reading 
    /** Read at most @p max characters into buffer @p buf.  See also
	the public input method. 
	@param buf The buffer to read into
	@param max The maximum number of characters to read
	@return The number of characters read */
    virtual int read_buffered(char* buf, int max);
    /** Read at most @p max characters from input, or up to the first
	occurence of delimiter.  See also the public input method.  
	@param buf The buffer to read into		   
	@param max The maximum number of characters to read
	@param delim the delimiter to read to
	@return The number of characters read */
    virtual int read_interactive(char* buf, int max, char delim);
    //@}

    //@{
    /// @name Extra functions to help with @b Flex
    /** An extra action to take when creating a new buffer. 
	@param size is the size of the buffer */
    void new_extra(int size);
    /** An extra action to take when deallocating the buffer.*/
    void delete_extra(); 
    /** An extra action to take when flushing the buffer. */
    void flush_extra(); 
    /** An extra action to take when making interactive or buffered. 
	@param inter Whether it's interactive (ignored) */
    void interactive_extra(bool inter);
    /** An extra action to take when setting begning-of-line flag 
	@param bol ignored */
    void at_bol_extra(bool bol);
    //@}
  public:
    /** Make a new state.  
	@param stream Stream to read from.  This could be a
	@c std::istream like @c std::cin, or a @c std::stringstream
	refering to an external string buffer.  If argument is null (0)
	then the stream is set to @c std::cin and interactive reads
	are enabled. 
	@param inter Is this stream interactive or not. 
	@param autoinc Should we do automatic location tracking? That
	is should we set the _line and _column number automatically by
	parsing the read stream, or should we let the client code do
	that job for us. */
    basic_buffer(std::istream& stream, bool inter=true, bool autoinc=false);
    /** Make a new state. 
	@param filename Name of a file to read from. If this is `-' 
	then std::cin is opened for input.  If file can not be found,
	then this throws a std::runtime_error. 
	@param inter Is this stream interactive or not. 
	@param autoinc Should we do automatic location tracking? That
	is should we set the _line and _column number automatically by
	parsing the read stream, or should we let the client code do
	that job for us. 
	@exception std::runtime_error Thrown if named file cannot be
	found. */
    basic_buffer(const std::string& filename, 
		 bool inter=false, bool autoinc=true);
    /** Free this buffer.  
	If this buffer is the @e owner of the used input stream, that
	is, if this buffer opened it because it was constructed using
	the string constructor, then the underlying stream is closed
	and deleted here. Otherwise, it's up to the client code to
	free the memory associated with the stream, if needed. */
    virtual ~basic_buffer();

    //@{
    /// @name Stream
    /** Get the input stream 
	@return the input stream */
    std::istream* stream() { return _stream; }
    //@}

    //@{
    /// @name position handling 
    /** Get the line number 
	@return the line number */
    int line() const { return _line; }
    /** Get the column number 
	@return the column number */
    int column() const { return _column; }
    /** Increment line and possibly column 
	@param l number to add to current line number
	@param c number to add to current  column number */
    virtual void increment_line(int l, int c=0);
    /** Increment the column 
	@param c number to add to current  column number */
    virtual void increment_column(int c) { _column += c; }
    //@}

    //@{
    /// @name Read handling
    /** Flush the buffer */
    void flush() { flush_extra(); }
    /** Read one character from input 
	Note, when EOF is encountered, 0 (and not EOF) is returned. 
	@return the character read */
    virtual int read();
    /** Read at most @p max characters into buffer @p buf. 
	@param buf Buffer of at least size @p max to read into. 
	@param max Maximum number of characters to read from input. 
	@return The number of characters read.  Returns -1 in case of
	errors. */
    virtual int read(char* buf, int max);
    /** Put one character back into the stream. 
	@param c character to put back. */
    virtual void putback(int c);
    //@}

    //@{
    /// @name Miscellanious
    /** Are we doing automatic increments? 
	@return true if auto incrementing */
    bool auto_increment() const { return _auto_increment; }
    /** Set wether we should do automatic increments. 
	@param ai If true, auto increment, otherwise don't 
	@return true if auto incrementing */	
    bool auto_increment(bool ai) { return _auto_increment = ai; }
    /** Test if we're at the start of a line 
	@return true if last read characer was a newline */
    bool at_bol() const { return _last_read == '\n'; }
    /** Set wether we're at the start of a line 
	@param bol set the last read character to be a newline
	@return true if last read characer was a newline */
    bool at_bol(bool bol);
    /** Check if buffer is interactive 
	@return true if we're reading interactively */
    bool interactive() const { return _interactive; }
    /** Set whether buffer is interactive 
	@param inter if true, read inteactively
	@return true if we're reading interactively */
    bool interactive(bool inter);
    /** Called when this buffer is restored as current buffer. 
	@return true if this is the current buffer after activation */
    bool activate();
    //@}
  };

  //__________________________________________________________________
  inline 
  basic_buffer::basic_buffer(std::istream& stream, bool inter, 
			     bool autoinc) 
  {
    _interactive    = inter;
    _stream         = (!stream ? &std::cin : &stream);
    _line           = 0;
    _column         = 0;
    _is_owner       = false;
    _auto_increment = autoinc;
    _last_read      = '\n';
    if (_stream == &std::cin) _interactive = true;
    new_extra(0);
  }
  //__________________________________________________________________
  inline 
  basic_buffer::basic_buffer(const std::string& filename, bool inter, 
			     bool autoinc) 
  {
    _interactive    = inter;
    if (filename.empty()) throw std::runtime_error("no filename given");
    if (!filename.compare("-")) {
      _stream   = &std::cin;
      _is_owner = false;
    }
    else {
      _stream   = new std::ifstream(filename.data());
      if (!(*_stream) || _stream->bad()) {
	throw std::runtime_error("failed to open file");
      }
      _is_owner = true;
    }
    _line           = 0;
    _column         = 0;
    _auto_increment = autoinc;
    _last_read      = '\n';
    if (_stream == &std::cin) _interactive = true;
    new_extra(0);
  }

  //__________________________________________________________________
  inline 
  basic_buffer::~basic_buffer() 
  {
    delete_extra();
    if (_is_owner && _stream) {
      std::ifstream* is;
      if ((is = dynamic_cast<std::ifstream*>(_stream))) is->close();
      delete _stream;
    }
  }
  
  //__________________________________________________________________
  inline void basic_buffer::increment_it(char c)
  {
    switch(c) {
    case '\n': _line++; _column = 0; break;
    default:   _column++;
    }
  }

  //__________________________________________________________________
  inline int basic_buffer::read_buffered(char* buf, int max) 
  {
    if (!_stream || _stream->eof()) return 0;
    _stream->read(buf, max); 
    if (_stream->bad()) return 0;
    int c = _stream->gcount();
    if (c == 0) return 0;
    _last_read = buf[c-1];
    if (_auto_increment) { 
      for (int i = 0; i < c; i++) increment_it(buf[i]); }
    return c;
  }

  //__________________________________________________________________
  inline int basic_buffer::read_interactive(char* buf, int max, char delim) 
  {
    if (max <= 0 || !_stream || _stream->eof()) return 0;
    int i = 0;
    do {
      _last_read = _stream->get();
      if (_auto_increment) increment_it(_last_read);
      buf[i++] = _last_read;
      if (_stream->bad()) return -1;
    } while(i < max && _last_read != EOF && _last_read != delim);
    return i;
  }

  //__________________________________________________________________
  inline void basic_buffer::increment_line(int l, int c) 
  {
    if (_auto_increment) return; 
    _line = l; 
    _column = c; 
  }
  
  //__________________________________________________________________
  inline bool basic_buffer::at_bol(bool bol) 
  { 
    at_bol_extra(bol);
    _last_read = bol ? '\n' : '\0'; return bol; 
  }
  
  //__________________________________________________________________
  inline bool basic_buffer::interactive(bool inter) 
  { 
    interactive_extra(inter);
    return _interactive = inter; 
  }

  //__________________________________________________________________
  inline int basic_buffer::read() 
  { 
    _last_read = _stream->get();
    if (_stream->eof()) return 0;
    if (_stream->bad()) return -1;
    if (_auto_increment) increment_it(_last_read);
    return _last_read;
  }

  //__________________________________________________________________
  inline int basic_buffer::read(char* buf, int max) 
  { 
    if (_interactive) return read_interactive(buf, max, '\n');
    else return read_buffered(buf, max); 
  }

  //__________________________________________________________________
  inline void basic_buffer::putback(int c) 
  { 
    _stream->putback(char(c)); 
    if (!_stream->bad() && _auto_increment) _column--; 
  }
}

//__________________________________________________________________
/** Print a std::basic_buffer on an output stream
    @param o output stream to write to
    @param b the ylmm::basic_buffer to write out
    @return @a o */
inline std::ostream& 
operator<<(std::ostream& o, const ylmm::basic_buffer& b) 
{
  return o << b.line() << "," << b.column();
}

#if defined(FLEX_SCANNER) && !defined(YLMM_flex_buffer_impl)
#define YLMM_flex_buffer_impl

//__________________________________________________________________
/** Create a new Flex buffer. 
    @param file The file to associate with the buffer. Can be NULL.
    @param size The size of the buffer
    @return A newly allocated buffer. */
extern YY_BUFFER_STATE yy_create_buffer(FILE *file, int size);  

inline void 
ylmm::basic_buffer::new_extra(int size) 
{
  // If size is not given (0 or less), then use the default. 
  if (size < 0) { _extra = 0; return; }
  if (size == 0) size = YY_BUF_SIZE;
  _extra = static_cast<YY_BUFFER_STATE>(yy_create_buffer(0, size));
}

//__________________________________________________________________
/** De-allocate a Flex buffer. 
    @param b The buffer to de-allocate.  */
extern void yy_delete_buffer(YY_BUFFER_STATE b);

inline void 
ylmm::basic_buffer::delete_extra() 
{
  yy_delete_buffer(static_cast<YY_BUFFER_STATE>(_extra));
}

//__________________________________________________________________
/** Switch to another Flex buffer 
    @param b The buffer to switch to. */
extern void yy_switch_to_buffer(YY_BUFFER_STATE b);

inline bool 
ylmm::basic_buffer::activate() 
{
  yy_switch_to_buffer(static_cast<YY_BUFFER_STATE>(_extra));
  return static_cast<YY_BUFFER_STATE>(_extra) == YY_CURRENT_BUFFER;
}

//__________________________________________________________________
inline void 
ylmm::basic_buffer::interactive_extra(bool inter) 
{
  (static_cast<YY_BUFFER_STATE>(_extra))->yy_is_interactive = inter ? 1 : 0;
}

//__________________________________________________________________
inline void 
ylmm::basic_buffer::at_bol_extra(bool inter) 
{
  (static_cast<YY_BUFFER_STATE>(_extra))->yy_at_bol = inter ? 1 : 0;
}

//__________________________________________________________________
/** Flush a buffer. 
    @param b The buffer to flush. */
extern void yy_flush_buffer(YY_BUFFER_STATE b);

inline void 
ylmm::basic_buffer::flush_extra() 
{
  yy_flush_buffer(static_cast<YY_BUFFER_STATE>(_extra));
}
#else 

//__________________________________________________________________
inline void ylmm::basic_buffer::new_extra(int size) {}
inline void ylmm::basic_buffer::delete_extra() {}
inline void ylmm::basic_buffer::flush_extra() {}
inline void ylmm::basic_buffer::interactive_extra(bool) {}
inline void ylmm::basic_buffer::at_bol_extra(bool inter) {}
inline bool ylmm::basic_buffer::activate() { return true; }

#endif
#endif


    
    
