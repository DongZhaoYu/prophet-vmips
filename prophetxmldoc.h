/* Main driver program for Prophet.
   Copyright2009 DongZhaoyu, GaoBing.

This file is part of Prophet.

Prophet is free software developed based on VMIPS, you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Prophet is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with VMIPS; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _PROPHETXMLDOC_H
#define _PROPHETXMLDOC_H

#include <string>

class XmlElement;

/*! \brief A XML Document.
 *
 * A XmlDocument represents a XML file. It holds a pointer on the root XmlElement
 * of the document. It also holds the encoding and style sheet used.
 *
 * By default, the XML document is stand-alone and tagged with enconding "ISO-8859-1".
 */

class XmlDocument
{
public:
	/*! \brief Constructs a XmlDocument object.
	* \param encoding Encoding used in the XML file (default is Latin-1, ISO-8859-1 ). 
	* \param styleSheet Name of the XSL style sheet file used. If empty then no
	*                   style sheet will be specified in the output.
	*/
	XmlDocument( const std::string &encoding = "",
		const std::string &styleSheet = "" );

	/// Destructor.
	virtual ~XmlDocument();

	std::string encoding() const;
	void setEncoding( const std::string &encoding = "" );

	std::string styleSheet() const;
	void setStyleSheet( const std::string &styleSheet = "" );

	bool standalone() const;

	/*! \brief set the output document as standalone or not.
	*
	*  For the output document, specify wether it's a standalone XML
	*  document, or not.
	*
	*  \param standalone if true, the output will be specified as standalone.
	*         if false, it will be not.
	*/
	void setStandalone( bool standalone );

	void setRootElement( XmlElement *rootElement );
	XmlElement &rootElement() const;

	std::string toString() const;

private:
	/// Prevents the use of the copy constructor.
	XmlDocument( const XmlDocument &copy );

	/// Prevents the use of the copy operator.
	void operator =( const XmlDocument &copy );

protected:
	std::string m_encoding;
	std::string m_styleSheet;
	XmlElement *m_rootElement;
	bool m_standalone;
};

#endif  // _XMLDOCUMENT_H