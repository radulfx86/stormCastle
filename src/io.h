#ifndef _IO_H_
#define _IO_H_
#include <string>
#include <fstream>
#include <sstream>

std::string loadText(const std::string &path)
{
    std::ifstream str(path, std::ios::in);
    std::stringstream sstr;
    sstr << str.rdbuf();
    return sstr.str();
}
#endif // _IO_H_