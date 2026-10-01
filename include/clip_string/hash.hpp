// hash.hpp 
//
// ripped directly from https://create.stephan-brumme.com/fnv-hash/ or https://github.com/lcn2/fnv
//
// JAF 9/30/2026

#ifndef CLIPSTRING_HASH_H
#define CLIPSTRING_HASH_H 


#include "ClipString.hpp"

template<std::size_t kPtrBytes>
struct fnv_inits{static_assert((kPtrBytes==8) || (kPtrBytes==4), "kPtrBtes must be 4 or 8"); };

template<std::size_t kPtrBytes>
std::size_t fnv1a_impl(const void* data, std::size_t numBytes){ return -1; } // silent fallback

template<>
std::size_t fnv1a_impl<4>(const void* data, std::size_t numBytes) // silent fallback
{
  const std::uint8_t* ptr = static_cast<const std::uint8_t*>(data);
  constexpr std::size_t kPrime = 0x01000193; //   16777619
  std::size_t hash = 0x811C9DC5; // 2166136261 
  while (numBytes--)
  {
    hash ^= (*ptr); 
    hash *= kPrime;
    ++ptr;
  }
  return hash;
}

template<>
std::size_t fnv1a_impl<8>(const void* data, std::size_t numBytes)
{
  const std::uint8_t* ptr = static_cast<const std::uint8_t*>(data);
  constexpr std::size_t kPrime = 0x00000100000001b3; //   1099511628211
  std::size_t hash = 0xcbf29ce484222325; // 14695981039346656037 
  while (numBytes--)
  {
    hash ^= (*ptr); 
    hash *= kPrime;
    ++ptr;
  }
  return hash;
}

std::size_t fnv1a(const void* data, std::size_t numBytes){ return fnv1a_impl<sizeof(std::size_t)>(data,numBytes); }

namespace std{

template<std::size_t kSize, typename CharT, typename Traits>
struct hash<ClipString<kSize,CharT,Traits>>
{
  std::size_t operator()(const ClipString<kSize,CharT,Traits>& str) const
  {
    return fnv1a(str.data(), sizeof(CharT)*str.length());
  }
};

} // end namespace std

#endif //hash.hpp