/* 
 * File:   Inotify.cpp
 * Author: reyoung
 * 
 * Created on 2010年11月20日, 下午7:26
 */

#include "Inotify.h"
#include <map>
#include <string>
#include <sys/inotify.h>
#include <iostream>
#include <cstdio>
#include <list>
#include <boost/function.hpp>
#include <error.h>
#include <errno.h>
#include <cstring>
#include <stdlib.h>
#include <fstream>
#include "FileInfo.h"
#include <boost/lexical_cast.hpp>



extern int errno;
using namespace std;

//For the stupid CPP template expression.
struct CallBackType
{
    boost::function<void(const inotify_event&)> func;
};

struct FilterType
{
    boost::function<bool(const inotify_event&)> func;
};


struct INotifyDataStruct
{
    list<CallBackType > callbacks;
    map<string,int> name_map;
    int file_description;
    uint32_t mask;
    list<pair<string,bool> > basename_filters;
    list<FilterType> filter_functions;
    list<pair<string,bool> > watch_files;
};
struct ConfigDataStruct
{
    uint32_t mask;
    list<pair<string,bool> > basename_filters;
    list<string> names;
    void setValue(const INotifyDataStruct& other)
    {
        mask = other.mask;
        basename_filters = other.basename_filters;
        for(map<string,int>::const_iterator it = other.name_map.begin();
                it!=other.name_map.end();++it)
        {
            names.push_back(it->first);
        }
    }
};


INotify::INotify(uint32_t mask):mdata(new INotifyDataStruct) {
    this->mdata->file_description = inotify_init1(IN_CLOEXEC);
    this->mdata->mask = mask;
}

INotify::~INotify() {
    delete this->mdata;
}

bool INotify::isSupported() const
{
    return this->mdata->file_description != -1;
}

bool INotify::addWatch(const std::string& filename, bool recurse)
{
    this->mdata->watch_files.push_back(pair<string,bool> (filename,recurse));
    return this->in_addWatch(filename,recurse);
}

bool INotify::in_addWatch(const std::string& filename,bool recurse)
{
    if(this->isSupported())
    {
        #ifdef __DEBUG__
//        cout<<"Adding "<<filename<<endl;
        #endif
        int wd = inotify_add_watch(this->mdata->file_description,filename.c_str(),this->mdata->mask);
        if(wd < 0)
            return false;
        else
        {
            pair<string,int> p(filename,wd);
            this->mdata->name_map.insert(p);
            #ifdef __DEBUG__
//            cout<<"After Add Watch, Name_Map Size = "<<this->mdata->name_map.size()<<endl;
            #endif
            if(!recurse)
                return true;
        }
    }
    else
    {
        return false;
    }

    if(recurse == true)
    {
           struct dirent **namelist;
           int n;
           n = scandir(filename.c_str(), &namelist, 0, alphasort);
           if (n < 0)
               return false;
           else {
               while (n--) {
                   if(strcmp(namelist[n]->d_name,".")!=0&&strcmp(namelist[n]->d_name,"..")!=0)
                   {
                        in_addWatch(filename+"/"+namelist[n]->d_name,true);
                   }
                   free(namelist[n]);
               }
               free(namelist);
           }
    }
    return true;
}

void INotify::addEventCallBack(boost::function<void(const inotify_event&)> f)
{
    CallBackType c;
    c.func = f;
    this->mdata->callbacks.push_back(c);
}
void INotify::addDefaultCallBack()
{
    this->addEventCallBack(DefaultCallBack);
}
void INotify::DefaultCallBack(const inotify_event& e)
{
   switch (e.mask)
   {
      /* File was accessed */
      case IN_ACCESS:
         printf("ACCESS EVENT OCCURRED\n");
      break;
      /* File was modified */
      case IN_MODIFY:
         printf("MODIFY EVENT OCCURRED \n");
      break;
      /* File changed attributes */
      case IN_ATTRIB:
         printf("ATTRIB EVENT OCCURRED: \n");
      break;
      /* File was closed */
      case IN_CLOSE:
         printf("CLOSE EVENT OCCURRED:  \n ");
      break;
      /* File was opened */
      case IN_OPEN:
         printf("OPEN EVENT OCCURRED:\n");
      break;
      /* File was moved from X */
      case IN_MOVED_FROM:
         printf("MOVE_FROM EVENT OCCURRED\n");
      break;
      /* File was moved to X */
      case IN_MOVED_TO:
         printf("MOVE_TO EVENT OCCURRED:\n");
      break;
      default:
         printf ("UNKNOWN EVENT OCCURRED \n");
      break;
   }
}
void INotify::step(int s)const
{
    static const int buffersize = 1024;
    static char buffer[buffersize];
    memset(buffer,0,buffersize);
    int readsize = 0;
    bool isForever = (s == -1);
    while((readsize = read(this->mdata->file_description,buffer,buffersize))!= -1 &&
            (isForever||s--))
    {
        char * offset = buffer;
        inotify_event* event = (inotify_event* )offset;
        while(((char *)event - buffer) < readsize)
        {
            this->call_allCallBack(*event);
            offset += event->len+sizeof(inotify_event);
            event=  (inotify_event* )offset;
        }

    }
}
bool INotify::removeWatch(const std::string& filename)
{
     map<string,int>::iterator it =  this->mdata->name_map.find(filename);
     if(it == this->mdata->name_map.end())
         return false;
     else
     {
         if(inotify_rm_watch(this->mdata->file_description,it->second) == -1)
             return false;
         this->mdata->name_map.erase(it);
         return true;
     }
}
void INotify::call_allCallBack(const inotify_event& e)const
{
    for(list<CallBackType>::iterator it = this->mdata->callbacks.begin();
                it!= this->mdata->callbacks.end();++it)
    {
        bool flag =false;
        FileInfo file(e.name);
        //Base Name Event Filter
        if(this->mdata->basename_filters.size()!=0)
        {
            
            for(list<pair<string,bool> >::iterator it = this->mdata->basename_filters.begin();
            it!= this->mdata->basename_filters.end();++it)
            {
                if(file.isBaseNameMatch(it->first)==it->second)
                {
                    flag = false;
                    break;
                }
                else
                {
                    flag = true;
                }
            }
        }
        //Base Name Event Filter end;
        //Option Filter Function Begin
        for(list<FilterType>::iterator itt = this->mdata->filter_functions.begin();
                itt!=this->mdata->filter_functions.end();++itt)
        {
            if(!itt->func(e))
            {
                flag = true;
                break;
            }
        }
        //Option Filter Function End
        if(!flag)
        {
            it->func(e);
        }
    }
}
string INotify::getFileName(uint32_t wd)
{
    for( map<string,int>::iterator it= this->mdata->name_map.begin();
            it!= this->mdata->name_map.end();++it)
    {
        if(it->second == wd)return it->first;
    }
#ifdef Throwable
    throw "The File is Not Exist";
#endif
    return "";
}

string INotify::getOperationString(uint32_t mask,bool isShort)
{
    string retstr;
    if(mask&IN_ACCESS)
        retstr+=isShort?"Access":"File was accessed(read) ";
    if(mask&IN_ATTRIB)
        retstr+=isShort?"Attrib":"File's Metadata changed ";
    if(mask&IN_CLOSE_WRITE)
        retstr+=isShort?"Close And Write":"File was closed and written ";
    if(mask&IN_CLOSE_NOWRITE)
        retstr+=isShort?"Close No Write":"File was closed and no write";
    if(mask&IN_CREATE)
        retstr+= isShort?"Create":"File/dirctory created in watched directory";
    if(mask& IN_DELETE)
        retstr+= isShort?"Delete":"File/dirctory deleted from watched directory";
    if(mask&IN_DELETE_SELF)
        retstr+=isShort?"DeleteSelf":"Watched file deleted";
    if(mask&IN_MODIFY)
        retstr+=isShort?"Modify":"File was modified";
    if(mask&IN_MOVE_SELF)
        retstr+=isShort?"MoveSelf":"Watched File was moved";
    if(mask&IN_MOVED_FROM)
        retstr+=isShort?"MoveFrom":"File move out of watch directory";
    if(mask&IN_MOVED_TO)
        retstr+=isShort?"MoveTo":"File move in watch directory";
    if(mask&IN_OPEN)
        retstr+=isShort?"Open":"File was opened";
    return retstr;
}
void INotify::addBaseNameFilter(const std::string& basename, bool match)
{
    this->mdata->basename_filters.push_back(pair<string,bool>(basename,match) );
}
void INotify::addEventFilter(boost::function<bool(const inotify_event&)> f)
{
    FilterType ft;ft.func = f;
    this->mdata->filter_functions.push_back(ft);
}

void INotify::saveConfig(const std::string& ConfigFileName)
{
//    ConfigDataStruct config;config.setValue(*this->mdata);
    string rmcommand = "rm -f ";
    system((rmcommand+ConfigFileName).c_str());
    minIni iniparser(ConfigFileName);
    iniparser.put("Mask","Mask",(int)this->mdata->mask);
    int i=0;
    for(list<pair<string,bool> >::const_iterator it = this->mdata->watch_files.begin();
            it!=this->mdata->watch_files.end();++it,++i)
    {
        iniparser.put("Watch Files",string("Name")+boost::lexical_cast<string>(i),it->first);
        iniparser.put("Watch Files",string("Recurse")+boost::lexical_cast<string>(i)
        ,it->second);
    }

    i = 0;
    for( list<pair<string,bool> >::const_iterator it = this->mdata->basename_filters.begin();
            it!=this->mdata->basename_filters.end();++it,++i)
    {
        iniparser.put("Name Filter",string("Name")+boost::lexical_cast<string>(i),it->first);
        iniparser.put("Name Filter",string("Bool")+boost::lexical_cast<string>(i),(int)it->second);
    }
}

void INotify::loadConfig(const std::string& ConfigFileName)
{
    minIni ini(ConfigFileName);
    this->mdata->mask = ini.geti("Mask","Mask");
    for(int i=0;;++i)
    {
        string str = ini.gets("Watch Files",string("Name")+boost::lexical_cast<string>(i));
        if(str.size() == 0)
            break;
        this->addWatch(str,ini.geti("Watch Files",string("Recurse")+boost::lexical_cast<string>(i)));
    }

    for(int i=0;;++i)
    {
        string str = ini.gets("Name Filter",string("Name")+boost::lexical_cast<string>(i));
        if(str.size()==0)
            break;
        this->addBaseNameFilter(str,ini.geti("Name Filter",string("Bool")+boost::lexical_cast<string>(i)));
    }
}


#ifdef __DEBUG__
void INotify::printOneEvent()
{
    char event[1024];
    int size = read(this->mdata->file_description,event,1024);
    cout<<size<<endl;
    inotify_event* e;
    e = (inotify_event* )event;
    cout<<e->name<<endl;
}
#endif