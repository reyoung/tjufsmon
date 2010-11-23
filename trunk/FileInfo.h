/*
 * File:   FileInfo.h
 * Author: reyoung
 *
 * Created on 2010年11月21日, 下午3:34
 */

#ifndef _FILEINFO_H
#define	_FILEINFO_H
#include <string>

class FileInfo {
public:
    /** \brief
     *  File Info Default Constructor .
     * \param fn const std::string&
     * The File Name.
     */
    FileInfo(const std::string& fn);

    /** \brief
     * Copy Constructor.
     * \param orig const FileInfo&
     *
     */
    FileInfo(const FileInfo& orig);

    /** \brief
     *  Get Base Name Of FileInfo
     *  For Example:
     *          the base name of 'a.txt' is 'txt'.
     * \return const std::string
     *  base Name string.
     */
    const std::string getBaseName()const;
    /** \brief
     *  Is Base Name Matched.
     *  input the base name, return the base name is match to
     *  File Info or not.
     * \param fn const std::string&
     * \return bool
     *  Is Matched Return true.
     */
    bool isBaseNameMatch(const std::string& fn)const;

    /** \brief
     *  Deconstructor.
     * \return virtual
     *
     */
    virtual ~FileInfo();
private:
    std::string m_filename;
};

#endif	/* _FILEINFO_H */

