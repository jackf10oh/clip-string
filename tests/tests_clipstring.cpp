// tests_clipstring.cpp 
// 
// gtests for ClipString.hpp header only class
// 
// JAF 5/18/2026 

#include<clip_string/ClipString.hpp>
#include<gtest/gtest.h>
#include<gmock/gmock.h>

template<std::size_t kSize_, typename CharT_ = char>
struct ParamPack
{
  static constexpr std::size_t kSize = kSize_;
  using CharT = CharT_;
  static constexpr CharT kCharacter = 'A';
};

using ClipStringParams = ::testing::Types<
  ParamPack<1>,
  ParamPack<2>,
  ParamPack<5>,
  ParamPack<9>,
  ParamPack<13>,
  ParamPack<19>,
  ParamPack<52>,
  ParamPack<64>,
  ParamPack<128>,
  ParamPack<204>,
  ParamPack<407>,
  ParamPack<512>,
  ParamPack<1028>,
  ParamPack<1,char16_t>,
  ParamPack<13,char16_t>,
  ParamPack<52,char16_t>,
  ParamPack<204,char16_t>,
  ParamPack<1028,char16_t>,
  ParamPack<1,char32_t>,
  ParamPack<13,char32_t>,
  ParamPack<52,char32_t>,
  ParamPack<204,char32_t>,
  ParamPack<1028,char32_t>
>;

template<typename T>
class ConstructorSuite : public ::testing::Test
{};

TYPED_TEST_SUITE(ConstructorSuite, ClipStringParams);

// Constructor Suite ------------------------------------------------- 
TYPED_TEST(ConstructorSuite, Default){
  constexpr std::size_t kSize = TypeParam::kSize;
  using CharT = typename TypeParam::CharT;
  ClipString<kSize,CharT> clip_str;
  ASSERT_EQ(clip_str.size(), 0);
  ASSERT_EQ(clip_str.length(), 0);
  ASSERT_FALSE(clip_str.clipped());
  ASSERT_FALSE(clip_str.null_passed());
  ASSERT_TRUE(clip_str.empty());
}

TYPED_TEST(ConstructorSuite, SizeChar){
  constexpr std::size_t kSize = TypeParam::kSize;
  using CharT = typename TypeParam::CharT;

  ClipString<kSize,CharT> str_0(0,TypeParam::kCharacter);
  ASSERT_EQ(str_0.size(), 0);
  ASSERT_EQ(str_0.length(), 0);
  ASSERT_FALSE(str_0.clipped());
  ASSERT_FALSE(str_0.null_passed());
  ASSERT_TRUE(str_0.empty());

  constexpr std::size_t kSize2 = std::max(std::size_t{1},kSize/2);
  ClipString<kSize,CharT> str_1(kSize2,TypeParam::kCharacter);
  ASSERT_EQ(str_1.size(), kSize2);
  ASSERT_EQ(str_1.length(), kSize2);
  ASSERT_FALSE(str_1.clipped());
  ASSERT_FALSE(str_1.null_passed());
  ASSERT_FALSE(str_1.empty());

  constexpr std::size_t kSize3 = kSize + 1;
  ClipString<kSize,CharT> str_2(kSize3,TypeParam::kCharacter);
  ASSERT_EQ(str_2.size(), kSize-1);
  ASSERT_EQ(str_2.length(), kSize-1);
  ASSERT_TRUE(str_2.clipped());
  ASSERT_FALSE(str_2.null_passed());
  ASSERT_EQ(!str_2.empty(), (kSize>1));
}

TYPED_TEST(ConstructorSuite, InputIterators){
  constexpr std::size_t kSize = TypeParam::kSize;
  constexpr std::size_t kSize2 = std::max(std::size_t{1},kSize/2);
  constexpr std::size_t kSize3 = kSize + 1;
  using CharT = typename TypeParam::CharT;
  std::vector<CharT> char_vec(kSize3, TypeParam::kCharacter);

  ClipString<kSize,CharT> str_0(char_vec.begin(), char_vec.begin());
  ASSERT_EQ(str_0.size(), 0);
  ASSERT_EQ(str_0.length(), 0);
  ASSERT_FALSE(str_0.clipped());
  ASSERT_FALSE(str_0.null_passed());
  ASSERT_TRUE(str_0.empty());

  ClipString<kSize,CharT> str_1(char_vec.begin(),char_vec.begin()+kSize2);
  ASSERT_EQ(str_1.size(), kSize2);
  ASSERT_EQ(str_1.length(), kSize2);
  ASSERT_FALSE(str_1.clipped());
  ASSERT_FALSE(str_1.null_passed());
  ASSERT_FALSE(str_1.empty());

  ClipString<kSize,CharT> str_2(char_vec.begin(),char_vec.end());
  ASSERT_EQ(str_2.size(), kSize-1);
  ASSERT_EQ(str_2.length(), kSize-1);
  ASSERT_TRUE(str_2.clipped());
  ASSERT_FALSE(str_2.null_passed());
  ASSERT_EQ(!str_2.empty(), (kSize>1));
}

TYPED_TEST(ConstructorSuite, CharStar){
  constexpr std::size_t kSize = TypeParam::kSize;
  constexpr std::size_t kSize2 = std::max(std::size_t{1},kSize/2);
  constexpr std::size_t kSize3 = kSize + 1;
  using CharT = typename TypeParam::CharT;
  std::basic_string<CharT> str{};

  str.resize(0,TypeParam::kCharacter);
  ClipString<kSize,CharT> str_0(str.c_str());
  ASSERT_EQ(str_0.size(), 0);
  ASSERT_EQ(str_0.length(), 0);
  ASSERT_FALSE(str_0.clipped());
  ASSERT_FALSE(str_0.null_passed());
  ASSERT_TRUE(str_0.empty());

  str.resize(kSize2,TypeParam::kCharacter);
  ClipString<kSize,CharT> str_1(str.c_str());
  ASSERT_EQ(str_1.size(), kSize2);
  ASSERT_EQ(str_1.length(), kSize2);
  ASSERT_FALSE(str_1.clipped());
  ASSERT_FALSE(str_1.null_passed());
  ASSERT_FALSE(str_1.empty());

  str.resize(kSize3,TypeParam::kCharacter);
  ClipString<kSize,CharT> str_2(str.c_str());
  ASSERT_EQ(str_2.size(), kSize-1);
  ASSERT_EQ(str_2.length(), kSize-1);
  ASSERT_TRUE(str_2.clipped());
  ASSERT_FALSE(str_2.null_passed());
  ASSERT_EQ(!str_2.empty(), (kSize>1));

  ClipString<kSize,CharT> str_3((CharT*)nullptr);
  ASSERT_TRUE(str_3.null_passed);
}

template<typename T>
class STLContainerSuite : public ::testing::Test
{};

TYPED_TEST_SUITE(STLContainerSuite, ClipStringParams);

TYPED_TEST(STLContainerSuite, Iterators){
  constexpr std::size_t kSize = TypeParam::kSize;
  using CharT = typename TypeParam::CharT;
  for(const auto& i : {0,1,3,6,40,80,150})
  {
    ClipString<kSize,CharT> str(i,TypeParam::kCharacter);
    if(str.clipped())
    {
      ASSERT_EQ(kSize-1, std::distance(str.begin(),str.end()));
      ASSERT_EQ(kSize-1, std::distance(str.cbegin(),str.cend()));
      ASSERT_EQ(kSize-1, std::distance(str.rbegin(),str.rend()));
      ASSERT_EQ(kSize-1, std::distance(str.crbegin(),str.crend()));
    }
    else
    {
      ASSERT_EQ(i, std::distance(str.begin(),str.end()));
      ASSERT_EQ(i, std::distance(str.cbegin(),str.cend()));
      ASSERT_EQ(i, std::distance(str.rbegin(),str.rend()));
      ASSERT_EQ(i, std::distance(str.crbegin(),str.crend()));
    } 
  }
}

TYPED_TEST(STLContainerSuite, Capacity){
  constexpr std::size_t kSize = TypeParam::kSize;
  using CharT = typename TypeParam::CharT;
  std::array<std::size_t,7> arr = {1,3,6,40,80,150, kSize+1}; 
  for(const auto& i : arr)
  {
    ClipString<kSize,CharT> str(i,TypeParam::kCharacter);
    if(kSize>1)
    {
      ASSERT_FALSE(str.empty());
    }
    if(str.clipped())
    {
      ASSERT_EQ(str.size(), kSize-1);
      ASSERT_EQ(str.capacity(),kSize-1);
    }
    else
    {
      ASSERT_EQ(str.size(), i);
      ASSERT_EQ(str.capacity(),kSize);
    }
    str.clear();
    ASSERT_TRUE(str.empty());
    ASSERT_EQ(str.size(),0);
  }
  constexpr std::size_t m = ClipString<kSize,CharT>::max_size(); 
  ASSERT_EQ(m, kSize);
}

TYPED_TEST(STLContainerSuite, PushBack){
  constexpr std::size_t kSize = TypeParam::kSize;
  using CharT = typename TypeParam::CharT;

  ClipString<kSize,CharT> str_01; 
  str_01.push_back(TypeParam::kCharacter);
  ASSERT_EQ(str_01[0], TypeParam::kCharacter);

  ClipString<kSize,CharT> str_02(kSize,TypeParam::kCharacter);
  ASSERT_FALSE(str_02.clipped());
  str_02.push_back('B');
  ASSERT_TRUE(str_02.clipped());

  constexpr std::size_t kSize2 = std::max(std::size_t{1},kSize/2);
  ClipString<kSize,CharT> str_03(kSize2,'B'); 
  str_03.push_back(TypeParam::kCharacter);
  if(!str_03.clipped()) ASSERT_EQ(str_03[kSize2], TypeParam::kCharacter);
}

TYPED_TEST(STLContainerSuite, PopBack){
  constexpr std::size_t kSize = TypeParam::kSize;
  using CharT = typename TypeParam::CharT;

  ClipString<kSize,CharT> str_01(1,TypeParam::kCharacter);
  ASSERT_FALSE(str_01.empty());
  str_01.pop_back(); 
  ASSERT_TRUE(str_01.empty());

  ClipString<kSize,CharT> str_02(kSize,TypeParam::kCharacter);
  ASSERT_EQ(str_02.length(), kSize);
  str_02.pop_back(); 
  ASSERT_EQ(str_02.length(), kSize-1);
  std::size_t len = ClipString<kSize,CharT>::traits_type::length(str_02.c_str());
  ASSERT_EQ(str_02.length(), len);
}

