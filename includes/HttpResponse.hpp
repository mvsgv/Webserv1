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

class HttpResponse{
	public:
		int statusCode;
		std::map<std::string, std::string> headers;
		std::string body;	//new
		std::string buildResponse();

		HttpResponse();
		~HttpResponse();
};


std::string status_message(int code);
//Code	Meaning
// 200	OK
// 201	Created
// 400	Bad Request
// 403	Forbidden
// 404	Not Found
// 500	Internal Server Error