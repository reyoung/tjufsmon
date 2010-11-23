/* 
 * File:   FileInfo.cpp
 * Author: reyoung
 * 
 * Created on 2010年11月21日, 下午3:34
 */

#include "FileInfo.h"
#include <string>
#include <iostream>
#include <cstring>
using namespace std;
FileInfo::FileInfo(const std::string& fn):m_filename(fn) {
}

FileInfo::FileInfo(const FileInfo& orig) {
    this->m_filename = orig.m_filename;
}

FileInfo::~FileInfo() {
}

const string FileInfo::getBaseName() const
{
    string str;
    const char* cstr = this->m_filename.c_str();
    size_t size = this->m_filename.size();
    for(int i = size-1;i>0;--i)
    {
        if(cstr[i]=='.')break;
        str.insert(str.begin(),1,cstr[i]);
    }
    return str;
}
bool FileInfo::isBaseNameMatch(const std::string& fn)const
{
    return strcmp(this->getBaseName().c_str(),fn.c_str())==0;
}