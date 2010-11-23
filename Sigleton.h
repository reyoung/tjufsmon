/*
 * File:   Config..hxx
 * Author: reyoung
 *
 * Created on 2010年2月12日, 下午2:27
 */
#ifndef SIGLETON_H
#define SIGLETON_H

template <typename T>
/** \brief
 *  Impletment of sigleton design pattarn.
 */
class Sigleton
{
public:
    /** \brief
     *  Singleton instance
     * \return T&
     *
     */
	static T & instance()
	{
		static T m_instance;
		return m_instance;
	}

private:
	Sigleton();
	~Sigleton();
	Sigleton(const Sigleton& other);
	Sigleton operator = (const Sigleton& other);
};


#endif // SIGLETON_H
