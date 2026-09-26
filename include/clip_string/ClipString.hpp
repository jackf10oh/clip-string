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
#include<string> // still need std::char_traits
#include<iostream> 

#ifndef _CLIPSTRING_CLIPSTRING_H
#define _CLIPSTRING_CLIPSTRING_H

using std::cout;

template<std::size_t kSize, typename CharT = char, typename TraitsT = std::char_traits<CharT>>
class ClipString
{
  static_assert(kSize > 0, "must pass size >= 1 to ClipString template parameters");
  private:
    // Friends ------------------- 
    template<std::size_t kSizeOther, typename CharU, typename TraitsU>
    friend class ClipString;

    // Type defs + Flags -----------------
    template<std::size_t kSizeOther>
    using CopyableClipString = ClipString<kSizeOther,CharT,TraitsT>; // easier than asserting std::is_same at every step

    using UnsignedInt = std::conditional_t< // CharT as a unsigned integer type
      (sizeof(CharT)==1),
      std::uint8_t,
      std::conditional_t<
        sizeof(CharT)==2,
        std::uint16_t,
        std::conditional_t<
          sizeof(CharT)==4,
          std::uint32_t,
          void
        >        
      >
    >;
    static_assert(!std::is_same_v<UnsignedInt,void>,"invalid CharT passed to ClipString<...> template parameters");

  public:
    using value_type = CharT;
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
      static constexpr UnsignedInt Clipped = static_cast<UnsignedInt>(1u << (8 * sizeof(CharT) -1));
      static constexpr UnsignedInt NullptrPassed = static_cast<UnsignedInt>(1u << (8 * sizeof(CharT) - 2));
      static constexpr UnsignedInt FlagsMask = static_cast<UnsignedInt>(Clipped | NullptrPassed);
      static constexpr UnsignedInt SlackMask = static_cast<UnsignedInt>(~FlagsMask);
      static constexpr UnsignedInt ExtraSlack = static_cast<UnsignedInt>(1u << (8 * sizeof(CharT) - 3));
      static constexpr UnsignedInt ShortSlackMask = static_cast<UnsignedInt>(~(FlagsMask | ExtraSlack));
    };

    // Constructors + destructor ---------------------

    // default
    ClipString()
      : m_flags(0x00)
    {
      m_data[0] = UnsignedInt(0x00);
      set_slack(kSize);
      cout << "constuctor<" << kSize << ">\n";
      cout << "typeid: " << typeid(UnsignedInt).name() << "\n";
      cout << "clipped: " << Flags::Clipped << "\n";
    }

    // destructor
    ~ClipString()=default;

    // Member Functions -----------------

    // Element access ------------------

    constexpr reference at(size_type i) noexcept
    {
      if(i >= size()) throw std::out_of_range{};
      return m_data[i];
    }
    constexpr const_reference at(size_type i) const noexcept
    {
      if(i >= size()) throw std::out_of_range{};
      return m_data[i];
    }
    constexpr reference operator[](size_type i) noexcept { return m_data[i]; }
    constexpr const_reference operator[](size_type i) const noexcept{ return m_data[i]; }
    constexpr reference front() noexcept { return m_data[0]; }
    constexpr const_reference front() const noexcept{ return m_data[0]; }
    constexpr reference back() noexcept { return m_data[size()]; }
    constexpr const_reference back() const noexcept{ return m_data[size()]; }
    constexpr pointer data() noexcept { return m_data.data(); }
    constexpr const_pointer data() const noexcept{ return m_data.data(); }
    constexpr const_pointer c_str() const noexcept{ return m_data.data(); }
    constexpr operator std::basic_string_view<CharT,TraitsT>() const noexcept { return {m_data.data(), size()}; }

    // Iterators -------------- 
    constexpr iterator begin() noexcept { return m_data.data(); }
    constexpr const_iterator begin() const noexcept { return m_data.data(); }
    constexpr const_iterator cbegin() const noexcept { return m_data.data(); }
    constexpr iterator end() noexcept { return m_data.data() + size(); }
    constexpr const_iterator end() const noexcept { return m_data.data() + size(); }
    constexpr const_iterator cend() const noexcept { return m_data.data() + size(); }
    constexpr reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(cend()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }
    constexpr reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(cbegin()); }
    constexpr const_reverse_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }

    // Capacity ---------------- 
    constexpr size_type empty() const noexcept { return (slack()==kSize); }
    constexpr size_type size() const noexcept { return kSize - slack(); }
    constexpr size_type length() const noexcept { return kSize - slack(); }
    static constexpr size_type max_size() noexcept { return kSize; }
    void reserve()=delete;
    size_type capacity() const noexcept { return ((m_flags & Flags::FlagsMask)!=0) ? kSize-1 : kSize; } // if flags are set. can't be null terminator
    size_type shrink_to_fit()=delete;

    // Modifiers ----------------- 
    constexpr void clear() noexcept
    { 
      m_data[0] = UnsignedInt(0x00); // null terminator at [0]
      m_flags = m_flags & Flags::SlackMask; // drop booleans 
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

    // --------------
    // swap
    // --------------

  public: // TODO make protected
    // Implementations ------------------
    size_type slack() const
    {
      if constexpr(sizeof(CharT)==1){
        if constexpr(kSize <= 63){ // 2^6
          return static_cast<size_type>(m_flags & Flags::SlackMask);
        }
        else if constexpr(kSize < 8191){ // 2^(8+5)
            if((m_flags & Flags::ExtraSlack)!=0){
              // this was actually the high byte
              std::uint16_t high_byte = m_flags & (Flags::SlackMask ^ Flags::ExtraSlack);
              std::uint16_t low_byte = static_cast<std::uint8_t>(m_data[kSize-1]);
              return ((high_byte << 8) | low_byte);
            }
            else{
              // wasn't the high byte
              return (m_flags & (Flags::SlackMask));
            }
        }
      }
      else if constexpr(sizeof(CharT)==2){
        // TODO
      }
      else if constexpr(sizeof(CharT)==4){
        // TODO
      }
      else{
        static_assert(false, "invalid CharT passed to ClipString template parameter");
      }
    }

    void set_slack(size_type slack)
    {
      if constexpr(sizeof(CharT)==1){
        assert((slack<16383) && "invalid slack passed to set_slack()");
        if constexpr(kSize <= 63){ // 2^6
          m_flags = (m_flags & Flags::FlagsMask) | static_cast<std::uint8_t>(slack);
        }
        else if constexpr(kSize < 8191){ // 2^(8+5)
          if(slack < 32){ // 2^5
            m_flags = (m_flags & Flags::FlagsMask) | static_cast<std::uint8_t>(slack); 
          }
          else{
            std::uint16_t low_pair = static_cast<std::uint16_t>(slack);
            m_data[kSize-1] = static_cast<std::uint8_t>(low_pair); // lower byte goes to 2nd to last byte
            m_flags = (m_flags & Flags::FlagsMask) | Flags::ExtraSlack | static_cast<std::uint8_t>(low_pair >> 8); // higher byte goes to last byte
          }
        }
      }
      else if constexpr(sizeof(CharT)==2){
        // TODO
      }
      else if constexpr(sizeof(CharT)==4){
        // TODO
      }
      else{
        static_assert(false, "invalid CharT passed to ClipString template parameter");
      }
    }

    // Member Data
    std::array<CharT,kSize> m_data; // N CharT array 
    UnsignedInt m_flags;            // 1 extra entry that has to work as null terminator
                                    // plus boolean flags. 
                                    // [ clip nullptr big_slack _ _ ... _ ]
                                    // where big slack is when slack won't fit into just 1 UnsignedInt
};

#endif // ClipString.hpp