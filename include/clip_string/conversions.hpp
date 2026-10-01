// conversions.hpp
//
//
//
// JAF 9/30/2026

#ifndef CLIPSTRING_CONVERSIONS_H
#define CLIPSTRING_CONVERSIONS_H 

#include "ClipString.hpp"
#include<limits>
#include<cwchar>
// ========================
// Numeric conversions
// ========================

// stoi, stol, stoll ----------------
template<std::size_t kSize, typename Traits>
int stoi(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  long result = std::strtol(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  bool narrowing = (result < std::numeric_limits<int>::min()) || (result > std::numeric_limits<int>::max()); 
  if((errno==ERANGE) || narrowing) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return static_cast<int>(result);
}

template<std::size_t kSize, typename Traits>
long stol(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  long result = std::strtol(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
long long stoll(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  long long result = std::strtoll(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

// stoi, stol, stoll (wchar_t) ----------------
template<std::size_t kSize, typename Traits>
int stoi(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  long result = std::wcstol(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  bool narrowing = (result < std::numeric_limits<int>::min()) || (result > std::numeric_limits<int>::max()); 
  if((errno==ERANGE) || narrowing) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return static_cast<int>(result);
}

template<std::size_t kSize, typename Traits>
long stol(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  long result = std::wcstol(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
long long stoll(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  long long result = std::wcstoll(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

// stoul, stoull ----------------
template<std::size_t kSize, typename Traits>
unsigned long stoul(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  unsigned long result = std::strtoul(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
unsigned long long stoull(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  unsigned long long result = std::strtoull(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

// stoul, stoull (wchar_t) ----------------
template<std::size_t kSize, typename Traits>
unsigned long stoul(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  unsigned long result = std::wcstoul(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
unsigned long long stoull(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr, int base = 10)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  unsigned long long result = std::wcstoull(str, &ptr, base); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

// stof, stod, stold ----------------
template<std::size_t kSize, typename Traits>
float stof(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  float result = std::strtof(str, &ptr); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
double stod(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  double result = std::strtod(str, &ptr); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
long double stold(const ClipString<kSize,char,Traits>& s, std::size_t* pos = nullptr)
{
  const char* str = s.c_str();
  errno = 0;
  char* ptr{};
  long double result = std::strtold(str, &ptr); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

// stof, stod, stold (wchar_t) ----------------
template<std::size_t kSize, typename Traits>
float stof(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  float result = std::wcstof(str, &ptr); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
double stod(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  double result = std::wcstod(str, &ptr); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

template<std::size_t kSize, typename Traits>
long double stold(const ClipString<kSize,wchar_t,Traits>& s, std::size_t* pos = nullptr)
{
  const wchar_t* str = s.c_str();
  errno = 0;
  wchar_t* ptr{};
  long double result = std::wcstold(str, &ptr); 
  if(str==ptr) throw std::invalid_argument{};
  if(errno==ERANGE) throw std::out_of_range{str};
  if(pos!=nullptr) *pos = (ptr - str);
  return result;
}

// ================================
// To String
// ================================

template<std::size_t kSize, typename Traits>
void to_string( int value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%i", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%l", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( long long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%ll", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( unsigned value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%u", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( unsigned long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%lu", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( unsigned long long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%llu", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( float value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%f", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( double value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%f", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_string( long double value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::snprintf(dest.data(), kSize+1, "%Lf", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( int value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%d", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%ld", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( long long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%lld", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( unsigned value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%u", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( unsigned long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%lu", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( unsigned long long value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%llu", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( float value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%f", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( double value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%f", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

template<std::size_t kSize, typename Traits>
void to_wstring( long double value, ClipString<kSize,char,Traits>& dest)
{
  int planned = std::swprintf(dest.data(), kSize+1, L"%Lf", value);
  if(planned<0) throw std::exception{};
  if(planned<=kSize+1)
  {
    dest.set_slack(kSize+1-planned);
  }
  else
  {
    typedef typename ClipString<kSize,char,Traits>::Flags Flags;
    typedef typename ClipString<kSize,char,Traits>::UnsignedCharT UnsignedCharT;
    Traits::assign(dest[kSize-1],char{});
    dest.set_flags(Flags::Clipped | UnsignedCharT{1});
  }
}

#endif