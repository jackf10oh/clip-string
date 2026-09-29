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
#include <string_view>
#include<iostream> 
#include<type_traits>

#ifndef CLIPSTRING_CLIPSTRING_H
#define CLIPSTRING_CLIPSTRING_H

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

    template<typename T>
    struct is_sv_convertible : std::conjunction<std::is_convertible<const T&, const CharT*>, std::negation<std::is_convertible<const T&, const CharT*>>>{};

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
    using difference_type = std::ptrdiff_t;
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

    // ====================
    // Constructors  
    // ====================
    constexpr ClipString() noexcept
    {
      clear();
    }

    constexpr ClipString(size_type count, CharT ch) noexcept
    {
      assign(count, ch);
    }

    template<class InputIt>
    constexpr ClipString(InputIt first, InputIt last) noexcept
    {
      assign<InputIt>(first,last);
    }

    constexpr ClipString(const CharT* s, size_type count) noexcept
    {
      assign(s,count);
    }

    constexpr ClipString(const CharT* s) noexcept
    {
      assign(s);
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    constexpr ClipString(const StringViewLike& t) noexcept // C++17 only
    {
      assign<StringViewLike>(t);
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
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

    ClipString( std::nullptr_t ) = delete;

    // destructor ------------------------
    ~ClipString()=default;

    // Assign ------------------------ 
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
    constexpr ClipString&  assign(InputIt first, InputIt last) noexcept
    {
      // copy up to kSize entries out of [first, last)
      size_type count = 0;
      while((first!=last) && (count!=kSize))
      {
        traits_type::assign(data()[count], *first);
        ++count;
        ++first;
      }
      // if we wrote exactly kSize entries and haven't reach the end of [first,last) -> clipped the range
      if((count==kSize) && (first!=last))
      {
        if(first!=last)
        {
          set_flags(Flags::Clipped);
          set_slack(1);
          traits_type::assign(data()[kSize-1],CharT{});
        }
        else
        {
          set_flags(0x00); // set all flags to false, sets ExtraSlack bit to false, also sets slack to 0
        }
      }
      else
      {
        set_flags(0x00);
        set_slack(kSize-count);
        traits_type::assign(data()[count],CharT{});
      }
      return *this;
    }

    constexpr ClipString& assign(const CharT* s, size_type count) noexcept
    {
      if(s == nullptr)
      {
        clear();
        set_flags(flags() | Flags::NullptrPassed);
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
      if(s == nullptr)
      {
        clear();
        set_flags( flags() | Flags::NullptrPassed);
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

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
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

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
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

    // operator= -------------------------
    template<std::size_t kSizeOther>
    constexpr ClipString& operator=(const CopyableClipString<kSizeOther>& other) noexcept
    {
      return assign(other);
    }

    constexpr ClipString& operator=(const CharT* s) noexcept
    {
      return assign(s);
    }

    constexpr ClipString& operator=(CharT ch) noexcept
    {
      return assign(std::addressof(ch),1);
    }

    constexpr ClipString& operator=( std::initializer_list<CharT> ilist ) noexcept
    {
      return assign(ilist);
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    ClipString& operator=( const StringViewLike& t )
    {
      return assign(t);
    }

    ClipString& operator=( std::nullptr_t ) = delete;

    // get_allocate (deleted) ---------------------
    void get_allocator() const = delete;

    // ===========================
    // Member Functions
    // ===========================
    bool clipped() const noexcept { return ((flags() & Flags::Clipped)!=0); }
    bool null_passed() const noexcept { return ((flags() & Flags::NullptrPassed)!=0); }

    // ===========================
    // Element access 
    // ===========================
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
    constexpr pointer data() noexcept { return m_data.data(); }
    constexpr const_pointer data() const noexcept { return m_data.data(); }
    constexpr const_pointer c_str() const noexcept { return m_data.data(); }
    constexpr operator std::basic_string_view<CharT,Traits>() const noexcept { return {c_str(), size()}; } // C++17 only

    // ===========================
    // Iterators
    // ===========================
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

    // ===========================
    // Capacity  
    // ===========================
    constexpr bool empty() const noexcept { return (slack()==kSize); }
    constexpr size_type size() const noexcept { return kSize - slack(); }
    constexpr size_type length() const noexcept { return kSize - slack(); }
    static constexpr size_type max_size() noexcept { return kSize; }
    void reserve()=delete;
    size_type capacity() const noexcept { return ((flags() & Flags::FlagsMask)!=0) ? kSize-1 : kSize; } // if flags are set. can't be null terminator
    size_type shrink_to_fit()=delete;

    // ===========================
    // Modifiers  
    // ===========================

    // clear ----------------------
    constexpr void clear() noexcept
    { 
      traits_type::assign(data()[0], CharT{});
      set_flags(0x00);
      set_slack(kSize);
    }

    // Insert -------------- 
    constexpr ClipString& insert(size_type index, size_type count, CharT ch)
    {
      if(index > size()) throw std::out_of_range{c_str()};

      // if inserting this many were to cause clipping 
      std::size_t c = capacity();
      std::size_t s = size(); 

        
      if((s + count) > c) // clipping will occur
      {
        if((index + count) > c)
        {
          // clip happens somewhere in [index, index + count)
          traits_type::assign(data() + index, kSize - 1 - index, ch);
          traits_type::assign(data()[kSize-1], CharT{});
        }
        else
        {
          // clip happens somewhere in [index + count, size + count)
          std::size_t moved = kSize - 1 - (index + count);
          traits_type::move(data() + index + count, data() + index, moved); // move with a clip 
          traits_type::assign(data()[kSize-1], CharT{}); // null terminate
          traits_type::assign(data() + index, count, ch);
        }
        set_flags(flags() | Flags::Clipped);
        set_slack(1);   
      }
      else // safely copy everything
      {
        traits_type::move(data() + index + count, data() + index, s - index);
        traits_type::assign(data() + index, count, ch);
        traits_type::assign(data()[s + count], CharT{});
        set_slack(kSize - (s + count));
      }
      return *this;
    }

    constexpr ClipString& insert(size_type index, const CharT* s)
    {
      if(index > size()) throw std::out_of_range{c_str()};

      // guard against nullptr
      if(s == nullptr)
      {
        set_flags(flags() | Flags::NullptrPassed);
        // setting last bits to != null terminator -> may have to back up 1 character and clip something
        if(slack()==0)
        {
          set_flags(flags() | Flags::Clipped);
          set_slack(1);
          traits_type::assign(data()[kSize-1], CharT{});
        }
        return *this;
      }
      // if inserting this many were to cause clipping 
      std::size_t c = capacity();
      std::size_t sz = size(); 
      std::size_t count = traits_type::length(s);
        
      if((sz + count) > c) // clipping will occur
      {
        if((index + count) > c)
        {
          // clip happens somewhere in [index, index + count)
          traits_type::copy(data() + index, s, kSize - 1 - index);
          traits_type::assign(data()[kSize-1], CharT{});
        }
        else
        {
          // clip happens somewhere in [index + count, size + count)
          std::size_t moved = kSize - 1 - (index + count);
          traits_type::move(data() + index + count, data() + index, moved); // move with a clip 
          traits_type::assign(data()[kSize-1], CharT{}); // null terminate
          traits_type::copy(data() + index, s, count);
        }
        set_flags(flags() | Flags::Clipped);
        set_slack(1);   
      }
      else // safely copy everything
      {
        traits_type::move(data() + index + count, data() + index, sz - index);
        traits_type::copy(data() + index, s, count);
        traits_type::assign(data()[sz + count], CharT{});
        set_slack(kSize - (sz + count));
      }
      return *this;
    }

    constexpr ClipString& insert(size_type index, const CharT* s, size_type count)
    {
      if(index > size()) throw std::out_of_range{c_str()};

      // guard against nullptr
      if(s == nullptr)
      {
        set_flags(flags() | Flags::NullptrPassed);
        // setting last bits to != null terminator -> may have to back up 1 character and clip something
        if(slack()==0)
        {
          set_flags(flags() | Flags::Clipped);
          set_slack(1);
          traits_type::assign(data()[kSize-1], CharT{});
        }
        return *this;
      }
      // if inserting this many were to cause clipping 
      std::size_t c = capacity();
      std::size_t sz = size(); 
        
      if((sz + count) > c) // clipping will occur
      {
        if((index + count) > c)
        {
          // clip happens somewhere in [index, index + count)
          traits_type::copy(data() + index, s, kSize - 1 - index);
          traits_type::assign(data()[kSize-1], CharT{});
        }
        else
        {
          // clip happens somewhere in [index + count, size + count)
          std::size_t moved = kSize - 1 - (index + count);
          traits_type::move(data() + index + count, data() + index, moved); // move with a clip 
          traits_type::assign(data()[kSize-1], CharT{}); // null terminate
          traits_type::copy(data() + index, s, count);
        }
        set_flags(flags() | Flags::Clipped);
        set_slack(1);   
      }
      else // safely copy everything
      {
        traits_type::move(data() + index + count, data() + index, sz - index);
        traits_type::copy(data() + index, s, count);
        traits_type::assign(data()[sz + count], CharT{});
        set_slack(kSize - (sz + count));
      }
      return *this;
    }

    template<std::size_t kSizeOther>
    constexpr ClipString& insert(size_type index, const CopyableClipString<kSizeOther>& other)
    {
      // first copy the flags from other string
      UnsignedCharT f_other = other.flags() & Flags::FlagsMask;
      if(f_other != 0)
      {
        std::size_t slk = this->slack();
        if(slk != 0)
        {
          set_flags(flags() | f_other);
        }
        else
        {
          set_flags(flags() | f_other | Flags::Clipped);
          set_slack(1);
        }
      }
      return insert(index, other.c_str(), other.size());
    }

    template<std::size_t kSizeOther>
    constexpr ClipString& insert(size_type index, const CopyableClipString<kSizeOther>& str, size_type s_index, size_type count = npos )
    {
      if(s_index > str.size()) throw std::out_of_range{str.c_str()};
      // first copy the flags from other string
      UnsignedCharT f_other = str.flags() & Flags::FlagsMask;
      if(f_other != 0)
      {
        std::size_t slk = this->slack();
        if(slk != 0)
        {
          set_flags(flags() | f_other);
        }
        else
        {
          set_flags(flags() | f_other | Flags::Clipped);
          set_slack(1);
        }
      }
      return insert(index, str.c_str() + s_index, std::min(count, str.size()-s_index));
    }

    constexpr iterator insert( const_iterator pos, CharT ch ) noexcept
    {
      difference_type index = std::distance(cbegin(), pos);
      insert(index, 1, ch);
      return std::next(begin(), index);
    }

    constexpr iterator insert( const_iterator pos, size_type count, CharT ch ) noexcept
    {
      difference_type index = std::distance(cbegin(), pos);
      insert(index, count, ch);
      return std::next(begin(), index);
    }

    template< class InputIt >
    constexpr iterator insert( const_iterator pos, InputIt first, InputIt last ) noexcept
    {
      difference_type index = std::distance(cbegin(), pos);
      insert(index, ClipString(first,last));
      return std::next(begin(), index);
    }

    constexpr iterator insert( const_iterator pos, std::initializer_list<CharT> ilist ) noexcept
    {
      difference_type index = std::distance(cbegin(), pos);
      insert(index, ClipString(ilist));
      return std::next(begin(), index);
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    constexpr ClipString& insert( size_type index, const StringViewLike& t )
    {
      std::basic_string_view<CharT, Traits> sv = t;
      return insert(index, sv.c_str(), sv.length());
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    constexpr ClipString& insert(size_type index, const StringViewLike& t, size_type t_index, size_type count = npos)
    {
      std::basic_string_view<CharT,Traits> sv(t);
      if(t_index > sv.length()) throw std::out_of_range{sv.c_str()};
      return insert(index, sv.c_str() + t_index, std::min(count, sv.length()-t_index));
    }

    // erase --------------
    constexpr ClipString& erase( size_type index = 0, size_type count = npos )
    {
      size_type sz = size(); 
      if(index > sz) throw std::out_of_range{c_str()};
      size_type erased = std::min(count, sz - index);
      size_type trailing = sz - (index + erased);
      traits_type::move(data() + index, data() + index + erased, trailing); // shift left 
      traits_type::assign(data()[index + trailing], CharT{}); // null terminate
      // update slack
      set_slack(index + trailing);
      return *this;
    }

    constexpr iterator erase( const_iterator position ) noexcept
    {
      size_type index = std::distance(cbegin(), position);
      size_type sz = size(); 
      size_type trailing = sz - (index + 1);
      traits_type::move(data() + index, data() + index + 1, trailing); // shift left 
      traits_type::assign(data()[index + trailing], CharT{}); // null terminate
      // update slack
      set_slack(index + trailing);
      return *this;
    }

    constexpr iterator erase( const_iterator first, const_iterator last ) noexcept
    {
      size_type sz = size(); 
      size_type index = std::distance(cbegin(), first);
      size_type erased = std::distance(first,last);
      size_type trailing = sz - (index + erased);
      traits_type::move(data() + index, data() + index + erased, trailing); // shift left 
      traits_type::assign(data()[index + trailing], CharT{}); // null terminate
      // update slack
      set_slack(index + trailing);
      return *this;
    }

    // copy -----------------
    constexpr size_type copy( CharT* dest, size_type count, size_type pos = 0 ) const
    {
      size_type sz = size(); 
      if(pos > sz) throw std::out_of_range{c_str()};
      count = std::min(sz - pos, count);
      traits_type::copy(dest, data() + pos, count);
      return count;
    }

    // push_back --------------
    void push_back(CharT ch) noexcept
    {
      UnsignedCharT f = flags() & Flags::FlagsMask;
      size_type s = slack();
      // considered a clip if any flag is set, and at size kSize-1. or if at size kSize.
      // maybe useful to silently toggle NullptrPass? 
      bool possible = (f!=0) ? (s>1) : (s!=0);
      if(possible)
      {
        traits_type::assign(data()[kSize-s], ch);
        traits_type::assign(data()[kSize-s+1], CharT{});
        set_slack(s-1);
      }
      else
      {
        traits_type::assign(data()[kSize-1], CharT{}); // clip last character.
        set_flags(f | Flags::Clipped); // set clipped as true. 
        set_slack(1);
      }
    }

    // pop_back --------------
    constexpr void pop_back() noexcept
    {
      // UB if empty() == true
      std::size_t s = slack();
      traits_type::assign(data()[kSize - s - 1],CharT{});
      set_slack(s + 1);
    }

    // append --------------
    constexpr ClipString& append(size_type count, CharT ch) noexcept { return insert(size(), count, ch); } 
    constexpr ClipString& append(const CharT* s, size_type count) noexcept { return insert(size(), s, count); } 
    constexpr ClipString& append(const CharT* s) noexcept { return insert(size(), s); } 
    template<class SV, typename = std::enable_if_t<is_sv_convertible<SV>::value>>
    constexpr ClipString& append( const SV& t ) noexcept { return insert<SV>(size(), t); }
    template<class SV, typename = std::enable_if_t<is_sv_convertible<SV>::value>>
    constexpr ClipString& append(const SV& t, size_type pos, size_type count = npos){ return insert(size(), t, pos, count); }
    template<std::size_t kSizeOther>    
    constexpr ClipString& append(const CopyableClipString<kSizeOther>& str) noexcept { return insert(size(), str); } 
    template<std::size_t kSizeOther>    
    constexpr ClipString& append(const CopyableClipString<kSizeOther>& str, size_type pos, size_type count = npos ){ return insert(size(), str, pos, count); }
    template<class InputIt>
    constexpr ClipString& append(InputIt first, InputIt last) noexcept { return insert<InputIt>(size(), first, last); }
    constexpr ClipString& append( std::initializer_list<CharT> ilist ) noexcept { return insert(size(), ilist); }
    
    // opertator+= --------------
    template<std::size_t kSizeOther>
    constexpr ClipString& operator+=(const CopyableClipString<kSizeOther>& str) noexcept { return append<kSizeOther>(str); }
    constexpr ClipString& operator+=(CharT ch) noexcept { push_back(ch); return *this; }
    constexpr ClipString& operator+=( const CharT* s ) noexcept { return append(s); }
    constexpr ClipString& operator+=(std::initializer_list<CharT> ilist) noexcept { return append(ilist); }
    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    ClipString& operator+=( const StringViewLike& t ) noexcept { return append<StringViewLike>(t); }

    // replace --------------
    template<std::size_t kSizeOther>
    constexpr ClipString& replace( size_type pos, size_type count, const CopyableClipString<kSizeOther>& str )
    {
      UnsignedCharT f_other = str.get_flags() & Flags::FlagsMask; 
      if(f_other!=0)
      {
        size_type slk = slack(); 
        if(slk==0)
        {
          set_flags(flags() | f_other | Flags::Clipped); 
          set_slack(1);
          traits_type::assign(data()[kSize-1], CharT{});
        }
        else
        {
          set_flags(flags() | f_other); 
        }
      }
      return replace(pos, count, str.c_str(), str.length());
    }

    template<std::size_t kSizeOther>
    constexpr ClipString& replace(const_iterator first, const_iterator last, const CopyableClipString<kSizeOther>& str) noexcept
    {
      size_type pos = std::distance(cbegin(), first);
      size_type count = std::distance(first, last); 
      return replace(pos,count, str);
    }

    template<std::size_t kSizeOther>
    constexpr ClipString& replace( size_type pos, size_type count, const CopyableClipString<kSizeOther>& str, size_type pos2, size_type count2 = npos)
    {
      UnsignedCharT f_other = str.get_flags() & Flags::FlagsMask; 
      if(f_other!=0)
      {
        size_type slk = slack(); 
        if(slk==0)
        {
          set_flags(flags() | f_other | Flags::Clipped); 
          set_slack(1);
          traits_type::assign(data()[kSize-1], CharT{});
        }
        else
        {
          set_flags(flags() | f_other); 
        }
      }
      if(pos2 > str.size()) throw std::out_of_range{str.c_str()};
      count2 = std::min(str.length() - pos2, count2);
      return replace(pos, count, str.c_str() + pos2, count2);
    }

    constexpr ClipString& replace(size_type pos, size_type count, const CharT* cstr, size_type count2)
    {
      size_type sz = size();
      if(pos > sz) throw std::out_of_range(c_str());
      count = std::min(count, sz - pos);

      // check for null ptr
      if(cstr==nullptr)
      {
        size_type slk = slack(); 
        if(slk==0) // full size!
        {
          // have to back up size by 1. -> clipped something
          traits_type::assign(data()[kSize-1],CharT{});
          set_flags(flags() | Flags::NullptrPassed | Flags::Clipped);
          set_slack(1);
        }
        else
        {
          set_flags(flags() | Flags::NullptrPassed);
        }
        return *this;
      }      

      if(count == count2) // no need to worry about clipping
      {
        traits_type::copy(data() + pos, cstr, count);
      }
      else if(count > count2) // still no need to worry about clipping
      {
        traits_type::copy(data() + pos, cstr, count2); // copy cstrinto this
        traits_type::move(data() + pos + count2, data() + pos + count, sz - (pos + count)); // move trailing to the left
        sz = sz - (count - count2); // calculate new size
        traits_type::assign(data()[sz],CharT{}); // null terminator 
        set_slack(kSize - sz); // update slack
      }
      else // at risk of clipping characters when expanding
      {
        size_type future_sz = sz - count + count2;
        if(future_sz > capacity())
        {
          // a clip is necessary
          traits_type::assign(data()[kSize-1],CharT{}); // null terminate
  
          // shrink count2 if that is what would cause the truncation to occur
          count2 = std::min((kSize-1)-pos, count2);
          
          // how much data can be moved rightward? 
          size_type moveable = (kSize-1) - pos - count2;

          traits_type::move(data() + pos + count2, data() + pos + count, moveable);
          
          // copy into pos
          traits_type::copy(data()+pos, cstr,count2);

          set_flags(flags() | Flags::Clipped); // set the clipped flag
          set_slack(1);
        }
        else
        {
          // no clip needed
          traits_type::assign(data()[future_sz],CharT{}); // null terminate
          traits_type::move(data() + pos + count2, data() + pos + count, sz - pos - count); // shift right
          traits_type::copy(data()+pos, cstr,count2); // copy into pos
          set_slack(kSize-future_sz);
        }
      }
      return *this;
    }

    constexpr ClipString& replace(const_iterator first, const_iterator last, const CharT* cstr, size_type count2) noexcept
    {
      size_type pos = std::distance(cbegin(), first);
      size_type count = std::distance(first,last);
      return replace(pos,count,cstr,count2);
    }

    constexpr ClipString& replace(size_type pos, size_type count, const CharT* cstr)
    {
      if(cstr == nullptr)
      {
        if(slack()==0) // full size!
        {
          // back up by 1. -> clipped
          traits_type::assign(data()[kSize-1],CharT{});
          set_flags(flags() | Flags::NullptrPassed | Flags::Clipped);
          set_slack(1); 
        }
        else
        {
          set_flags(flags() | Flags::NullptrPassed);
        }
        return *this;
      } 
      return replace(pos,count,cstr,traits_type::length(cstr)); 
    }

    constexpr ClipString& replace( const_iterator first, const_iterator last, const CharT* cstr ) noexcept
    {
      size_type pos = std::distance(cbegin(), first);
      size_type count = std::distance(first,last);
      return replace(pos,count,cstr);
    }
                          
    constexpr ClipString& replace(size_type pos, size_type count, size_type count2, CharT ch)
    {
      size_type sz = size();
      if(pos > sz) throw std::out_of_range(c_str());
      count = std::min(count, sz - pos);

      if(count == count2) // no need to worry about clipping
      {
        traits_type::assign(data() + pos, count, ch);
      }
      else if(count > count2) // still no need to worry about clipping
      {
        traits_type::assign(data() + pos, count2, ch); // copy cstrinto this
        traits_type::move(data() + pos + count2, data() + pos + count, sz - (pos + count)); // move trailing to the left
        sz = sz - (count - count2); // calculate new size
        traits_type::assign(data()[sz],CharT{}); // null terminator 
        set_slack(kSize - sz); // update slack
      }
      else // at risk of clipping characters when expanding
      {
        size_type future_sz = sz - count + count2;
        if(future_sz > capacity())
        {
          // a clip is necessary
          traits_type::assign(data()[kSize-1],CharT{}); // null terminate
  
          // shrink count2 if that is what would cause the truncation to occur
          count2 = std::min((kSize-1)-pos, count2);
          
          // how much data can be moved rightward? 
          size_type moveable = (kSize-1) - pos - count2;
          traits_type::move(data() + pos + count2, data() + pos + count, moveable);
          
          // copy into pos
          traits_type::assign(data()+pos,count2, ch);

          set_flags(flags() | Flags::Clipped); // set the clipped flag
          set_slack(1);
        }
        else
        {
          // no clip needed
          traits_type::assign(data()[future_sz],CharT{}); // null terminate
          traits_type::move(data() + pos + count2, data() + pos + count, sz - pos - count); // shift right
          traits_type::assign(data()+pos, count2, ch); // copy into pos
          set_slack(kSize-future_sz);
        }
      }
      return *this;
    }

    template< class InputIt >
    constexpr ClipString& replace(const_iterator first, const_iterator last,InputIt first2, InputIt last2) noexcept 
    {
      ClipString temp(first2,last2);
      return replace(first,last,temp);
    }

    constexpr ClipString& replace( const_iterator first, const_iterator last, std::initializer_list<CharT> ilist ) noexcept
    {
      ClipString temp(ilist);
      return replace(first,last,temp);
    }  
                          
    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    constexpr ClipString& replace(size_type pos, size_type count, const StringViewLike& t)
    {
      std::basic_string_view<CharT,Traits> sv(t);
      return replace(pos,count,sv.c_str(), sv.length());
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    constexpr ClipString& replace(const_iterator first, const_iterator last, const StringViewLike& t) noexcept
    {
      std::basic_string_view<CharT,Traits> sv(t);
      return replace(first,last,sv.c_str(), sv.length());
    }                   

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    constexpr ClipString& replace(size_type pos, size_type count, const StringViewLike& t,size_type pos2, size_type count2 = npos)
    {
      std::basic_string_view<CharT,Traits> sv(t);
      size_type actual_count = std::min(count2, sv.length() - pos2);
      return replace(pos,count,sv.c_str() + pos2, actual_count);
    }

    // resize --------------
    constexpr void resize( size_type count, CharT ch = CharT{} )
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

    // swap --------------
    template<std::size_t kSizeOther>
    constexpr void swap(CopyableClipString<kSizeOther>& other) noexcept
    {
      CopyableClipString<kSizeOther> temp(other); 
      other.assign(*this);
      assign(temp);
      set_flags(temp.flags() & Flags::FlagsMask);
    } 

    // -----------------------
    // Search
    // -----------------------

    // find ------------------ 
    template<std::size_t kSizeOther>
    constexpr size_type find(const CopyableClipString<kSizeOther>& str, size_type pos=0) const noexcept
    {
      return find(str.c_str(), pos, str.length());
    }

    constexpr size_type find(const CharT* str, size_type pos, size_type count) const noexcept
    {
      if(!str) return npos;
      size_type sz = size();
      if(count > sz) return npos;
      size_type end = sz - count;
      for (; pos < end; ++pos)
      {
        if (traits_type::compare(data() + pos, str, count) == 0)
        {
          return pos;
        }
      }
      return npos;
    }

    constexpr size_type find(const CharT* str, size_type pos=0) const noexcept
    {
      if(!str) return npos;
      return find(str, pos, traits_type::length(str));
    }

    size_type find( CharT ch, size_type pos = 0 ) const noexcept
    {
      size_type result = npos;
      const size_type sz = this->size();
      if (pos < sz)
      {
        const size_type n = sz - pos;
        const CharT* ptr = traits_type::find(data() + pos, n, ch);
        if (ptr) result = ptr - data();
      }
      return result;      
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    size_type find(const StringViewLike& t, size_type pos = 0 ) const // TODO noexcept(std::is_nothrow_convertible_v<const StringViewLike&, std::basic_string_view<CharT, Traits>>)
    {
      std::basic_string_view<CharT,Traits> sv(t);
      return find(sv.c_str(), pos, sv.length());
    }

    // rfind ----------------------
    template<std::size_t kSizeOther>
    constexpr size_type rfind(const CopyableClipString<kSizeOther>& str, size_type pos=npos) const noexcept
    {
      return rfind(str.c_str(), pos, str.length());
    }

    constexpr size_type rfind(const CharT* str, size_type pos, size_type count) const noexcept
    {
      size_type sz = size();
      if((str==nullptr) || (count==0))
      {
        return (sz!=0) ? std::min(pos,sz) : npos;
      }
      if(count > sz) return npos;
      size_type end = sz - count;
      pos = std::min(pos,end);
      do
      {
        if (traits_type::compare(data() + pos, str, count) == 0)
        {
          return pos;
        }
      } while (pos-- != 0);
      return npos;
    }

    constexpr size_type rfind(const CharT* str, size_type pos=npos) const noexcept
    {
      if(!str) return npos;
      return rfind(str, pos, traits_type::length(str));
    }

    size_type rfind(CharT ch, size_type pos = npos) const noexcept
    {
      const size_type sz = size();
      if (sz == 0) return npos;
      pos = std::min(pos, sz - 1);
      do
      {
          if (traits_type::eq(data()[pos], ch))
          {
            return pos;
          }
      }
      while (pos-- != 0);
      return npos;
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible<StringViewLike>::value>>
    size_type rfind(const StringViewLike& t, size_type pos = npos ) const // TODO noexcept(std::is_nothrow_convertible_v<const StringViewLike&, std::basic_string_view<CharT, Traits>>)
    {
      std::basic_string_view<CharT,Traits> sv(t);
      return rfind(sv.c_str(), pos, sv.length());
    }

    // ------------------
    // Operations 
    // ------------------ 


  public: // TODO make protected
    // Implementations ------------------
    UnsignedCharT flags() const noexcept 
    {
      UnsignedCharT flags; 
      std::memcpy(&flags, &m_data[kSize], sizeof(UnsignedCharT));
      return flags;
    }

    void set_flags(UnsignedCharT f) noexcept
    {
      std::memcpy(&m_data[kSize], &f, sizeof(UnsignedCharT));
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
          size_type result;
          std::memcpy(&result, reinterpret_cast<const unsigned char*>(&m_data[kSize])-sizeof(std::size_t), sizeof(std::size_t));
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
    std::array<CharT, kSize+1> m_data;
    // kSize CharT character slots + one UnsignedCharT metadata slot.
    // When no flags are set and slack == 0, the metadata slot is zero
    // and therefore serves as the null terminator for a kSize-character string.
    // When flags are set, the maximum null-terminated string length is kSize-1.
};

#endif // ClipString.hpp