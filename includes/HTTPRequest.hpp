/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mariamevissargova <mariamevissargova@st    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 13:01:06 by stephen           #+#    #+#             */
/*   Updated: 2026/06/05 15:10:57 by mariameviss      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include <iostream>
# include <sstream>
# include <string>
# include <algorithm>
# include <map>
# include <sys/socket.h>

//add some getters to make things pretty
class HttpRequest{
	private:
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	bool check_empty();
public:
	bool parse(const std::string& rawRequest);
	const std::string& getMethod() const;
    const std::string& getUri() const;
    const std::string& getVersion() const;
    const std::string& getBody() const;
    std::string getHeader(const std::string& key) const;
};

std::string gnl_req(const std::string &rawRequest, size_t &eol);
//classic http request looks like this :
//
//[METHOD] [URI] [VERSION]					//request line
//[HEADERS]
//
//[BODY]

