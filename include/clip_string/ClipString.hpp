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
    struct is_sv_convertible : std::conjunction<std::is_convertible<const T&, const CharT*>, std::negation<std::is_convertible_v<const T&, const CharT*>>>{};
    
    template<typename T>
    using is_sv_convertible_v = is_sv_convertible::value;

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

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
    constexpr ClipString(const StringViewLike& t) noexcept // C++17 only
    {
      assign<StringViewLike>(t);
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
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
        set_flags(Flags::Clipped);
        set_slack(1);
        traits_type::assign(data()[kSize-1],CharT{});
      }
      else
      {
        set_flags(0x00);
        set_slack(kSize-count);
        traits_type::assign(data[kSize-count],CharT{});
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

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
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

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
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

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
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
    constexpr pointer data() noexcept { return reinterpret_cast<CharT*>(m_data.data()); }
    constexpr const_pointer data() const noexcept { return reinterpret_cast<const CharT*>(m_data.data()); }
    constexpr const_pointer c_str() const noexcept { return reinterpret_cast<const CharT*>(m_data.data()); }
    constexpr operator std::basic_string_view<CharT,Traits>() const noexcept { return {data(), size()}; } // C++17 only

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

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
    constexpr ClipString& insert( size_type index, const StringViewLike& t )
    {
      std::basic_string_view<CharT, Traits> sv = t;
      return insert(index, sv.c_str(), sv.lengt());
    }

    template<class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
    constexpr basic_string& insert(size_type index, const StringViewLike& t, size_type t_index, size_type count = npos)
    {
      std::basic_string_view<CharT,Traits> sv(t);
      if(t_index > sv.length()) throw std::out_of_range{sv.c_str()};
      return insert(index, sv.c_str() + t_index, std::min(count, sv.length()-t_index));
    }

    // erase --------------

    // push_back --------------
    void push_back(CharT ch) noexcept
    {
      UnsignedCharT f = flags();
      UnsignedCharT s = f & Flags::SlackMask;
      // considered a clip if any flag is set, and at size kSize-1. or if at size kSize.
      // maybe useful to silently toggle NullptrPass? 
      bool possible = ((f & Flags::FlagsMask)!=0) ? (s>1) : (s!=0);
      if(possible)
      {
        traits_type::assign(data()[kSize-s-1], ch);
        traits_type::assign(data()[kSize-s], CharT{});
        set_slack(s-1);
      }
      else
      {
        traits_type::assign(data()[kSize-1], CharT{}); // clip last character.
        set_flags((f & Flags::FlagsMask) | Flags::Clipped | UnsignedCharT{1}); // set clipped as true
      }
    }

    // pop_back --------------
    constexpr void pop_back() noexcept
    {
      // UB if empty() == true
      std::size_t s = slack();
      traits_type::assign(data()[kSize - s],CharT{});
      set_slack(s - 1);
    }

    // --------------
    // append
    // --------------
    constexpr ClipString& append(size_type count, CharT ch) noexcept { return insert(size(), count, ch); } 
    constexpr ClipString& append(const CharT* s, size_type count) noexcept { return insert(size(), s, count); } 
    constexpr ClipString& append(const CharT* s) noexcept { return insert(size(), s); } 
    template< class SV, typename = std::enable_if_t<is_sv_convertible_v<SV>>>
    constexpr ClipString& append( const SV& t ) noexcept { return insert<SV>(size(), t); }
    template< class SV, typename = std::enable_if_t<is_sv_convertible_v<SV>>>
    constexpr ClipString& append(const SV& t, size_type pos, size_type count = npos){ return insert(size(), t, pos, count); }
    template<std::size_t kSizeOther>    
    constexpr ClipString& append(const CopyableClipString<kSizeOther>& str) noexcept { return insert(size(), str); } 
    template<std::size_t kSizeOther>    
    constexpr ClipString& append(const CopyableClipString<kSizeOther>& str, size_type pos, size_type count = npos ){ return insert(size(), str, pos, count); }
    template<class InputIt>
    constexpr ClipString& append(InputIt first, InputIt last) noexcept { return insert<InputIt>(size(), first, last); }
    constexpr ClipString& append( std::initializer_list<CharT> ilist ) noexcept { return insert(size(), ilist); }
    
    // --------------
    // opertator+=
    // --------------
    template<std::size_t kSizeOther>
    constexpr ClipString& operator+=(const CopyableClipString<kSizeOther>& str) noexcept { return append<kSizeOther>(std); }
    constexpr ClipString& operator+=(CharT ch) noexcept { push_back(ch); return *this; }
    constexpr ClipString& operator+=( const CharT* s ) noexcept { return append(s); }
    constexpr ClipString& operator+=(std::initializer_list<CharT> ilist) noexcept { return append(ilist); }
    template< class StringViewLike, typename = std::enable_if_t<is_sv_convertible_v<StringViewLike>>>
    ClipString& operator+=( const StringViewLike& t ) noexcept { return append<StringViewLike>(t); }

    // --------------
    // replace
    // --------------

    // --------------
    // copy
    // --------------

    // --------------
    // resize
    // --------------
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