// ClipString.hpp
//
// stack allocated string that stores the exact minimum # of bytes
// 
// e.g. sizeof(ClipString<N,CharT>) == (N + 1) * sizeof(CharT). for N CharT + 1 null terminator. 
// 
// in the event of an operation that would be truncated. 
// the capacity will shrink to N-1 
// and a persistent boolean flag will be set true.
// 
// if a null ptr is ever passed into a CharT* argument, 
// a different persisten boolean flag is set.
//
// JAF 9/25/2026

#include<cstdint>
#include<cstring>
#include<cassert>
#include<array>
#include<algorithm>
#include<string> // still need std::char_traits
#include<iostream> 

#ifndef _CLIPSTRING_CLIPSTRING_H
#define _CLIPSTRING_CLIPSTRING_H

using std::cout;

template<std::size_t kSize, typename CharT = char, typename Traits = std::char_traits<CharT>>
class ClipString
{
  static_assert(kSize > 0, "must pass size >= 1 to ClipString template parameters");
  static_assert(std::is_trivial_v<CharT>);
  static_assert(std::is_standard_layout_v<CharT>);
  static_assert(std::is_integral_v<CharT>);

  private:
    // Friends ------------------- 
    template<std::size_t kSizeOther, typename CharU, typename TraitsU>
    friend class ClipString;

    // Type defs + Flags -----------------
    template<std::size_t kSizeOther>
    using CopyableClipString = ClipString<kSizeOther,CharT,Traits>; // easier than asserting std::is_same at every step

    using UnsignedCharT = typename std::make_unsigned<CharT>::type;
    static_assert(sizeof(UnsignedCharT) == sizeof(CharT));
    static_assert(alignof(UnsignedCharT) == alignof(CharT));

  public:
    using traits_type = Traits;
    using value_type = CharT;
    using allocator_type = void;
    using reference = CharT&; 
    using const_reference = const CharT&;
    using size_type = std::size_t; 
    using different_type = std::ptrdiff_t;
    using pointer = CharT*;
    using const_pointer = const CharT*;
    using iterator = CharT*;
    using const_iterator = const CharT*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    static constexpr size_type npos = -1; 
    struct Flags{
      static constexpr UnsignedCharT Clipped = UnsignedCharT{1}  << (CHAR_BIT*sizeof(CharT)-1);
      static constexpr UnsignedCharT NullptrPassed = UnsignedCharT{1}  << (CHAR_BIT*sizeof(CharT)-2);
      static constexpr UnsignedCharT FlagsMask = (Clipped | NullptrPassed);
      static constexpr UnsignedCharT SlackMask = static_cast<UnsignedCharT>(~FlagsMask);
      static constexpr UnsignedCharT ExtraSlack = UnsignedCharT{1}  << (CHAR_BIT*sizeof(CharT)-3);
      static constexpr UnsignedCharT ShortSlackMask = static_cast<UnsignedCharT>(~(FlagsMask | ExtraSlack));
    };

    // Constructors + destructor ---------------------

    // default
    constexpr ClipString() noexcept
    {
      clear();
    }

    constexpr ClipString(size_type count, CharT ch) noexcept
    {
      assign(count, ch);
    }

    template< class InputIt >
    constexpr ClipString( InputIt first, InputIt last) noexcept
    {
      assign<InputIt>(first,last);
    }

    constexpr ClipString(const CharT* s, size_type count) noexcept
    {
      assign(s,count);
    }

    constexpr ClipString( const CharT* s) noexcept
    {
      assign(s);
    }

    template<class StringViewLike>
    constexpr ClipString( const StringViewLike& t) noexcept // C++17 only
    {
      assign<StringViewLike>(t);
    }

    template<class StringViewLike>
    constexpr ClipString( const StringViewLike& t, size_type pos, size_type count) // C++17 only
    {
      assign<StringViewLike>(t,pos,count);
    }

    template<std::size_t kSizeOther>
    constexpr ClipString(const CopyableClipString<kSizeOther>& s) noexcept
    {
      assign<kSizeOther>(s);
    }

    template<std::size_t kSizeOther>
    constexpr ClipString(const CopyableClipString<kSizeOther>& s, size_type pos)
    {
      assign<kSizeOther>(s,pos);
    }

    template<std::size_t kSizeOther>
    constexpr ClipString(const CopyableClipString<kSizeOther>& s, size_type pos, size_type count)
    {
      assign<kSizeOther>(s,pos,count);
    }

    constexpr ClipString( std::initializer_list<CharT> ilist ) noexcept
    {
      assign(ilist);
    }

    // ------------------------
    // Assign 
    // ------------------------
    constexpr ClipString& assign(size_type count, CharT ch) noexcept
    {
      if(count <= kSize)
      {
        traits_type::assign(data(), std::min(count,kSize), ch);
        traits_type::assign(data()[count], CharT{});
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask));
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::assign(data(), kSize-1, ch);
        traits_type::assign(data()[kSize-1], CharT{});
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      return *this;
    }

    template< class InputIt >
    constexpr ClipString&  assign( InputIt first, InputIt last) noexcept
    {
      size_type count = std::distance(first, last); // TODO not actually single pass through range
      if(count <= kSize)
      {
        auto ptr = data();
        auto lam = [&ptr](const CharT& ch){traits_type::assign(*ptr, ch); ++ptr; };
        std::for_each_n(first, count, lam);
        traits_type::assign(*ptr, CharT{}); // null terminate
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask)); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        auto ptr = data();
        auto lam = [&ptr](const CharT& ch){traits_type::assign(*ptr, ch); ++ptr; };
        std::for_each_n(first, kSize-1, lam);
        traits_type::assign(*ptr, CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      return *this;
    }

    constexpr ClipString& assign(const CharT* s, size_type count) noexcept
    {
      if(!s)
      {
        clear();
        set_flags(Flags::NullptrPassed);
        return *this;
      }
      if(count <= kSize)
      {
        traits_type::copy(data(), s, count);
        traits_type::assign(data()[count], CharT{}); // null terminate
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask)); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::copy(data(), s, kSize-1);
        traits_type::assign(data()[kSize-1], CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      return *this;
    }

    constexpr ClipString& assign( const CharT* s) noexcept
    {
      if(!s)
      {
        clear();
        set_flags(Flags::NullptrPassed);
        return *this;
      }
      size_type count = traits_type::length(s);
      if(count <= kSize)
      {
        traits_type::copy(data(), s, count);
        traits_type::assign(data()[count], CharT{}); // null terminate
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask)); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::copy(data(), s, kSize-1);
        traits_type::assign(data()[kSize-1], CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      return *this;
    }

    template<class StringViewLike>
    constexpr ClipString& assign( const StringViewLike& t) noexcept // C++17 only
    {
      std::basic_string_view<CharT, Traits> sv = t;
      size_type count = sv.length();
      if(count <= kSize)
      {
        traits_type::copy(data(), sv.data(), count);
        traits_type::assign(data()[count], CharT{}); // null terminate
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask)); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::copy(data(), sv.data(), kSize-1);
        traits_type::assign(data()[kSize-1], CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }      
      return *this;
    }

    template<class StringViewLike>
    constexpr ClipString& assign( const StringViewLike& t, size_type pos, size_type count) // C++17 only
    {
      if(pos > t.length()) throw std::out_of_range{};
      std::basic_string_view<CharT, Traits> sv = t;
      count = std::min(count, t.length() - pos);
      if(count <= kSize)
      {
        traits_type::copy(data(), sv.data()+pos, count);
        traits_type::assign(data()[count], CharT{}); // null terminate
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask)); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::copy(data(), sv.data()+pos, kSize-1);
        traits_type::assign(data()[kSize-1], CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }      
      return *this;
    }

    template<std::size_t kSizeOther>
    constexpr ClipString& assign(const CopyableClipString<kSizeOther>& s) noexcept
    {
      size_type count = s.length();
      if(count <= kSize)
      {
        traits_type::copy(data(), s.c_str(), count);
        traits_type::assign(data()[count], CharT{}); // null terminate
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask)); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::copy(data(), s.c_str(), kSize-1);
        traits_type::assign(data()[kSize-1], CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      UnsignedCharT f = flags() | (s.flags() & Flags::FlagsMask);
      set_flags(f);
      return *this;
    }

    template<std::size_t kSizeOther>
    constexpr ClipString& assign(const CopyableClipString<kSizeOther>& s, size_type pos)
    {
      if(pos > s.length()) throw std::out_of_range{};
      size_type count = s.length() - pos;
      if(count <= kSize)
      {
        traits_type::copy(data(), s.c_str() + pos, count);
        traits_type::assign(data()[count], CharT{}); // null terminate
        set_flags(static_cast<UnsignedCharT>(~Flags::FlagsMask)); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::copy(data(), s.c_str() + pos, kSize-1);
        traits_type::assign(data()[kSize-1], CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      UnsignedCharT f = flags() | (s.flags() & Flags::FlagsMask);
      set_flags(f);
      return *this;
    }

    template<std::size_t kSizeOther>
    constexpr ClipString& assign(const CopyableClipString<kSizeOther>& s, size_type pos, size_type count)
    {
      if(pos > s.length()) throw std::out_of_range{};
      count = std::min(s.length() - pos, count);
      if(count <= kSize)
      {
        traits_type::copy(data(), s.c_str() + pos, count);
        traits_type::assign(data()[count], CharT{}); // null terminate
        set_flags(0x00); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        traits_type::copy(data(), s.c_str() + pos, kSize-1);
        traits_type::assign(data()[kSize-1], CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      UnsignedCharT f = flags() | (s.flags() & Flags::FlagsMask);
      set_flags(f);
      return *this;
    }

    constexpr ClipString& assign( std::initializer_list<CharT> ilist ) noexcept
    {
      size_type count = ilist.size();
      auto first = ilist.begin();
      if(count <= kSize)
      {
        auto ptr = data();
        auto lam = [&ptr](const CharT& ch){traits_type::assign(*ptr, ch); ++ptr; };
        std::for_each_n(first, count, lam);
        traits_type::assign(*ptr, CharT{}); // null terminate
        set_flags(0x00); // flags are all false
        set_slack(kSize-count);
      }
      else
      {
        // need to set flags + clip
        auto ptr = data();
        auto lam = [&ptr](const CharT& ch){traits_type::assign(*ptr, ch); ++ptr; };
        std::for_each_n(first, kSize-1, lam);
        traits_type::assign(*ptr, CharT{}); // null terminate
        set_flags(Flags::Clipped);
        set_slack(1);
      }
      return *this;
    }

    // destructor
    ~ClipString()=default;

    // Member Functions -----------------
    bool clipped() const noexcept { return ((flags() & Flags::Clipped)!=0); }
    bool null_passed() const noexcept { return ((flags() & Flags::NullptrPassed)!=0); }

    // Element access ------------------

    constexpr reference at(size_type i)
    {
      if(i >= size()) throw std::out_of_range{};
      return data()[i];
    }
    constexpr const_reference at(size_type i) const
    {
      if(i >= size()) throw std::out_of_range{};
      return data()[i];
    }
    constexpr reference operator[](size_type i) noexcept { return data()[i]; }
    constexpr const_reference operator[](size_type i) const noexcept{ return data()[i]; }
    constexpr reference front() noexcept { return data()[0]; }
    constexpr const_reference front() const noexcept{ return data()[0]; }
    constexpr reference back() noexcept { return data()[size()-1]; }
    constexpr const_reference back() const noexcept{ return data()[size()-1]; }
    constexpr pointer data() noexcept { return reinterpret_cast<CharT*>(m_data.data()); }
    constexpr const_pointer data() const noexcept { return reinterpret_cast<const CharT*>(m_data.data()); }
    constexpr const_pointer c_str() const noexcept { return reinterpret_cast<const CharT*>(m_data.data()); }
    constexpr operator std::basic_string_view<CharT,Traits>() const noexcept { return {data(), size()}; } // C++17 only

    // Iterators -------------- 
    constexpr iterator begin() noexcept { return data(); }
    constexpr const_iterator begin() const noexcept { return data(); }
    constexpr const_iterator cbegin() const noexcept { return data(); }
    constexpr iterator end() noexcept { return data() + size(); }
    constexpr const_iterator end() const noexcept { return data() + size(); }
    constexpr const_iterator cend() const noexcept { return data() + size(); }
    constexpr reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(cend()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }
    constexpr reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(cbegin()); }
    constexpr const_reverse_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }

    // Capacity ---------------- 
    constexpr bool empty() const noexcept { return (slack()==kSize); }
    constexpr size_type size() const noexcept { return kSize - slack(); }
    constexpr size_type length() const noexcept { return kSize - slack(); }
    static constexpr size_type max_size() noexcept { return kSize; }
    void reserve()=delete;
    size_type capacity() const noexcept { return ((flags() & Flags::FlagsMask)!=0) ? kSize-1 : kSize; } // if flags are set. can't be null terminator
    size_type shrink_to_fit()=delete;

    // Modifiers ----------------- 
    constexpr void clear() noexcept
    { 
      CharT zero{};
      std::memcpy(m_data.data(), &zero, sizeof(zero));
      set_flags(0x00);
      set_slack(kSize);
    }

    // --------------
    // Insert 
    // --------------

    // --------------
    // erase
    // --------------

    // --------------
    // push_back
    // --------------

    // --------------
    // pop_back
    // --------------

    // --------------
    // append
    // --------------

    // --------------
    // opertator+=
    // --------------

    // --------------
    // replace
    // --------------

    // --------------
    // copy
    // --------------

    // --------------
    // resize
    // --------------
    constexpr void resize( size_type count, CharT ch ={} )
    {
      if(count > capacity())
      {
        throw std::length_error{c_str()};
      }
      std::size_t s = size();
      if(count > s)
      {
        traits_type::assign(data() + s, count - s, ch);
        traits_type::assign(data()[count],CharT{});
        set_slack(kSize - count);
      }
      else
      {
        traits_type::assign(data()[count],CharT{});
        set_slack(kSize - count);
      }
    }

  public: // TODO make protected
    // Implementations ------------------
    UnsignedCharT flags() const noexcept 
    {
      UnsignedCharT flags; 
      std::memcpy(&flags, &m_data[sizeof(CharT) * kSize], sizeof(UnsignedCharT));
      return flags;
    }

    void set_flags(UnsignedCharT f) noexcept
    {
      std::memcpy(&m_data[sizeof(CharT) * kSize], &f, sizeof(UnsignedCharT));
    }

    size_type slack() const
    {
      if constexpr(kSize <= ((std::size_t{1} << (CHAR_BIT*sizeof(CharT)-2))-1))
      {
        return static_cast<size_type>(flags() & Flags::SlackMask); 
      }
      else
      {
        if(( flags() & Flags::ExtraSlack )!=0)
        {
          // store the std::size_t in the bytes just before m_flags.
          // since we hold a much smaller string it should never collide with this data. 
          std::size_t result;
          std::memcpy(&result, &m_data[sizeof(CharT)*(kSize)-sizeof(std::size_t)], sizeof(std::size_t));
          return result;
        }
        else
        {
          return (flags() & Flags::SlackMask);
        }
      }
    }
    void set_slack(size_type slack)
    {
      if constexpr(kSize <= ((std::size_t{1} << (CHAR_BIT*sizeof(CharT)-2))-1))
      {
        UnsignedCharT f = (flags() & Flags::FlagsMask) | static_cast<UnsignedCharT>(slack);
        set_flags(f);
      }
      else
      {
        if(slack <= ((std::size_t{1} << (CHAR_BIT*sizeof(CharT)-3))-1))
        {
          UnsignedCharT f = (flags() & Flags::FlagsMask) | static_cast<UnsignedCharT>(slack); 
          set_flags(f & ~Flags::ExtraSlack);
        }
        else
        {
          UnsignedCharT f = (flags() & Flags::FlagsMask) | Flags::ExtraSlack;
          set_flags(f);
          // store the std::size_t in the bytes just before m_flags.
          // since we hold a much smaller string it should never collide with this data. 
          std::memcpy(&m_data[sizeof(CharT)*kSize-sizeof(std::size_t)], &slack, sizeof(std::size_t));
        }
      }
    }

    // Member Data
    alignas(CharT) std::array<std::uint8_t, (kSize+1)*sizeof(CharT)> m_data;
    // kSize CharT character slots + one UnsignedCharT metadata slot.
    // When no flags are set and slack == 0, the metadata slot is zero
    // and therefore serves as the null terminator for a kSize-character string.
    // When flags are set, the maximum null-terminated string length is kSize-1.
};

#endif // ClipString.hpp