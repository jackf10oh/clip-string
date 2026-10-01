// io.hpp
//
// operator >> and << for std::basic_ostream, std::basic_istream, etc
// get_line for std::basic_istream.
//
// JAF 9/30/2026

#ifndef CLIPSTRING_IO_H
#define CLIPSTRING_IO_H

#include "ClipString.hpp"

// operator<< -------------
template<std::size_t kSize, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const ClipString<kSize,CharT,Traits>& str )
{
  typedef typename std::basic_ostream<CharT, Traits>::sentry sentry_type; 
  sentry_type s(os);
  if(!s) return os;

  const std::streamsize len = static_cast<std::streamsize>(str.length());
  const std::streamsize width = os.width();
  if(width <= len)
  {
    if(os.rdbuf()->sputn(str.c_str(), len)!=len)
    {
      os.setstate(std::ios_base::failbit);
    }
  }
  else
  {
    if((os.flags() & std::ios_base::adjustfield) == std::ios_base::left)
    {
      if(os.rdbuf()->sputn(str.c_str(), len)!=len)
      {
        os.setstate(std::ios_base::failbit);
      }
      for (std::streamsize pad = width - len; pad != 0; --pad)
      {
          if (os.rdbuf()->sputc(os.fill()) == Traits::eof())
          {
              os.setstate(std::ios_base::failbit);
              break;
          }
      }
    }
    else
    {
      for (std::streamsize pad = width - len; pad != 0; --pad)
      {
          if (os.rdbuf()->sputc(os.fill()) == Traits::eof())
          {
              os.setstate(std::ios_base::failbit);
              break;
          }
      }     
      if(os && os.rdbuf()->sputn(str.c_str(), len)!=len)
      {
        os.setstate(std::ios_base::failbit);
      }
    }
  }
  os.width(0);
  return os;
}

// operator>> -------------
template<std::size_t kSize, typename CharT, typename Traits>
std::basic_istream<CharT,Traits>& operator>>(std::basic_istream<CharT,Traits>& in, ClipString<kSize,CharT,Traits>& str)
{
  // create sentry. check input stream is ok
  typedef typename std::basic_istream<CharT, Traits>::sentry sentry_type; 
  sentry_type ok(in);
  if(!ok)
  {
    in.setstate(std::ios_base::failbit);
    return in;
  }

  // empty out the string
  str.clear();

  const std::ctype<CharT>& ct = std::use_facet<std::ctype<CharT>>(in.getloc());

  // read up to kSize characters
  typename ClipString<kSize,CharT,Traits>::size_type i = 0;
  typename std::basic_istream<CharT, Traits>::int_type c;
  CharT ch;
  CharT ws = in.widen(' ');
  bool cleanly;
  while(true)
  {
    c = in.peek(); 
    ch = Traits::to_char_type(c);
    // if next character is whitespace or EOF
    if(Traits::eq_int_type(c,Traits::eof()) || ct.is(std::ctype_base::space, ch))
    {
      cleanly = (i != ( kSize + 1 ));
      break;
    }
    in.get(); // consume the character
    
    // a little sketchy. we write kSize+1 characters into the buffer. 
    // which isn't an error since there's N+1 room counting the null terminator. 
    // but we have to keep incrementing i past kSize to see if we are truncating 
    // the istream or not...
    if(i<kSize+1) 
    {
      Traits::assign(str[i],ch); // write into ClipStrin
      ++i;
    }
  }
  if(cleanly)
  {
    str.set_slack(kSize-i); // set the slack 
    Traits::assign(str[i],CharT{}); // set the null terminator
  }
  else
  {
    typedef typename ClipString<kSize,CharT,Traits>::Flags flags_type;
    typedef typename ClipString<kSize,CharT,Traits>::UnsignedCharT uchar_type;
    str.set_flags(flags_type::Clipped | uchar_type{1}); // sets clipped to true. slack to 1 
    Traits::assign(str[kSize-1],CharT{}); // set the null terminator
  }
  // nothing written from istream 
  if(i==0) in.setstate(std::ios_base::failbit);
  in.width(0); // clear width
  return in;
}

// getline ----------------
template<std::size_t kSize, typename CharT, typename Traits>
std::basic_istream<CharT,Traits>& getline(std::basic_istream<CharT,Traits>& in, ClipString<kSize,CharT,Traits>& str)
{
  return getline(in,str,in.widen('\n'));
}

template<std::size_t kSize, typename CharT, typename Traits>
std::basic_istream<CharT,Traits>& getline(std::basic_istream<CharT,Traits>& in, ClipString<kSize,CharT,Traits>& str, CharT delim)
{
  // create sentry. check input stream is ok
  typedef typename std::basic_istream<CharT, Traits>::sentry sentry_type; 
  sentry_type ok(in);
  if(!ok)
  {
    in.setstate(std::ios_base::failbit);
    return in;
  }

  // empty out the string
  str.clear();

  const std::ctype<CharT>& ct = std::use_facet<std::ctype<CharT>>(in.getloc());

  // read up to kSize characters
  typename ClipString<kSize,CharT,Traits>::size_type i = 0;
  typename std::basic_istream<CharT, Traits>::int_type c;
  CharT ch;
  CharT ws = in.widen(' ');
  bool cleanly;
  while(true)
  {
    c = in.peek(); 
    ch = Traits::to_char_type(c);

    // if next character is EOF
    if(Traits::eq_int_type(c,Traits::eof()))
    {
      in.setstate(std::ios_base::eofbit);
      cleanly = (i != ( kSize + 1 ));
      break;
    }

    // if next character is delimiter
    if(Traits::eq(c,delim))
    {
      in.get(); // consume the character. without putting into str
      cleanly = (i != ( kSize + 1 ));
      break;
    }

    in.get(); // consume the character
    
    // a little sketchy. we write kSize+1 characters into the buffer. 
    // which isn't an error since there's N+1 room counting the null terminator. 
    // but we have to keep incrementing i past kSize to see if we are truncating 
    // the istream or not...
    if(i<kSize+1) 
    {
      Traits::assign(str[i],ch); // write into ClipStrin
      ++i;
    }
  }
  if(cleanly)
  {
    str.set_slack(kSize-i); // set the slack 
    Traits::assign(str[i],CharT{}); // set the null terminator
  }
  else
  {
    typedef typename ClipString<kSize,CharT,Traits>::Flags flags_type;
    typedef typename ClipString<kSize,CharT,Traits>::UnsignedCharT uchar_type;
    str.set_flags(flags_type::Clipped | uchar_type{1}); // sets clipped to true. slack to 1 
    Traits::assign(str[kSize-1],CharT{}); // set the null terminator
  }
  // nothing written from istream 
  if(i==0) in.setstate(std::ios_base::failbit);
  return in;
}

#endif // io.hpp