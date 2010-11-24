/*
 * File:   Inotify.h
 * Author: reyoung
 *
 * Created on 2010年11月20日, 下午7:26
 */

#ifndef _INOTIFY_H
#define	_INOTIFY_H

#include "Config..hxx"
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <string>
#include <unistd.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include "minIni.h"
class INotifyDataStruct;/**< INotify Data Struct  decleration*/

class INotify : public boost::noncopyable /**< INotify cannot be copy. */
{
public:

    /** \brief
     *  INotify Constructor
     * \param mask=IN_ALL_EVENTS uint32_t
     *  The Event Mask Default IN_ALL_EVENTS
     */
    INotify(uint32_t mask = IN_ALL_EVENTS);

    /** \brief
     *  Return the system is support inotify or not.
     * \return bool
     *
     */
    bool isSupported()const;

    /** \brief
     *  Add Watch Dir or file
     * \param filename const std::string&
     *          Watch File Name
     * \param recurse=false bool
     *          is recurse?
     * \return bool
     *          return true if success
     */
    bool addWatch(const std::string& filename, bool recurse = false);

    /** \brief
     *  remove watch dir or file
     * \param filename const std::string&
     * \return bool
     *  return true if success.
     */
    bool removeWatch(const std::string& filename);

    /** \brief
     *  deconstructor
     * \return
     *
     */
    virtual ~INotify();

    /** \brief
     *  Add Event Call Back.
     *      The Call back  which when event happend called.
     * \param f boost::function<void(const inotify_event&) >
     *    The Call Back Function Address.
     *      The call back function must return void
     *      and has one const inotify_event& argument
     *    For example.
     *      void ExampleCallBack(const inotify_event& e);
     * \return void
     *
     */
    void addEventCallBack(boost::function<void(const inotify_event&) > f);

    /** \brief
     *  Add Default Call Back
     *      The Default Call Back just has simple output method.
     *      Set For Debug.
     * \return void
     *
     */
    void addDefaultCallBack();


    /** \brief
     *  Step function.
     *      Set iteration times, each time will get current event,
     *      and filter file base names, then filter all filter callbacks,
     *      at last will call all event callbacks.
     * \param s int
     *  if s >= 0:
     *      s stand for iteration times.
     *  elif s == -1:
     *      iteration will forever.
     *  else
     *      throw a error.
     * \return void
     *
     */
    void step(int s)const;


    /** \brief
     *  Use wd get file name.
     *  wd stand for watch description
     * \param wd uint32_t
     * \return std::string
     *
     */
    std::string getFileName(uint32_t wd);
    /** \brief
     *  Get Operation String from mask
     *      this operation will convert mask to string.
     * \param mask uint32_t
     *  Mask value
     * \param isShort=true bool
     *  return short string or not, default is true(short).
     * \return std::string
     *
     */
    static std::string getOperationString(uint32_t mask, bool isShort = true);
    /** \brief
     *  Add Base Name Filter
     * \param basename const std::string&
     *  Set Base Name Filters
     * \param match=true bool
     *  Set Filter Need Match Or Not.
     * \return void
     *
     */
    void addBaseNameFilter(const std::string& basename, bool match = true);

    /** \brief
     *  Set Filter callback
     * \param f boost::function<bool(const inotify_event&) >
     *  Filter Callback Function.
     *      The Function Must return bool and has a const inotify_event& arg.
     *      If the function return true, the event will allow to process.
     *      Otherwise the event will forbidened to process.
     * \return void
     *
     */
    void addEventFilter(boost::function<bool(const inotify_event&) > f);

    /** \brief
     *  Save Config Method
     * \return void
     *
     */
    void saveConfig(const std::string& ConfigFileName =DefaultConfigFileName );

    /** \brief
     *  Load Config Method
     * \return void
     *
     */
    void loadConfig(const std::string& ConfigFileName =DefaultConfigFileName);

#ifdef __DEBUG__
    /** \brief
     *  Print One Event.
     *  Only for debug. Just like step(1);
     * \return void
     *
     */
    void printOneEvent();
#endif
private:

    /** \brief
     *  Defult call back method
     *      Just Print Something to test this class run correctly.
     * \param e const inotify_event&
     * \return void
     *
     */
    static void DefaultCallBack(const inotify_event& e);

    /** \brief
     *  Call All Callback
     *  In this function, the all the call backs will called
     *  by same arg.
     * \param inotify_event& const
     * \return void
     *
     */
    void call_allCallBack(const inotify_event&)const;

    /** \brief
     *  Inner addwatch
     *      Assist Function for addWatch();
     * \param filename const std::string&
     * \param recurse bool
     * \return bool
     *
     */
    bool in_addWatch(const std::string& filename, bool recurse);

    /** \brief
     *  This Class Cannot be copied.
     * \param orig const INotify&
     *
     */
    INotify(const INotify& orig);
    INotifyDataStruct* mdata;
};

#endif	/* _INOTIFY_H */

