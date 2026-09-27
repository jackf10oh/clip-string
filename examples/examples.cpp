// test.cpp
//
// JAF 9/25/2026

#include<string>
#include<iostream>
#include<clip_string/ClipString.hpp>

using std::cout;
using std::endl;

int main(int argc, char** argv)
{
  std::string test_str_01 = "foobar";
  cout << test_str_01 << '\n';

  auto debug_print = [](auto c)
  {
    cout << "empty? " << c.empty() << '\n';
    cout << "length: " << c.length() << '\n';
    cout << "slack: " << c.slack() << '\n';
    cout << "clipped: " << c.clipped() << '\n';
    cout << "str: " << c.c_str() << "\n";
  };

  ClipString<32> clip_01(50,'a'); 
  debug_print(clip_01);

  ClipString<1000> clip_02(30,'a');
  debug_print(clip_02);

  ClipString<64> clip_03(clip_01);
  debug_print(clip_03);

  clip_03.resize(40,'b');
  debug_print(clip_03);

  clip_03.resize(20,'b');
  debug_print(clip_03);

  clip_02 = clip_03; 
  debug_print(clip_02);
}