#include <unistd.h>
#include "Inotify.h"
#include <iostream>
#include <syslog.h>
#include <string>
#include "rootpermission.h"
#include <cstdio>
#include "FileInfo.h"
#include <signal.h>
#include <list>
#include "Sigleton.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
using namespace std;

/** \brief
 *  Event Handler
 *      Wrtie Event into syslog,user.
 * \param event const inotify_event&
 * \return void
 *
 */
void EventHandler(const inotify_event& event)
{
    INotify& ino = INotify_Monitor;
    string watchfn;
    try
    {
        watchfn = ino.getFileName(event.wd);
    }
    catch(const char* errstr)
    {
        cerr<<errstr<<endl;
    }
    try
    {
    syslog(LOG_USER,"In %s : %s Changed, Operation = %s",watchfn.c_str(),event.name,INotify::getOperationString(event.mask,true).c_str());
    }
    catch(const char* str)
    {
        cerr<<str<<endl;
    }
}

/** \brief
 *  Signal Handler
 *      Before exit, Save Config
 * \param sig int
 * \return void
 *
 */
void SignalHandler(int sig)
{
    INotify& ino = INotify_Monitor;
    ino.saveConfig();
    syslog(LOG_USER,"Program Quit!");
    SafeExit(0);
}
/** \brief
 *  Print Usage.
 * \return void
 *
 */
void PrintUsage()
{
    cout<<"Usage:\n";
    cout<<"Options: -D<Directory(Recurse)> -d<Directory> -f<file filter(match)> -n<file filter(unmatch)> -h:help"<<endl;
    cout<<"Example:"<<endl;
    cout<<"  fsmon -D'/home' -f'txt' "<<endl;
    cout<<"This option will monitor the txt files changed in the /home directory and its subdirectory"<<endl;
    SafeExit(1);
}

/** \brief
 *  Process Commad option, and set config
 * \param argc int
 * \param argv char**
 * \return void
 *
 */
void CatchCMDOption(int argc,char** argv)
{
    list<string> Dlist;
    list<string> dlist;
    list<string> flist,Flist;
    int opt;
    opterr = 0;
    while((opt = getopt(argc,argv,"D:d:f:n:h"))!= -1)
    {
        switch(opt)
        {
            case 'D':
                Dlist.push_back(optarg);
                break;
            case 'd':
                dlist.push_back(optarg);
                break;
            case 'f':
                flist.push_back(optarg);
                break;
            case 'n':
                Flist.push_back(optarg);
                break;
            case 'h':
            default:
                PrintUsage();
                break;
        }
    }
    INotify& ino = INotify_Monitor;
    if ((Dlist.size()==0&&dlist.size() == 0&&flist.size() == 0&&Flist.size()==0))
    {
        ino.loadConfig();
    }
    else
    {
        for(list<string>::const_iterator it = Dlist.begin();it!=Dlist.end();++it)
        {
            ino.addWatch(*it,true);
        }
        for(list<string>::const_iterator it = dlist.begin();it!=dlist.end();++it)
        {
            ino.addWatch(*it,false);
        }
        for(list<string>::const_iterator it = flist.begin();it!=flist.end();++it)
        {
            ino.addBaseNameFilter(*it,true);
        }
        for(list<string>::const_iterator it = Flist.begin();it!=Flist.end();++it)
        {
            ino.addBaseNameFilter(*it,false);
        }
    }
}

/** \brief
 *  Assert There is only one instance running,
 * \return void
 *
 */
void AssertOneInstance()
{
    if(access("/tmp/fsmon_mutex",F_OK) == 0)
    {
        cerr<<"Another instance already run!, exit!"<<endl;
        exit(1);
    }
}
int main(int argc,char** argv)
{

    AssertOneInstance();

    system("touch /tmp/fsmon_mutex");
    INotify& ino = INotify_Monitor;
    if(!ino.isSupported())
    {
        cerr<<"Your System Is Not Supported Inotify"<<endl;
    }
    AssertRootPermission();

    if(SIG_ERR == signal(15,SignalHandler))
        cerr<<"Signal Catch Error";
    signal(SIGQUIT,SignalHandler);
    signal(SIGINT,SignalHandler);


    openlog("fsmon",LOG_NDELAY,LOG_USER);
    CatchCMDOption(argc,argv);
    ino.addEventCallBack(EventHandler);
    pid_t p;
    p = fork();
    if(p!=0){exit(0);}
    setsid();
    if((p=fork())!=0){exit(0);}
    else if(p<0){exit(1);}
    umask(0);
    ofstream fout;
    fout.open("/tmp/fsmon_mutex");
    fout<<getpid();
    fout.close();
    ino.step(-1);
    ino.saveConfig();
    closelog();
    return 0;
}
