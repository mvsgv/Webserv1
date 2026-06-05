/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stephen <stephen@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/31 14:08:55 by stephen           #+#    #+#             */
/*   Updated: 2026/06/03 15:55:25 by stephen          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"
# include <fcntl.h>
# include <unistd.h>

class Router{	//new
private:
public:
	HttpResponse route(const HttpRequest& req);
};

HttpResponse handleGet(const HttpRequest& request);
HttpResponse handlePost(const HttpRequest& request);
HttpResponse handleDelete(const HttpRequest& request);
HttpResponse errorResponse(int status);