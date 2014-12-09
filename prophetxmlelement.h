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

#ifndef _PROPHETXMLELEMENT_H
#define _PROPHETXMLELEMENT_H

#include <deque>
#include <string>

class XmlElement;

/*! \brief A XML Element.
 * 
 * A XML element has:
 * - a name, specified on construction,
 * - a content, specified on construction (may be empty),
 * - zero or more attributes, added with addAttribute(),
 * - zero or more child elements, added with addElement().
 */
class XmlElement
{
public:
	/*! \brief Constructs an element with the specified name and string content.
	* \param elementName Name of the element. Must not be empty.
	* \param content Content of the element.
	*/
	XmlElement( std::string elementName,
		std::string content ="" );

	/*! \brief Constructs an element with the specified name and numeric content.
	* \param elementName Name of the element. Must not be empty.
	* \param numericContent Content of the element.
	*/
	XmlElement( std::string elementName,
		int numericContent );

	/*! \brief Destructs the element and its child elements.
	*/
	virtual ~XmlElement();

	/*! \brief Returns the name of the element.
	* \return Name of the element.
	*/
	std::string name() const;

	/*! \brief Returns the content of the element.
	* \return Content of the element.
	*/
	std::string content() const;

	/*! \brief Sets the name of the element.
	* \param name New name for the element.
	*/
	void setName( const std::string &name );

	/*! \brief Sets the content of the element.
	* \param content New content for the element.
	*/
	void setContent( const std::string &content );

	/*! \overload void setContent( const std::string &content )
	*/
	void setContent( int numericContent );

	/*! \brief Adds an attribute with the specified string value.
	* \param attributeName Name of the attribute. Must not be an empty.
	* \param value Value of the attribute.
	*/
	void addAttribute( std::string attributeName,
                     std::string value );

	/*! \brief Adds an attribute with the specified numeric value.
	* \param attributeName Name of the attribute. Must not be empty.
	* \param numericValue Numeric value of the attribute.
	*/
	void addAttribute( std::string attributeName,
		int numericValue );

	/*! \brief Adds a child element to the element.
	* \param element Child element to add. Must not be \c NULL.
	*/
	void addElement( XmlElement *element );

	/*! \brief Returns the number of child elements.
	* \return Number of child elements (element added with addElement()).
	*/
	int elementCount() const;

	/*! \brief Returns the child element at the specified index.
	* \param index Zero based index of the element to return.
	* \returns Element at the specified index. Never \c NULL.
	* \exception std::invalid_argument if \a index < 0 or index >= elementCount().
	*/
	XmlElement *elementAt( int index ) const;

	/*! \brief Returns the first child element with the specified name.
	* \param name Name of the child element to return.
	* \return First child element found which is named \a name.
	* \exception std::invalid_argument if there is no child element with the specified
	*            name.
	*/
	XmlElement *elementFor( const std::string &name ) const;

	/*! \brief Returns a XML string that represents the element.
	* \param indent String of spaces representing the amount of 'indent'.
	* \return XML string that represents the element, its attributes and its
	*         child elements.
	*/
	std::string toString( const std::string &indent = "" ) const;

private:
	typedef std::pair<std::string,std::string> Attribute;

	std::string attributesAsString() const;
	std::string escape( std::string value ) const;

private:
	std::string m_name;
	std::string m_content;

	typedef std::deque<Attribute> Attributes;
	Attributes m_attributes;

	typedef std::deque<XmlElement *> Elements;
	Elements m_elements;
};

#endif  // _XMLELEMENT_H