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

#include "prophetxmldoc.h"
#include "prophetxmlelement.h"

XmlDocument::XmlDocument( const std::string &encoding,
                          const std::string &styleSheet )
	: m_rootElement( new XmlElement( "DummyRoot" ) )
	, m_styleSheet( styleSheet )
	, m_standalone( true )
{
	setEncoding( encoding );
}

XmlDocument::~XmlDocument()
{
	delete m_rootElement;
}

std::string XmlDocument::encoding() const
{
	return m_encoding;
}

void 
XmlDocument::setEncoding( const std::string &encoding )
{
	m_encoding = encoding.empty() ? std::string("ISO-8859-1") : encoding;
}

std::string XmlDocument::styleSheet() const
{
	return m_styleSheet;
}

void XmlDocument::setStyleSheet( const std::string &styleSheet )
{
	m_styleSheet = styleSheet;
}

bool XmlDocument::standalone() const
{
	return m_standalone;
}

void XmlDocument::setStandalone( bool standalone )
{
	m_standalone = standalone;
}

void XmlDocument::setRootElement( XmlElement *rootElement )
{
	if ( rootElement == m_rootElement )
		return;

	delete m_rootElement;
	m_rootElement = rootElement;
}

XmlElement& XmlDocument::rootElement() const
{
	return *m_rootElement;
}

std::string XmlDocument::toString() const
{
	std::string asString = "<?xml version=\"1.0\" "
		"encoding='" + m_encoding + "'";
	if ( m_standalone )
		asString += " standalone='yes'";

	asString += " ?>\n"; 

	if ( !m_styleSheet.empty() )
		asString += "<?xml-stylesheet type=\"text/xsl\" href=\"" + m_styleSheet + "\"?>\n";

	asString += m_rootElement->toString();

	return asString;
}