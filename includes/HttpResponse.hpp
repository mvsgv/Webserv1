/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stephen <stephen@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 14:57:09 by stephen           #+#    #+#             */
/*   Updated: 2026/06/03 15:03:40 by stephen          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include <iostream>
# include <sstream>
# include <map>

//this will produce final raw string that the future server will send with send() through the socket
//add some getters to make things pretty
class HttpResponse{
private:

public:
	int statusCode;
	std::map<std::string, std::string> headers;
	std::string body;	//new
	std::string buildResponse();
	//wouldn't the responsse need a version member too ?
};

std::string itostr(size_t n);
std::string status_message(int code);
//Code	Meaning
// 200	OK
// 201	Created
// 400	Bad Request
// 403	Forbidden
// 404	Not Found
// 500	Internal Server Error