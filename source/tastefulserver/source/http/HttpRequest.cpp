/**
 * (C) LGPL-3
 *
 * Tasteful Server <https://github.com/scheibel/tasteful-server>
 *
 * Copyright: 2012-2014 Lux, Scheibel
 * Authors:
 *     Roland Lux <rollux2000@googlemail.com>
 *     Willy Scheibel <willyscheibel@gmx.de>
 *
 * This file is part of Tasteful Server.
 *
 * Tasteful Server is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tasteful Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tasteful Server.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <tastefulserver/HttpRequest.h>

#include <QTextStream>

namespace tastefulserver {

HttpRequest::HttpRequest()
: m_bad(true)
, m_port(0)
{
}

HttpRequest::HttpRequest(const HttpMethod & method, const QString & requestUri, const HttpVersion & httpVersion)
: HttpMessage(httpVersion)
, m_bad(false)
, m_port(0)
, m_method(method)
, m_requestUri(requestUri)
{
    m_url = QUrl::fromEncoded(requestUri.toLatin1());
    if (m_url.scheme().isEmpty())
    {
        m_url.setScheme("http");
    }
    m_requestParams.parseUrl(m_url);
}

bool HttpRequest::isBad() const
{
    return m_bad;
}

void HttpRequest::markBad()
{
    m_bad = true;
}

void HttpRequest::setHttps(bool isHttps)
{
    m_url.setScheme(isHttps ? "https" : "http");
}

const QHostAddress &HttpRequest::address() const
{
    return m_address;
}

void HttpRequest::setAddress(const QHostAddress & address)
{
    m_address = address;
}

unsigned HttpRequest::port() const
{
    return m_port;
}

void HttpRequest::setPort(unsigned port)
{
    m_port = port;
}

bool HttpRequest::isXMLHttpRequest() const
{
    return getHeader(http::XRequestedWith).getValue()=="XMLHttpRequest";
}

HttpMethod HttpRequest::getMethod() const
{
    return m_method;
}

QString HttpRequest::getRequestUri() const
{
    return m_requestUri;
}

void HttpRequest::parseHeader(const HttpHeader & header)
{
    addHeader(header);
    QString headerName = header.getName();
    QString value = header.getValue();
    if (headerName==http::Host)
    {
        m_url.setAuthority(value);
    }
    else if (headerName==http::Cookie)
    {
        m_cookies.parse(value);
    }
    else if (headerName==http::ContentType)
    {
        m_contentType.parse(value);
    }
}

void HttpRequest::parseContent(const QByteArray & content)
{
    setContent(content);
    if (isMultiPart())
    {
        m_multiPart = MultiPart(m_contentType);
        m_multiPart.parse(content);
        if (m_multiPart.isFormData())
        {
            m_requestParams.parseMultiPart(m_multiPart);
        }
    }
    else if (m_contentType.is(ContentType::Application, ContentType::XWWWFormUrlEncoded))
    {
        m_requestParams.parseUrlEncoded(content);
    }
}

QUrl HttpRequest::getUrl() const
{
    return m_url;
}

QString HttpRequest::getPath() const
{
    return m_url.path();
}

RequestParameters &HttpRequest::getParameters()
{
    return m_requestParams;
}

const RequestParameters &HttpRequest::getParameters() const
{
    return m_requestParams;
}

QByteArray HttpRequest::toByteArray() const
{
    QByteArray byteArray;
    QTextStream stream(&byteArray);

    stream << m_method.toString() << " " << m_requestUri << " " << m_httpVersion.toString() << http::Linebreak;

    stream << HttpMessage::toByteArray();

    return byteArray;
}

} // namespace tastefulserver
