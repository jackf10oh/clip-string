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
  ClipString<100> str{};
  while(getline(std::cin,str))
  {
    std::cout << str;
    if(str.clipped())
    {
      std::cout << " -- clipped!" << std::endl;
    }
    else
    {
      std::cout << std::endl;
    }
  }
}