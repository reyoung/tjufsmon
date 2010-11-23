/*
 * File:   Config..hxx
 * Author: reyoung
 *
 * Created on 2010年11月21日, 下午3:27
 */

#ifndef _CONFIG__HXX
#define	_CONFIG__HXX

/** \brief
 *  Singleton INotify.
 * \return #define INotify_Monitor
 *
 */
#define INotify_Monitor (Sigleton<INotify>::instance())

//#define __DEBUG__
#define Throwable


/** \brief
 *  Config File Name Defines
 *
 */
#define ConfigFileName "/etc/fsmon_Config.ini"

/** \brief
 *  Remove Fsmon Mutex and exit with code x
 * \param x
 * \return #define SafeExit(x)
 *
 */
#define SafeExit(x) {system("rm /tmp/fsmon_mutex");exit(x);}

#endif	/* _CONFIG__HXX */

